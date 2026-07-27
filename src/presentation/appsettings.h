// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class AppSettings final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(InterfaceStyle interfaceStyle READ interfaceStyle WRITE setInterfaceStyle NOTIFY interfaceStyleChanged)
    Q_PROPERTY(ColorMode colorMode READ colorMode WRITE setColorMode NOTIFY colorModeChanged)
    Q_PROPERTY(AccentPreset accentPreset READ accentPreset WRITE setAccentPreset NOTIFY accentPresetChanged)
    Q_PROPERTY(int dailyCapacityMinutes READ dailyCapacityMinutes WRITE setDailyCapacityMinutes NOTIFY dailyCapacityMinutesChanged)
    Q_PROPERTY(int defaultEstimatedMinutes READ defaultEstimatedMinutes WRITE setDefaultEstimatedMinutes NOTIFY defaultEstimatedMinutesChanged)
    Q_PROPERTY(int defaultImportance READ defaultImportance WRITE setDefaultImportance NOTIFY defaultImportanceChanged)
    Q_PROPERTY(bool use24HourClock READ use24HourClock WRITE setUse24HourClock NOTIFY use24HourClockChanged)
    Q_PROPERTY(bool showPriorityReasons READ showPriorityReasons WRITE setShowPriorityReasons NOTIFY showPriorityReasonsChanged)
    Q_PROPERTY(bool confirmTaskCompletion READ confirmTaskCompletion WRITE setConfirmTaskCompletion NOTIFY confirmTaskCompletionChanged)
    Q_PROPERTY(QString dataDirectory READ dataDirectory CONSTANT)

public:
    enum InterfaceStyle {
        MidnightCommand = 0,
        SpatialMap = 1,
        QuietFocus = 2,
        Hybrid = 3
    };
    Q_ENUM(InterfaceStyle)

    enum ColorMode {
        Dark = 0,
        Light = 1
    };
    Q_ENUM(ColorMode)

    enum AccentPreset {
        MatchInterface = 0,
        Violet = 1,
        Cyan = 2,
        Amber = 3,
        Green = 4
    };
    Q_ENUM(AccentPreset)

    explicit AppSettings(
        const QString &dataDirectory = QString(),
        QObject *parent = nullptr);

    [[nodiscard]] InterfaceStyle interfaceStyle() const;
    void setInterfaceStyle(InterfaceStyle style);

    [[nodiscard]] ColorMode colorMode() const;
    void setColorMode(ColorMode mode);

    [[nodiscard]] AccentPreset accentPreset() const;
    void setAccentPreset(AccentPreset preset);

    [[nodiscard]] int dailyCapacityMinutes() const;
    void setDailyCapacityMinutes(int minutes);

    [[nodiscard]] int defaultEstimatedMinutes() const;
    void setDefaultEstimatedMinutes(int minutes);

    [[nodiscard]] int defaultImportance() const;
    void setDefaultImportance(int importance);

    [[nodiscard]] bool use24HourClock() const;
    void setUse24HourClock(bool enabled);

    [[nodiscard]] bool showPriorityReasons() const;
    void setShowPriorityReasons(bool enabled);

    [[nodiscard]] bool confirmTaskCompletion() const;
    void setConfirmTaskCompletion(bool enabled);

    [[nodiscard]] QString dataDirectory() const;

    Q_INVOKABLE bool openDataDirectory() const;
    Q_INVOKABLE void resetDefaults();

signals:
    void interfaceStyleChanged();
    void colorModeChanged();
    void accentPresetChanged();
    void dailyCapacityMinutesChanged();
    void defaultEstimatedMinutesChanged();
    void defaultImportanceChanged();
    void use24HourClockChanged();
    void showPriorityReasonsChanged();
    void confirmTaskCompletionChanged();
    void settingsReset();

private:
    static constexpr auto InterfaceStyleKey = "appearance/interfaceStyle";
    static constexpr auto ColorModeKey = "appearance/colorMode";
    static constexpr auto AccentPresetKey = "appearance/accentPreset";
    static constexpr auto DailyCapacityKey = "planning/dailyCapacityMinutes";
    static constexpr auto DefaultEstimateKey = "tasks/defaultEstimatedMinutes";
    static constexpr auto DefaultImportanceKey = "tasks/defaultImportance";
    static constexpr auto Use24HourClockKey = "behavior/use24HourClock";
    static constexpr auto ShowPriorityReasonsKey = "behavior/showPriorityReasons";
    static constexpr auto ConfirmCompletionKey = "behavior/confirmTaskCompletion";

    static constexpr int DefaultDailyCapacityMinutes = 480;
    static constexpr int DefaultTaskEstimateMinutes = 30;
    static constexpr int DefaultTaskImportance = 3;

    [[nodiscard]] static bool localeUses24HourClock();

    QSettings m_settings;
    QString m_dataDirectory;
    InterfaceStyle m_interfaceStyle = Hybrid;
    ColorMode m_colorMode = Dark;
    AccentPreset m_accentPreset = MatchInterface;
    int m_dailyCapacityMinutes = DefaultDailyCapacityMinutes;
    int m_defaultEstimatedMinutes = DefaultTaskEstimateMinutes;
    int m_defaultImportance = DefaultTaskImportance;
    bool m_use24HourClock = false;
    bool m_showPriorityReasons = true;
    bool m_confirmTaskCompletion = false;
};
