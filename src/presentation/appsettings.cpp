// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/appsettings.h"

#include <QDesktopServices>
#include <QDir>
#include <QLocale>
#include <QUrl>

namespace {

template<typename Enum>
Enum storedEnum(
    const QSettings &settings,
    const char *key,
    Enum fallback,
    int minimum,
    int maximum)
{
    const int storedValue =
        settings.value(QString::fromLatin1(key), static_cast<int>(fallback)).toInt();
    if (storedValue < minimum || storedValue > maximum) {
        return fallback;
    }
    return static_cast<Enum>(storedValue);
}

int storedBoundedInt(
    const QSettings &settings,
    const char *key,
    int fallback,
    int minimum,
    int maximum)
{
    const int storedValue =
        settings.value(QString::fromLatin1(key), fallback).toInt();
    return storedValue >= minimum && storedValue <= maximum
        ? storedValue
        : fallback;
}

}

AppSettings::AppSettings(const QString &dataDirectory, QObject *parent)
    : QObject(parent)
    , m_dataDirectory(QDir::cleanPath(dataDirectory))
{
    m_interfaceStyle = storedEnum(
        m_settings,
        InterfaceStyleKey,
        Hybrid,
        static_cast<int>(MidnightCommand),
        static_cast<int>(Hybrid));
    m_language = storedEnum(
        m_settings,
        LanguageKey,
        English,
        static_cast<int>(English),
        static_cast<int>(French));
    m_colorMode = storedEnum(
        m_settings,
        ColorModeKey,
        Dark,
        static_cast<int>(Dark),
        static_cast<int>(Light));
    m_accentPreset = storedEnum(
        m_settings,
        AccentPresetKey,
        MatchInterface,
        static_cast<int>(MatchInterface),
        static_cast<int>(Green));
    m_dailyCapacityMinutes = storedBoundedInt(
        m_settings,
        DailyCapacityKey,
        DefaultDailyCapacityMinutes,
        60,
        960);
    m_defaultEstimatedMinutes = storedBoundedInt(
        m_settings,
        DefaultEstimateKey,
        DefaultTaskEstimateMinutes,
        5,
        480);
    m_defaultImportance = storedBoundedInt(
        m_settings,
        DefaultImportanceKey,
        DefaultTaskImportance,
        1,
        5);
    m_use24HourClock = m_settings.value(
        QString::fromLatin1(Use24HourClockKey),
        localeUses24HourClock()).toBool();
    m_showTimeline = m_settings.value(
        QString::fromLatin1(ShowTimelineKey),
        false).toBool();
    m_showPriorityReasons = m_settings.value(
        QString::fromLatin1(ShowPriorityReasonsKey),
        true).toBool();
    m_confirmTaskCompletion = m_settings.value(
        QString::fromLatin1(ConfirmCompletionKey),
        false).toBool();
}

AppSettings::InterfaceStyle AppSettings::interfaceStyle() const
{
    return m_interfaceStyle;
}

void AppSettings::setInterfaceStyle(InterfaceStyle style)
{
    if (style < MidnightCommand || style > Hybrid || m_interfaceStyle == style) {
        return;
    }
    m_interfaceStyle = style;
    m_settings.setValue(
        QString::fromLatin1(InterfaceStyleKey),
        static_cast<int>(style));
    emit interfaceStyleChanged();
}

AppSettings::Language AppSettings::language() const
{
    return m_language;
}

void AppSettings::setLanguage(Language language)
{
    if (language < English || language > French || m_language == language) {
        return;
    }
    m_language = language;
    m_settings.setValue(
        QString::fromLatin1(LanguageKey),
        static_cast<int>(language));
    emit languageChanged();
}

AppSettings::ColorMode AppSettings::colorMode() const
{
    return m_colorMode;
}

void AppSettings::setColorMode(ColorMode mode)
{
    if (mode < Dark || mode > Light || m_colorMode == mode) {
        return;
    }
    m_colorMode = mode;
    m_settings.setValue(
        QString::fromLatin1(ColorModeKey),
        static_cast<int>(mode));
    emit colorModeChanged();
}

AppSettings::AccentPreset AppSettings::accentPreset() const
{
    return m_accentPreset;
}

void AppSettings::setAccentPreset(AccentPreset preset)
{
    if (preset < MatchInterface || preset > Green || m_accentPreset == preset) {
        return;
    }
    m_accentPreset = preset;
    m_settings.setValue(
        QString::fromLatin1(AccentPresetKey),
        static_cast<int>(preset));
    emit accentPresetChanged();
}

int AppSettings::dailyCapacityMinutes() const
{
    return m_dailyCapacityMinutes;
}

void AppSettings::setDailyCapacityMinutes(int minutes)
{
    if (minutes < 60 || minutes > 960 || m_dailyCapacityMinutes == minutes) {
        return;
    }
    m_dailyCapacityMinutes = minutes;
    m_settings.setValue(QString::fromLatin1(DailyCapacityKey), minutes);
    emit dailyCapacityMinutesChanged();
}

int AppSettings::defaultEstimatedMinutes() const
{
    return m_defaultEstimatedMinutes;
}

void AppSettings::setDefaultEstimatedMinutes(int minutes)
{
    if (minutes < 5 || minutes > 480 || m_defaultEstimatedMinutes == minutes) {
        return;
    }
    m_defaultEstimatedMinutes = minutes;
    m_settings.setValue(QString::fromLatin1(DefaultEstimateKey), minutes);
    emit defaultEstimatedMinutesChanged();
}

int AppSettings::defaultImportance() const
{
    return m_defaultImportance;
}

void AppSettings::setDefaultImportance(int importance)
{
    if (importance < 1 || importance > 5 || m_defaultImportance == importance) {
        return;
    }
    m_defaultImportance = importance;
    m_settings.setValue(QString::fromLatin1(DefaultImportanceKey), importance);
    emit defaultImportanceChanged();
}

bool AppSettings::use24HourClock() const
{
    return m_use24HourClock;
}

void AppSettings::setUse24HourClock(bool enabled)
{
    if (m_use24HourClock == enabled) {
        return;
    }
    m_use24HourClock = enabled;
    m_settings.setValue(QString::fromLatin1(Use24HourClockKey), enabled);
    emit use24HourClockChanged();
}

bool AppSettings::showTimeline() const
{
    return m_showTimeline;
}

void AppSettings::setShowTimeline(bool enabled)
{
    if (m_showTimeline == enabled) {
        return;
    }
    m_showTimeline = enabled;
    m_settings.setValue(QString::fromLatin1(ShowTimelineKey), enabled);
    emit showTimelineChanged();
}

bool AppSettings::showPriorityReasons() const
{
    return m_showPriorityReasons;
}

void AppSettings::setShowPriorityReasons(bool enabled)
{
    if (m_showPriorityReasons == enabled) {
        return;
    }
    m_showPriorityReasons = enabled;
    m_settings.setValue(QString::fromLatin1(ShowPriorityReasonsKey), enabled);
    emit showPriorityReasonsChanged();
}

bool AppSettings::confirmTaskCompletion() const
{
    return m_confirmTaskCompletion;
}

void AppSettings::setConfirmTaskCompletion(bool enabled)
{
    if (m_confirmTaskCompletion == enabled) {
        return;
    }
    m_confirmTaskCompletion = enabled;
    m_settings.setValue(QString::fromLatin1(ConfirmCompletionKey), enabled);
    emit confirmTaskCompletionChanged();
}

QString AppSettings::dataDirectory() const
{
    return m_dataDirectory;
}

bool AppSettings::openDataDirectory() const
{
    if (m_dataDirectory.isEmpty() || !QDir().mkpath(m_dataDirectory)) {
        return false;
    }
    return QDesktopServices::openUrl(QUrl::fromLocalFile(m_dataDirectory));
}

void AppSettings::resetDefaults()
{
    m_settings.remove(QString::fromLatin1(InterfaceStyleKey));
    m_settings.remove(QString::fromLatin1(LanguageKey));
    m_settings.remove(QString::fromLatin1(ColorModeKey));
    m_settings.remove(QString::fromLatin1(AccentPresetKey));
    m_settings.remove(QString::fromLatin1(DailyCapacityKey));
    m_settings.remove(QString::fromLatin1(DefaultEstimateKey));
    m_settings.remove(QString::fromLatin1(DefaultImportanceKey));
    m_settings.remove(QString::fromLatin1(Use24HourClockKey));
    m_settings.remove(QString::fromLatin1(ShowTimelineKey));
    m_settings.remove(QString::fromLatin1(ShowPriorityReasonsKey));
    m_settings.remove(QString::fromLatin1(ConfirmCompletionKey));

    setInterfaceStyle(Hybrid);
    setLanguage(English);
    setColorMode(Dark);
    setAccentPreset(MatchInterface);
    setDailyCapacityMinutes(DefaultDailyCapacityMinutes);
    setDefaultEstimatedMinutes(DefaultTaskEstimateMinutes);
    setDefaultImportance(DefaultTaskImportance);
    setUse24HourClock(localeUses24HourClock());
    setShowTimeline(false);
    setShowPriorityReasons(true);
    setConfirmTaskCompletion(false);
    m_settings.sync();
    emit settingsReset();
}

bool AppSettings::localeUses24HourClock()
{
    return QLocale().timeFormat(QLocale::ShortFormat).contains(QLatin1Char('H'));
}
