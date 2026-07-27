// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QByteArray>
#include <QObject>
#include <QUrl>

#include <memory>

class QNetworkAccessManager;
class QNetworkReply;
class QProcess;
class QSaveFile;

class UpdateService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY releaseChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY releaseChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString installActionLabel READ installActionLabel NOTIFY languageChanged)
    Q_PROPERTY(QString installHint READ installHint NOTIFY languageChanged)
    Q_PROPERTY(qint64 downloadSize READ downloadSize NOTIFY releaseChanged)
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY stateChanged)
    Q_PROPERTY(bool checking READ checking NOTIFY stateChanged)
    Q_PROPERTY(bool downloading READ downloading NOTIFY stateChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY stateChanged)
    Q_PROPERTY(bool canDownload READ canDownload NOTIFY stateChanged)
    Q_PROPERTY(bool canInstall READ canInstall NOTIFY stateChanged)

public:
    enum State {
        Idle,
        Checking,
        UpToDate,
        UpdateAvailable,
        Downloading,
        ReadyToInstall,
        Installing,
        Error,
    };
    Q_ENUM(State)

    struct Release {
        QString version;
        QString notes;
        QString assetName;
        QUrl downloadUrl;
        QByteArray sha256;
        qint64 size = 0;
        bool isNewer = false;
    };

    explicit UpdateService(QObject *parent = nullptr);
    ~UpdateService() override;

    [[nodiscard]] State state() const;
    [[nodiscard]] QString currentVersion() const;
    [[nodiscard]] QString latestVersion() const;
    [[nodiscard]] QString releaseNotes() const;
    [[nodiscard]] QString statusMessage() const;
    [[nodiscard]] QString installActionLabel() const;
    [[nodiscard]] QString installHint() const;
    [[nodiscard]] qint64 downloadSize() const;
    [[nodiscard]] int downloadProgress() const;
    [[nodiscard]] bool busy() const;
    [[nodiscard]] bool checking() const;
    [[nodiscard]] bool downloading() const;
    [[nodiscard]] bool hasError() const;
    [[nodiscard]] bool canDownload() const;
    [[nodiscard]] bool canInstall() const;

    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void downloadUpdate();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void installUpdate();
    Q_INVOKABLE void retranslate();

    static bool parseRelease(
        const QByteArray &payload,
        const QString &currentVersion,
        const QString &platform,
        Release *release,
        QString *errorMessage);
    [[nodiscard]] static QUrl releaseManifestUrl(const QString &platform);

signals:
    void stateChanged();
    void releaseChanged();
    void statusMessageChanged();
    void downloadProgressChanged();
    void updateFound();
    void quitRequested();
    void languageChanged();

private:
    void setState(State state);
    void setStatusMessage(const QString &message);
    void fail(const QString &message);
    void finishCheck(QNetworkReply *reply);
    void finishDownload(QNetworkReply *reply);
    void writeDownloadData(QNetworkReply *reply);
    void clearDownload();
    void installLinuxArchive();
    [[nodiscard]] static QString platformKey();
    [[nodiscard]] static bool isTrustedDownloadUrl(const QUrl &url);

    QNetworkAccessManager *m_network = nullptr;
    QNetworkReply *m_checkReply = nullptr;
    QNetworkReply *m_downloadReply = nullptr;
    std::unique_ptr<QSaveFile> m_downloadFile;
    QProcess *m_installProcess = nullptr;
    Release m_release;
    QString m_statusMessage;
    QString m_downloadedPath;
    State m_state = Idle;
    qint64 m_downloadedBytes = 0;
    int m_downloadProgress = 0;
    bool m_canceling = false;
};
