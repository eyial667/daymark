// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/focussessionmodel.h"

#include <algorithm>

FocusSessionModel::FocusSessionModel(QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(1000);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &FocusSessionModel::tick);
}

QString FocusSessionModel::taskId() const
{
    return m_taskId;
}

QString FocusSessionModel::taskTitle() const
{
    return m_taskTitle;
}

bool FocusSessionModel::hasTask() const
{
    return !m_taskId.isEmpty() && !m_taskTitle.isEmpty();
}

bool FocusSessionModel::running() const
{
    return m_running;
}

bool FocusSessionModel::finished() const
{
    return m_finished;
}

int FocusSessionModel::totalSeconds() const
{
    return m_totalSeconds;
}

int FocusSessionModel::remainingSeconds() const
{
    return m_remainingSeconds;
}

QString FocusSessionModel::remainingText() const
{
    return formattedDuration(m_remainingSeconds);
}

QString FocusSessionModel::statusMessage() const
{
    return m_statusMessage;
}

bool FocusSessionModel::prepareTask(
    const QString &taskId,
    const QString &taskTitle,
    int estimatedMinutes)
{
    const QString cleanId = taskId.trimmed();
    const QString cleanTitle = taskTitle.trimmed();
    if (cleanId.isEmpty() || cleanTitle.isEmpty()) {
        setStatusMessage(QStringLiteral("Choose a task before starting focus."));
        return false;
    }
    if (m_running && cleanId != m_taskId) {
        setStatusMessage(QStringLiteral("Pause the current focus block before switching tasks."));
        return false;
    }
    if (cleanId == m_taskId && hasTask()) {
        return true;
    }

    m_timer.stop();
    m_taskId = cleanId;
    m_taskTitle = cleanTitle;
    m_totalSeconds = std::clamp(estimatedMinutes, 1, 480) * 60;
    m_remainingSeconds = m_totalSeconds;
    m_running = false;
    m_finished = false;
    setStatusMessage(QStringLiteral("Focus task ready."));
    emit stateChanged();
    return true;
}

bool FocusSessionModel::start()
{
    if (!hasTask()) {
        setStatusMessage(QStringLiteral("Choose a task before starting focus."));
        return false;
    }
    if (m_remainingSeconds <= 0) {
        m_remainingSeconds = m_totalSeconds;
    }
    if (m_running) {
        return true;
    }
    m_finished = false;
    m_running = true;
    m_timer.start();
    setStatusMessage(QStringLiteral("Focus timer started."));
    emit stateChanged();
    return true;
}

void FocusSessionModel::pause()
{
    if (!m_running) {
        return;
    }
    m_timer.stop();
    m_running = false;
    setStatusMessage(QStringLiteral("Focus timer paused."));
    emit stateChanged();
}

void FocusSessionModel::reset()
{
    if (!hasTask()) {
        return;
    }
    m_timer.stop();
    m_remainingSeconds = m_totalSeconds;
    m_running = false;
    m_finished = false;
    setStatusMessage(QStringLiteral("Focus timer reset."));
    emit stateChanged();
}

void FocusSessionModel::clear()
{
    m_timer.stop();
    m_taskId.clear();
    m_taskTitle.clear();
    m_totalSeconds = 0;
    m_remainingSeconds = 0;
    m_running = false;
    m_finished = false;
    setStatusMessage({});
    emit stateChanged();
}

void FocusSessionModel::clearStatus()
{
    setStatusMessage({});
}

void FocusSessionModel::tick()
{
    if (!m_running || m_remainingSeconds <= 0) {
        return;
    }
    --m_remainingSeconds;
    if (m_remainingSeconds == 0) {
        m_timer.stop();
        m_running = false;
        m_finished = true;
        setStatusMessage(QStringLiteral("Focus block complete."));
    }
    emit stateChanged();
}

void FocusSessionModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}

QString FocusSessionModel::formattedDuration(int seconds)
{
    const int safeSeconds = std::max(0, seconds);
    const int hours = safeSeconds / 3600;
    const int minutes = (safeSeconds % 3600) / 60;
    const int remaining = safeSeconds % 60;
    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(remaining, 2, 10, QLatin1Char('0'));
    }
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(remaining, 2, 10, QLatin1Char('0'));
}
