// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/meetingreminderservice.h"

#include <QLocale>

#include <algorithm>
#include <limits>

MeetingReminderService::MeetingReminderService(
    TaskRepository &repository,
    QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &MeetingReminderService::refresh);
}

void MeetingReminderService::start()
{
    refresh();
}

void MeetingReminderService::processDueMeetings(const QDateTime &now)
{
    QString errorMessage;
    const QVector<Meeting> meetings = m_repository.meetings(&errorMessage);
    if (!errorMessage.isEmpty()) {
        return;
    }

    bool changed = false;
    for (const Meeting &meeting : meetings) {
        if (meeting.notifiedAt.isValid() || meeting.startsAt <= now
            || meeting.startsAt.addSecs(-10 * 60) > now) {
            continue;
        }

        QString updateError;
        if (!m_repository.markMeetingNotified(meeting.id, now, &updateError)) {
            continue;
        }

        const QString startTime = QLocale().toString(meeting.startsAt.time(), QLocale::ShortFormat);
        const QString body = meeting.notes.isEmpty()
            ? tr("%1 starts at %2.").arg(meeting.title, startTime)
            : tr("%1 starts at %2.\n%3").arg(meeting.title, startTime, meeting.notes);
        emit notificationRequested(tr("Meeting reminder"), body);
        changed = true;
    }

    if (changed) {
        emit remindersChanged();
    }
}

void MeetingReminderService::refresh()
{
    const QDateTime now = QDateTime::currentDateTime();
    processDueMeetings(now);
    scheduleNextCheck(now);
}

void MeetingReminderService::scheduleNextCheck(const QDateTime &now)
{
    QString errorMessage;
    const QVector<Meeting> meetings = m_repository.meetings(&errorMessage);
    qint64 delay = 60 * 1000;
    if (errorMessage.isEmpty()) {
        for (const Meeting &meeting : meetings) {
            if (meeting.notifiedAt.isValid() || meeting.startsAt <= now) {
                continue;
            }
            const qint64 candidate = now.msecsTo(meeting.startsAt.addSecs(-10 * 60));
            if (candidate > 0) {
                delay = std::min(delay, candidate);
            } else {
                delay = 250;
                break;
            }
        }
    }

    delay = std::clamp<qint64>(
        delay,
        250,
        std::numeric_limits<int>::max());
    m_timer.start(static_cast<int>(delay));
}
