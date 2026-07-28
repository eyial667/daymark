// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class AppSettings;
class CategoryListModel;
class GoalListModel;
class MeetingListModel;
class TaskListModel;
class TaskRepository;

class DataTransferService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit DataTransferService(
        TaskRepository &repository,
        TaskListModel &taskModel,
        CategoryListModel &categoryModel,
        GoalListModel &goalModel,
        MeetingListModel &meetingModel,
        AppSettings &appSettings,
        QObject *parent = nullptr);

    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool exportData(const QUrl &destination);
    Q_INVOKABLE bool importData(const QUrl &source);
    Q_INVOKABLE void clearStatus();

signals:
    void statusMessageChanged();

private:
    void setStatusMessage(const QString &message);

    TaskRepository &m_repository;
    TaskListModel &m_taskModel;
    CategoryListModel &m_categoryModel;
    GoalListModel &m_goalModel;
    MeetingListModel &m_meetingModel;
    AppSettings &m_appSettings;
    QString m_statusMessage;
};
