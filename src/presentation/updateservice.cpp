// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/updateservice.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QVersionNumber>

namespace {
constexpr qint64 maximumPackageSize = 1024LL * 1024LL * 1024LL;
const QUrl latestReleaseUrl(
    QStringLiteral("https://api.github.com/repos/eyial667/daymark/releases/latest"));

QString expectedAssetName(const QString &version, const QString &platform)
{
    if (platform == QStringLiteral("windows")) {
        return QStringLiteral("Daymark-%1-win64-setup.exe").arg(version);
    }
    if (platform == QStringLiteral("macos")) {
        return QStringLiteral("Daymark-%1-Darwin.dmg").arg(version);
    }
    if (platform == QStringLiteral("linux")) {
        return QStringLiteral("Daymark-%1-Linux.tar.gz").arg(version);
    }
    return {};
}

bool parseVersion(const QString &value, QVersionNumber *version)
{
    QString normalized = value.trimmed();
    if (normalized.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        normalized.remove(0, 1);
    }

    static const QRegularExpression pattern(
        QStringLiteral("^[0-9]+(?:\\.[0-9]+){2}$"));
    if (!pattern.match(normalized).hasMatch()) {
        return false;
    }

    *version = QVersionNumber::fromString(normalized);
    return !version->isNull();
}

QString cacheRoot()
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/updates");
}
} // namespace

UpdateService::UpdateService(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    m_network->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

UpdateService::~UpdateService() = default;

UpdateService::State UpdateService::state() const
{
    return m_state;
}

QString UpdateService::currentVersion() const
{
    return QCoreApplication::applicationVersion();
}

QString UpdateService::latestVersion() const
{
    return m_release.version;
}

QString UpdateService::releaseNotes() const
{
    return m_release.notes;
}

QString UpdateService::statusMessage() const
{
    return m_statusMessage;
}

QString UpdateService::installActionLabel() const
{
#if defined(Q_OS_WIN)
    return tr("Open installation wizard");
#elif defined(Q_OS_MACOS)
    return tr("Open disk image");
#else
    return tr("Install and close Daymark");
#endif
}

QString UpdateService::installHint() const
{
#if defined(Q_OS_WIN)
    return tr("The native setup wizard will open and Daymark will close.");
#elif defined(Q_OS_MACOS)
    return tr("The disk image will open. Replace Daymark in Applications, then relaunch it.");
#else
    return tr("The per-user installer will run and Daymark will close. Reopen it when installation finishes.");
#endif
}

qint64 UpdateService::downloadSize() const
{
    return m_release.size;
}

int UpdateService::downloadProgress() const
{
    return m_downloadProgress;
}

bool UpdateService::busy() const
{
    return m_state == Checking || m_state == Downloading || m_state == Installing;
}

bool UpdateService::checking() const
{
    return m_state == Checking;
}

bool UpdateService::downloading() const
{
    return m_state == Downloading;
}

bool UpdateService::hasError() const
{
    return m_state == Error;
}

bool UpdateService::canDownload() const
{
    return m_state == UpdateAvailable;
}

bool UpdateService::canInstall() const
{
    return m_state == ReadyToInstall;
}

void UpdateService::checkForUpdates()
{
    if (m_state == Checking || m_state == Downloading || m_state == Installing) {
        return;
    }

    if (m_checkReply) {
        m_checkReply->abort();
        m_checkReply->deleteLater();
        m_checkReply = nullptr;
    }

    setState(Checking);
    setStatusMessage(tr("Checking GitHub for updates…"));

    QNetworkRequest request(latestReleaseUrl);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setRawHeader(
        "User-Agent",
        QStringLiteral("Daymark/%1").arg(currentVersion()).toUtf8());
    request.setTransferTimeout(15000);

    m_checkReply = m_network->get(request);
    connect(m_checkReply, &QNetworkReply::finished, this, [this, reply = m_checkReply] {
        finishCheck(reply);
    });
}

void UpdateService::finishCheck(QNetworkReply *reply)
{
    if (reply != m_checkReply) {
        reply->deleteLater();
        return;
    }
    m_checkReply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        const QString detail = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt() == 404
            ? tr("No published Daymark release is available yet.")
            : tr("The update check could not reach GitHub: %1").arg(reply->errorString());
        reply->deleteLater();
        fail(detail);
        return;
    }
    if (reply->url().scheme() != QStringLiteral("https")
        || reply->url().host().compare(
            QStringLiteral("api.github.com"), Qt::CaseInsensitive) != 0) {
        reply->deleteLater();
        fail(tr("The release check was redirected outside GitHub."));
        return;
    }

    Release release;
    QString parseError;
    const QByteArray payload = reply->readAll();
    reply->deleteLater();
    if (!parseRelease(payload, currentVersion(), platformKey(), &release, &parseError)) {
        fail(parseError);
        return;
    }

    m_release = release;
    emit releaseChanged();
    if (!release.isNewer) {
        setState(UpToDate);
        setStatusMessage(tr("Daymark %1 is up to date.").arg(currentVersion()));
        return;
    }

    setState(UpdateAvailable);
    setStatusMessage(tr("Daymark %1 is ready to download.").arg(release.version));
    emit updateFound();
}

void UpdateService::downloadUpdate()
{
    if (!canDownload()) {
        return;
    }

    if (!isTrustedDownloadUrl(m_release.downloadUrl)) {
        fail(tr("GitHub returned an untrusted update address."));
        return;
    }

    const QString directory = cacheRoot() + QLatin1Char('/') + m_release.version;
    if (!QDir().mkpath(directory)) {
        fail(tr("Daymark could not create its update cache."));
        return;
    }

    m_downloadedPath = directory + QLatin1Char('/') + m_release.assetName;
    m_downloadFile = std::make_unique<QSaveFile>(m_downloadedPath);
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
        fail(tr("Daymark could not write the downloaded update."));
        clearDownload();
        return;
    }

    m_downloadedBytes = 0;
    m_downloadProgress = 0;
    m_canceling = false;
    emit downloadProgressChanged();
    setState(Downloading);
    setStatusMessage(tr("Downloading Daymark %1…").arg(m_release.version));

    QNetworkRequest request(m_release.downloadUrl);
    request.setRawHeader("Accept", "application/octet-stream");
    request.setRawHeader(
        "User-Agent",
        QStringLiteral("Daymark/%1").arg(currentVersion()).toUtf8());
    request.setTransferTimeout(30000);
    m_downloadReply = m_network->get(request);
    connect(m_downloadReply, &QIODevice::readyRead, this, [this, reply = m_downloadReply] {
        writeDownloadData(reply);
    });
    connect(
        m_downloadReply,
        &QNetworkReply::downloadProgress,
        this,
        [this](qint64 received, qint64 total) {
            const qint64 denominator = total > 0 ? total : m_release.size;
            const int progress = denominator > 0
                ? qBound(0, static_cast<int>((received * 100) / denominator), 100)
                : 0;
            if (progress != m_downloadProgress) {
                m_downloadProgress = progress;
                emit downloadProgressChanged();
            }
        });
    connect(m_downloadReply, &QNetworkReply::finished, this, [this, reply = m_downloadReply] {
        finishDownload(reply);
    });
}

void UpdateService::writeDownloadData(QNetworkReply *reply)
{
    if (reply != m_downloadReply || !m_downloadFile) {
        return;
    }

    const QByteArray data = reply->readAll();
    m_downloadedBytes += data.size();
    if (m_downloadedBytes > maximumPackageSize || m_downloadedBytes > m_release.size
        || m_downloadFile->write(data) != data.size()) {
        m_canceling = false;
        reply->abort();
    }
}

void UpdateService::finishDownload(QNetworkReply *reply)
{
    if (reply != m_downloadReply) {
        reply->deleteLater();
        return;
    }
    writeDownloadData(reply);
    m_downloadReply = nullptr;
    const bool canceled = m_canceling;
    m_canceling = false;
    if (canceled) {
        reply->deleteLater();
        clearDownload();
        setState(UpdateAvailable);
        setStatusMessage(tr("Update download canceled."));
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        const QString error = reply->errorString();
        reply->deleteLater();
        clearDownload();
        fail(tr("The update download failed: %1").arg(error));
        return;
    }
    if (!isTrustedDownloadUrl(reply->url())) {
        reply->deleteLater();
        clearDownload();
        fail(tr("The update was redirected outside GitHub."));
        return;
    }
    reply->deleteLater();

    if (m_downloadedBytes != m_release.size || !m_downloadFile->commit()) {
        clearDownload();
        fail(tr("The downloaded package was incomplete."));
        return;
    }
    m_downloadFile.reset();

    QFile package(m_downloadedPath);
    if (!package.open(QIODevice::ReadOnly)) {
        clearDownload();
        fail(tr("The downloaded package could not be verified."));
        return;
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&package) || hash.result() != m_release.sha256) {
        package.close();
        QFile::remove(m_downloadedPath);
        clearDownload();
        fail(tr("The update failed its GitHub SHA-256 integrity check."));
        return;
    }

    m_downloadProgress = 100;
    emit downloadProgressChanged();
    setState(ReadyToInstall);
    setStatusMessage(tr("Daymark %1 was downloaded and verified.").arg(m_release.version));
}

void UpdateService::cancelDownload()
{
    if (m_state != Downloading || !m_downloadReply) {
        return;
    }
    m_canceling = true;
    m_downloadReply->abort();
}

void UpdateService::installUpdate()
{
    if (!canInstall() || !QFileInfo::exists(m_downloadedPath)) {
        return;
    }

#if defined(Q_OS_WIN)
    if (!QProcess::startDetached(m_downloadedPath, {})) {
        fail(tr("The Windows installation wizard could not be opened."));
        return;
    }
    setState(Installing);
    setStatusMessage(tr("The installation wizard is open."));
    emit quitRequested();
#elif defined(Q_OS_MACOS)
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadedPath))) {
        fail(tr("The macOS disk image could not be opened."));
        return;
    }
    setState(ReadyToInstall);
    setStatusMessage(tr("The disk image is open. Replace Daymark in Applications."));
#else
    installLinuxArchive();
#endif
}

void UpdateService::retranslate()
{
    emit languageChanged();
    switch (m_state) {
    case Checking:
        setStatusMessage(tr("Checking GitHub for updates…"));
        break;
    case UpToDate:
        setStatusMessage(tr("Daymark %1 is up to date.").arg(currentVersion()));
        break;
    case UpdateAvailable:
        setStatusMessage(tr("Daymark %1 is ready to download.").arg(m_release.version));
        break;
    case Downloading:
        setStatusMessage(tr("Downloading Daymark %1…").arg(m_release.version));
        break;
    case ReadyToInstall:
        setStatusMessage(tr("Daymark %1 was downloaded and verified.").arg(m_release.version));
        break;
    case Idle:
    case Installing:
    case Error:
        break;
    }
}

void UpdateService::installLinuxArchive()
{
    const QString tarExecutable = QStandardPaths::findExecutable(QStringLiteral("tar"));
    if (tarExecutable.isEmpty()) {
        fail(tr("The Linux updater needs the ‘tar’ command to install this package."));
        return;
    }

    const QString extractionDirectory = cacheRoot()
        + QStringLiteral("/install-") + m_release.version;
    QDir extraction(extractionDirectory);
    if (extraction.exists() && !extraction.removeRecursively()) {
        fail(tr("The previous update staging folder could not be cleared."));
        return;
    }
    if (!QDir().mkpath(extractionDirectory)) {
        fail(tr("The update staging folder could not be created."));
        return;
    }

    m_installProcess = new QProcess(this);
    m_installProcess->setProgram(tarExecutable);
    m_installProcess->setArguments({
        QStringLiteral("--extract"),
        QStringLiteral("--gzip"),
        QStringLiteral("--file"),
        m_downloadedPath,
        QStringLiteral("--directory"),
        extractionDirectory,
        QStringLiteral("--strip-components=1"),
        QStringLiteral("--no-same-owner"),
        QStringLiteral("--no-same-permissions"),
    });
    connect(
        m_installProcess,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this,
        [this, extractionDirectory](int exitCode, QProcess::ExitStatus exitStatus) {
            QProcess *process = m_installProcess;
            m_installProcess = nullptr;
            process->deleteLater();
            if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                fail(tr("The Linux update package could not be extracted."));
                return;
            }

            const QString installerPath = extractionDirectory
                + QStringLiteral("/install.sh");
            const QFileInfo installer(installerPath);
            if (!installer.isFile() || !installer.isExecutable()
                || !installer.canonicalFilePath().startsWith(
                    QFileInfo(extractionDirectory).canonicalFilePath()
                        + QDir::separator())) {
                fail(tr("The Linux update package did not contain a valid installer."));
                return;
            }
            if (!QProcess::startDetached(installerPath, {}, extractionDirectory)) {
                fail(tr("The Linux installer could not be started."));
                return;
            }
            setStatusMessage(tr("The local installer is running."));
            emit quitRequested();
        });
    connect(
        m_installProcess,
        &QProcess::errorOccurred,
        this,
        [this](QProcess::ProcessError error) {
            if (error != QProcess::FailedToStart || !m_installProcess) {
                return;
            }
            QProcess *process = m_installProcess;
            m_installProcess = nullptr;
            process->deleteLater();
            fail(tr("The Linux update package could not be extracted."));
        });

    setState(Installing);
    setStatusMessage(tr("Preparing the verified Linux update…"));
    m_installProcess->start();
}

bool UpdateService::parseRelease(
    const QByteArray &payload,
    const QString &currentVersion,
    const QString &platform,
    Release *release,
    QString *errorMessage)
{
    if (!release) {
        return false;
    }

    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = tr("GitHub returned invalid release metadata.");
        }
        return false;
    }

    QVersionNumber current;
    QVersionNumber latest;
    const QJsonObject root = document.object();
    if (root.value(QStringLiteral("draft")).toBool()
        || root.value(QStringLiteral("prerelease")).toBool()) {
        if (errorMessage) {
            *errorMessage = tr("GitHub returned an unpublished Daymark release.");
        }
        return false;
    }
    QString tag = root.value(QStringLiteral("tag_name")).toString();
    QString normalizedTag = tag.trimmed();
    if (normalizedTag.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        normalizedTag.remove(0, 1);
    }
    if (!parseVersion(currentVersion, &current) || !parseVersion(tag, &latest)) {
        if (errorMessage) {
            *errorMessage = tr("The installed or published Daymark version is invalid.");
        }
        return false;
    }

    Release parsed;
    parsed.version = normalizedTag;
    parsed.notes = root.value(QStringLiteral("body")).toString().left(12000).trimmed();
    parsed.isNewer = QVersionNumber::compare(latest, current) > 0;
    if (!parsed.isNewer) {
        *release = parsed;
        return true;
    }

    parsed.assetName = expectedAssetName(parsed.version, platform);
    if (parsed.assetName.isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("This operating system does not have a Daymark updater package.");
        }
        return false;
    }

    const QJsonArray assets = root.value(QStringLiteral("assets")).toArray();
    for (const QJsonValue &value : assets) {
        const QJsonObject asset = value.toObject();
        if (asset.value(QStringLiteral("name")).toString() != parsed.assetName
            || asset.value(QStringLiteral("state")).toString() != QStringLiteral("uploaded")) {
            continue;
        }

        parsed.downloadUrl = QUrl(
            asset.value(QStringLiteral("browser_download_url")).toString());
        parsed.size = asset.value(QStringLiteral("size")).toInteger();
        const QString digest = asset.value(QStringLiteral("digest")).toString();
        static const QRegularExpression digestPattern(
            QStringLiteral("^sha256:([0-9a-fA-F]{64})$"));
        const QRegularExpressionMatch match = digestPattern.match(digest);
        if (!isTrustedDownloadUrl(parsed.downloadUrl)
            || parsed.size <= 0 || parsed.size > maximumPackageSize
            || !match.hasMatch()) {
            if (errorMessage) {
                *errorMessage = tr("The matching GitHub package has invalid integrity metadata.");
            }
            return false;
        }
        parsed.sha256 = QByteArray::fromHex(match.captured(1).toLatin1());
        *release = parsed;
        return true;
    }

    if (errorMessage) {
        *errorMessage = tr("GitHub does not contain a %1 update package for this release.")
            .arg(platform);
    }
    return false;
}

void UpdateService::setState(State state)
{
    if (state == m_state) {
        return;
    }
    m_state = state;
    emit stateChanged();
}

void UpdateService::setStatusMessage(const QString &message)
{
    if (message == m_statusMessage) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}

void UpdateService::fail(const QString &message)
{
    setState(Error);
    setStatusMessage(message);
}

void UpdateService::clearDownload()
{
    if (m_downloadFile) {
        m_downloadFile->cancelWriting();
        m_downloadFile.reset();
    }
    m_downloadedBytes = 0;
    m_downloadProgress = 0;
    emit downloadProgressChanged();
}

QString UpdateService::platformKey()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("windows");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("macos");
#else
    return QStringLiteral("linux");
#endif
}

bool UpdateService::isTrustedDownloadUrl(const QUrl &url)
{
    if (!url.isValid() || url.scheme() != QStringLiteral("https")
        || (url.port() != -1 && url.port() != 443)) {
        return false;
    }
    const QString host = url.host().toLower();
    return host == QStringLiteral("github.com")
        || host == QStringLiteral("objects.githubusercontent.com")
        || host.endsWith(QStringLiteral(".githubusercontent.com"));
}
