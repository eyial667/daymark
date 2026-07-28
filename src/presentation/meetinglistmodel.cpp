// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/meetinglistmodel.h"

#include "presentation/appsettings.h"

#include <QDate>
#include <QLocale>
#include <QTime>
#include <QTimeZone>
#include <QUuid>

MeetingListModel::MeetingListModel(
    TaskRepository &repository,
    AppSettings &appSettings,
    QObject *parent)
    : QAbstractListModel(parent)
    , m_repository(repository)
    , m_appSettings(appSettings)
{
    connect(&m_appSettings, &AppSettings::use24HourClockChanged, this, [this] {
        if (!m_meetings.isEmpty()) {
            emit dataChanged(
                index(0),
                index(m_meetings.size() - 1),
                {StartsAtTextRole, ReminderTextRole});
        }
        emit summaryChanged();
    });
    reload();
}

int MeetingListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_meetings.size();
}

QVariant MeetingListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_meetings.size()) {
        return {};
    }

    const Meeting &meeting = m_meetings.at(index.row());
    switch (role) {
    case IdRole:
        return meeting.id;
    case TitleRole:
        return meeting.title;
    case NotesRole:
        return meeting.notes;
    case StartsAtRole:
        return meeting.startsAt;
    case StartsAtTextRole:
        return formatStartsAt(meeting.startsAt);
    case ReminderTextRole:
        return formatReminder(meeting);
    default:
        return {};
    }
}

QHash<int, QByteArray> MeetingListModel::roleNames() const
{
    return {
        {IdRole, "meetingId"},
        {TitleRole, "title"},
        {NotesRole, "notes"},
        {StartsAtRole, "startsAt"},
        {StartsAtTextRole, "startsAtText"},
        {ReminderTextRole, "reminderText"},
    };
}

int MeetingListModel::meetingCount() const
{
    return m_meetings.size();
}

QString MeetingListModel::nextMeetingTitle() const
{
    return m_meetings.isEmpty() ? tr("No meetings scheduled") : m_meetings.first().title;
}

QString MeetingListModel::nextMeetingWhen() const
{
    return m_meetings.isEmpty() ? tr("Your calendar is clear")
                                : formatStartsAt(m_meetings.first().startsAt);
}

QString MeetingListModel::statusMessage() const
{
    return m_statusMessage;
}

bool MeetingListModel::addMeeting(
    const QString &title,
    const QString &date,
    const QString &time,
    const QString &notes)
{
    const QString cleanTitle = title.trimmed();
    if (cleanTitle.isEmpty()) {
        setStatusMessage(tr("A meeting needs a title."));
        return false;
    }

    const QDate parsedDate = QDate::fromString(date.trimmed(), Qt::ISODate);
    if (!parsedDate.isValid()) {
        setStatusMessage(tr("Use YYYY-MM-DD for the meeting date."));
        return false;
    }

    const QTime parsedTime = QTime::fromString(time.trimmed(), QStringLiteral("HH:mm"));
    if (!parsedTime.isValid() || time.trimmed().size() != 5) {
        setStatusMessage(tr("Use HH:MM for the meeting time."));
        return false;
    }

    const QDateTime startsAt(parsedDate, parsedTime, QTimeZone::LocalTime);
    if (!startsAt.isValid() || startsAt <= QDateTime::currentDateTime()) {
        setStatusMessage(tr("Choose a meeting time in the future."));
        return false;
    }

    Meeting meeting;
    meeting.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    meeting.title = cleanTitle;
    meeting.notes = notes.trimmed();
    meeting.startsAt = startsAt;
    meeting.createdAt = QDateTime::currentDateTime();

    QString errorMessage;
    if (!m_repository.addMeeting(meeting, &errorMessage)) {
        setStatusMessage(tr("Could not save the meeting: %1").arg(errorMessage));
        return false;
    }

    reload();
    emit meetingsChanged();
    setStatusMessage(startsAt.addSecs(-10 * 60) <= QDateTime::currentDateTime()
            ? tr("Meeting added. Its reminder will be sent now.")
            : tr("Meeting added. A local reminder is scheduled for 10 minutes before it starts."));
    return true;
}

bool MeetingListModel::removeMeeting(int row)
{
    if (row < 0 || row >= m_meetings.size()) {
        setStatusMessage(tr("That meeting is no longer in the list."));
        return false;
    }

    const QString title = m_meetings.at(row).title;
    QString errorMessage;
    if (!m_repository.deleteMeeting(m_meetings.at(row).id, &errorMessage)) {
        setStatusMessage(tr("Could not remove the meeting: %1").arg(errorMessage));
        return false;
    }

    reload();
    emit meetingsChanged();
    setStatusMessage(tr("Removed “%1”.").arg(title));
    return true;
}

void MeetingListModel::clearStatus()
{
    setStatusMessage({});
}

void MeetingListModel::reload()
{
    QString errorMessage;
    const QVector<Meeting> storedMeetings = m_repository.meetings(&errorMessage);
    const QDateTime now = QDateTime::currentDateTime();
    QVector<Meeting> upcoming;
    upcoming.reserve(storedMeetings.size());
    for (const Meeting &meeting : storedMeetings) {
        if (meeting.startsAt > now) {
            upcoming.append(meeting);
        }
    }

    beginResetModel();
    m_meetings = std::move(upcoming);
    endResetModel();
    emit summaryChanged();

    if (!errorMessage.isEmpty()) {
        setStatusMessage(tr("Could not load meetings: %1").arg(errorMessage));
    }
}

void MeetingListModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}

QString MeetingListModel::formatStartsAt(const QDateTime &startsAt) const
{
    const QLocale locale;
    const QString date = locale.toString(startsAt.date(), QLocale::LongFormat);
    const QString time = locale.toString(
        startsAt.time(),
        m_appSettings.use24HourClock() ? QStringLiteral("HH:mm") : QStringLiteral("h:mm AP"));
    return tr("%1 at %2").arg(date, time);
}

QString MeetingListModel::formatReminder(const Meeting &meeting) const
{
    if (meeting.notifiedAt.isValid()) {
        return tr("Reminder sent");
    }
    const QDateTime reminderAt = meeting.startsAt.addSecs(-10 * 60);
    if (reminderAt <= QDateTime::currentDateTime()) {
        return tr("Reminder due now");
    }
    const QLocale locale;
    const QString time = locale.toString(
        reminderAt.time(),
        m_appSettings.use24HourClock() ? QStringLiteral("HH:mm") : QStringLiteral("h:mm AP"));
    return tr("Reminder at %1").arg(time);
}
