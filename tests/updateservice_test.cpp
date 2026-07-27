// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/updateservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest>

namespace {
QByteArray releasePayload(
    const QString &version,
    const QString &platform,
    const QString &assetName = {},
    const QString &digest = QString(64, QLatin1Char('a')),
    const QString &urlHost = QStringLiteral("github.com"))
{
    QJsonObject root{
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("version"), version},
        {QStringLiteral("platform"), platform},
        {QStringLiteral("notes"), QStringLiteral("A focused new release.")},
    };
    if (!assetName.isEmpty()) {
        QString tag = version;
        if (!tag.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
            tag.prepend(QLatin1Char('v'));
        }
        root.insert(QStringLiteral("assetName"), assetName);
        root.insert(QStringLiteral("size"), 4096);
        root.insert(QStringLiteral("sha256"), digest);
        root.insert(
            QStringLiteral("downloadUrl"),
            QStringLiteral("https://%1/eyial667/daymark/releases/download/%2/%3")
                .arg(urlHost, tag, assetName));
    }
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}
} // namespace

class UpdateServiceTest final : public QObject
{
    Q_OBJECT

private slots:
    void acceptsVerifiedPlatformRelease();
    void treatsCurrentAndOlderReleasesAsUpToDate();
    void usesTokenFreeReleaseManifests();
    void rejectsAnotherPlatformManifest();
    void rejectsUnexpectedPlatformPackage();
    void rejectsMissingDigest();
    void rejectsUntrustedDownloadHost();
    void rejectsInvalidVersions();
};

void UpdateServiceTest::acceptsVerifiedPlatformRelease()
{
    UpdateService::Release release;
    QString error;
    const QByteArray payload = releasePayload(
        QStringLiteral("0.2.0"),
        QStringLiteral("linux"),
        QStringLiteral("Daymark-0.2.0-Linux.tar.gz"));

    QVERIFY(UpdateService::parseRelease(
        payload,
        QStringLiteral("0.1.0"),
        QStringLiteral("linux"),
        &release,
        &error));
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QVERIFY(release.isNewer);
    QCOMPARE(release.version, QStringLiteral("0.2.0"));
    QCOMPARE(release.assetName, QStringLiteral("Daymark-0.2.0-Linux.tar.gz"));
    QCOMPARE(release.size, 4096);
    QCOMPARE(release.sha256, QByteArray(32, static_cast<char>(0xaa)));
}

void UpdateServiceTest::treatsCurrentAndOlderReleasesAsUpToDate()
{
    for (const QString &version : {QStringLiteral("0.2.0"), QStringLiteral("0.1.9")}) {
        UpdateService::Release release;
        QString error;
        QVERIFY(UpdateService::parseRelease(
            releasePayload(version, QStringLiteral("windows")),
            QStringLiteral("0.2.0"),
            QStringLiteral("windows"),
            &release,
            &error));
        QVERIFY(!release.isNewer);
        QVERIFY(error.isEmpty());
    }
}

void UpdateServiceTest::usesTokenFreeReleaseManifests()
{
    const QUrl linuxUrl = UpdateService::releaseManifestUrl(QStringLiteral("linux"));
    QCOMPARE(linuxUrl.host(), QStringLiteral("github.com"));
    QVERIFY(!linuxUrl.host().startsWith(QStringLiteral("api.")));
    QVERIFY(linuxUrl.path().endsWith(QStringLiteral("Daymark-update-linux.json")));
    QVERIFY(UpdateService::releaseManifestUrl(QStringLiteral("freebsd")).isEmpty());
}

void UpdateServiceTest::rejectsAnotherPlatformManifest()
{
    UpdateService::Release release;
    QString error;
    const QByteArray payload = releasePayload(
        QStringLiteral("0.3.0"),
        QStringLiteral("windows"),
        QStringLiteral("Daymark-0.3.0-win64-setup.exe"));

    QVERIFY(!UpdateService::parseRelease(
        payload,
        QStringLiteral("0.2.0"),
        QStringLiteral("linux"),
        &release,
        &error));
    QVERIFY(error.contains(QStringLiteral("metadata")));
}

void UpdateServiceTest::rejectsUnexpectedPlatformPackage()
{
    UpdateService::Release release;
    QString error;
    const QByteArray payload = releasePayload(
        QStringLiteral("0.3.0"),
        QStringLiteral("linux"),
        QStringLiteral("Daymark-0.3.0-win64-setup.exe"));

    QVERIFY(!UpdateService::parseRelease(
        payload,
        QStringLiteral("0.2.0"),
        QStringLiteral("linux"),
        &release,
        &error));
    QVERIFY(error.contains(QStringLiteral("integrity")));
}

void UpdateServiceTest::rejectsMissingDigest()
{
    UpdateService::Release release;
    QString error;
    const QByteArray payload = releasePayload(
        QStringLiteral("0.3.0"),
        QStringLiteral("macos"),
        QStringLiteral("Daymark-0.3.0-Darwin.dmg"),
        QString());

    QVERIFY(!UpdateService::parseRelease(
        payload,
        QStringLiteral("0.2.0"),
        QStringLiteral("macos"),
        &release,
        &error));
    QVERIFY(error.contains(QStringLiteral("integrity")));
}

void UpdateServiceTest::rejectsUntrustedDownloadHost()
{
    UpdateService::Release release;
    QString error;
    const QByteArray payload = releasePayload(
        QStringLiteral("0.3.0"),
        QStringLiteral("windows"),
        QStringLiteral("Daymark-0.3.0-win64-setup.exe"),
        QString(64, QLatin1Char('b')),
        QStringLiteral("example.com"));

    QVERIFY(!UpdateService::parseRelease(
        payload,
        QStringLiteral("0.2.0"),
        QStringLiteral("windows"),
        &release,
        &error));
    QVERIFY(error.contains(QStringLiteral("integrity")));
}

void UpdateServiceTest::rejectsInvalidVersions()
{
    UpdateService::Release release;
    QString error;
    QVERIFY(!UpdateService::parseRelease(
        releasePayload(QStringLiteral("nightly"), QStringLiteral("linux")),
        QStringLiteral("0.2.0"),
        QStringLiteral("linux"),
        &release,
        &error));
    QVERIFY(error.contains(QStringLiteral("version")));
}

QTEST_GUILESS_MAIN(UpdateServiceTest)

#include "updateservice_test.moc"
