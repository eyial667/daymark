// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "data/taskrepository.h"

#include <QObject>
#include <QTimer>

class MeetingReminderService final : public QObject
{
    Q_OBJECT

public:
    explicit MeetingReminderService(
        TaskRepository &repository,
        QObject *parent = nullptr);

    void start();
    void processDueMeetings(const QDateTime &now);

public slots:
    void refresh();

signals:
    void notificationRequested(const QString &title, const QString &body);
    void remindersChanged();

private:
    void scheduleNextCheck(const QDateTime &now);

    TaskRepository &m_repository;
    QTimer m_timer;
};
