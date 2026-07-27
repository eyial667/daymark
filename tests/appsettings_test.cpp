// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/appsettings.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

class AppSettingsTest final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void persistsSelectedSettings();
    void ignoresInvalidStoredValues();
    void resetsPreferencesWithoutTouchingData();
};

void AppSettingsTest::initTestCase()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
}

void AppSettingsTest::persistsSelectedSettings()
{
    QTemporaryDir settingsDirectory;
    QVERIFY(settingsDirectory.isValid());
    QSettings::setPath(
        QSettings::IniFormat,
        QSettings::UserScope,
        settingsDirectory.path());
    QCoreApplication::setOrganizationName(QStringLiteral("DaymarkTest"));
    QCoreApplication::setApplicationName(QStringLiteral("SettingsPersistence"));

    {
        AppSettings settings(settingsDirectory.path());
        QCOMPARE(settings.interfaceStyle(), AppSettings::Hybrid);
        QCOMPARE(settings.colorMode(), AppSettings::Dark);
        QCOMPARE(settings.accentPreset(), AppSettings::MatchInterface);
        QCOMPARE(settings.dailyCapacityMinutes(), 480);
        QCOMPARE(settings.defaultEstimatedMinutes(), 30);
        QCOMPARE(settings.defaultImportance(), 3);
        QVERIFY(!settings.showTimeline());
        QVERIFY(settings.showPriorityReasons());
        QVERIFY(!settings.confirmTaskCompletion());

        QSignalSpy changedSpy(&settings, &AppSettings::interfaceStyleChanged);
        settings.setInterfaceStyle(AppSettings::SpatialMap);
        settings.setColorMode(AppSettings::Light);
        settings.setAccentPreset(AppSettings::Cyan);
        settings.setDailyCapacityMinutes(360);
        settings.setDefaultEstimatedMinutes(45);
        settings.setDefaultImportance(5);
        settings.setUse24HourClock(true);
        settings.setShowTimeline(true);
        settings.setShowPriorityReasons(false);
        settings.setConfirmTaskCompletion(true);
        QCOMPARE(changedSpy.count(), 1);
    }

    AppSettings restoredSettings(settingsDirectory.path());
    QCOMPARE(restoredSettings.interfaceStyle(), AppSettings::SpatialMap);
    QCOMPARE(restoredSettings.colorMode(), AppSettings::Light);
    QCOMPARE(restoredSettings.accentPreset(), AppSettings::Cyan);
    QCOMPARE(restoredSettings.dailyCapacityMinutes(), 360);
    QCOMPARE(restoredSettings.defaultEstimatedMinutes(), 45);
    QCOMPARE(restoredSettings.defaultImportance(), 5);
    QVERIFY(restoredSettings.use24HourClock());
    QVERIFY(restoredSettings.showTimeline());
    QVERIFY(!restoredSettings.showPriorityReasons());
    QVERIFY(restoredSettings.confirmTaskCompletion());
}

void AppSettingsTest::ignoresInvalidStoredValues()
{
    QTemporaryDir settingsDirectory;
    QVERIFY(settingsDirectory.isValid());
    QSettings::setPath(
        QSettings::IniFormat,
        QSettings::UserScope,
        settingsDirectory.path());
    QCoreApplication::setOrganizationName(QStringLiteral("DaymarkTest"));
    QCoreApplication::setApplicationName(QStringLiteral("InvalidSettings"));

    QSettings rawSettings;
    rawSettings.setValue(QStringLiteral("appearance/interfaceStyle"), 99);
    rawSettings.setValue(QStringLiteral("appearance/colorMode"), 8);
    rawSettings.setValue(QStringLiteral("appearance/accentPreset"), -1);
    rawSettings.setValue(QStringLiteral("planning/dailyCapacityMinutes"), 10);
    rawSettings.setValue(QStringLiteral("tasks/defaultEstimatedMinutes"), 900);
    rawSettings.setValue(QStringLiteral("tasks/defaultImportance"), 8);

    AppSettings settings(settingsDirectory.path());
    QCOMPARE(settings.interfaceStyle(), AppSettings::Hybrid);
    QCOMPARE(settings.colorMode(), AppSettings::Dark);
    QCOMPARE(settings.accentPreset(), AppSettings::MatchInterface);
    QCOMPARE(settings.dailyCapacityMinutes(), 480);
    QCOMPARE(settings.defaultEstimatedMinutes(), 30);
    QCOMPARE(settings.defaultImportance(), 3);
}

void AppSettingsTest::resetsPreferencesWithoutTouchingData()
{
    QTemporaryDir settingsDirectory;
    QVERIFY(settingsDirectory.isValid());
    QSettings::setPath(
        QSettings::IniFormat,
        QSettings::UserScope,
        settingsDirectory.path());
    QCoreApplication::setOrganizationName(QStringLiteral("DaymarkTest"));
    QCoreApplication::setApplicationName(QStringLiteral("SettingsReset"));

    AppSettings settings(settingsDirectory.path());
    settings.setInterfaceStyle(AppSettings::MidnightCommand);
    settings.setColorMode(AppSettings::Light);
    settings.setDailyCapacityMinutes(240);
    settings.setDefaultImportance(5);
    settings.setShowTimeline(true);

    QSignalSpy resetSpy(&settings, &AppSettings::settingsReset);
    settings.resetDefaults();

    QCOMPARE(resetSpy.count(), 1);
    QCOMPARE(settings.interfaceStyle(), AppSettings::Hybrid);
    QCOMPARE(settings.colorMode(), AppSettings::Dark);
    QCOMPARE(settings.dailyCapacityMinutes(), 480);
    QCOMPARE(settings.defaultImportance(), 3);
    QVERIFY(!settings.showTimeline());
    QCOMPARE(settings.dataDirectory(), QDir::cleanPath(settingsDirectory.path()));
}

QTEST_GUILESS_MAIN(AppSettingsTest)

#include "appsettings_test.moc"
