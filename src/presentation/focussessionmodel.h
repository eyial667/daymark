// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

class FocusSessionModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString taskId READ taskId NOTIFY stateChanged)
    Q_PROPERTY(QString taskTitle READ taskTitle NOTIFY stateChanged)
    Q_PROPERTY(bool hasTask READ hasTask NOTIFY stateChanged)
    Q_PROPERTY(bool running READ running NOTIFY stateChanged)
    Q_PROPERTY(bool finished READ finished NOTIFY stateChanged)
    Q_PROPERTY(int totalSeconds READ totalSeconds NOTIFY stateChanged)
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY stateChanged)
    Q_PROPERTY(QString remainingText READ remainingText NOTIFY stateChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit FocusSessionModel(QObject *parent = nullptr);

    [[nodiscard]] QString taskId() const;
    [[nodiscard]] QString taskTitle() const;
    [[nodiscard]] bool hasTask() const;
    [[nodiscard]] bool running() const;
    [[nodiscard]] bool finished() const;
    [[nodiscard]] int totalSeconds() const;
    [[nodiscard]] int remainingSeconds() const;
    [[nodiscard]] QString remainingText() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool prepareTask(
        const QString &taskId,
        const QString &taskTitle,
        int estimatedMinutes);
    Q_INVOKABLE bool start();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void clear();
    Q_INVOKABLE void clearStatus();

signals:
    void stateChanged();
    void statusMessageChanged();

private:
    void tick();
    void setStatusMessage(const QString &message);
    [[nodiscard]] static QString formattedDuration(int seconds);

    QTimer m_timer;
    QString m_taskId;
    QString m_taskTitle;
    int m_totalSeconds = 0;
    int m_remainingSeconds = 0;
    bool m_running = false;
    bool m_finished = false;
    QString m_statusMessage;
};
