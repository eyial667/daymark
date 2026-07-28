// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "data/taskrepository.h"

#include <QAbstractListModel>
#include <QVector>

class AppSettings;

class MeetingListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int meetingCount READ meetingCount NOTIFY summaryChanged)
    Q_PROPERTY(QString nextMeetingTitle READ nextMeetingTitle NOTIFY summaryChanged)
    Q_PROPERTY(QString nextMeetingWhen READ nextMeetingWhen NOTIFY summaryChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        NotesRole,
        StartsAtRole,
        StartsAtTextRole,
        ReminderTextRole
    };
    Q_ENUM(Role)

    explicit MeetingListModel(
        TaskRepository &repository,
        AppSettings &appSettings,
        QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int meetingCount() const;
    [[nodiscard]] QString nextMeetingTitle() const;
    [[nodiscard]] QString nextMeetingWhen() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool addMeeting(
        const QString &title,
        const QString &date,
        const QString &time,
        const QString &notes = {});
    Q_INVOKABLE bool removeMeeting(int row);
    Q_INVOKABLE void clearStatus();
    Q_INVOKABLE void reload();

signals:
    void summaryChanged();
    void statusMessageChanged();
    void meetingsChanged();

private:
    void setStatusMessage(const QString &message);
    [[nodiscard]] QString formatStartsAt(const QDateTime &startsAt) const;
    [[nodiscard]] QString formatReminder(const Meeting &meeting) const;

    TaskRepository &m_repository;
    AppSettings &m_appSettings;
    QVector<Meeting> m_meetings;
    QString m_statusMessage;
};
