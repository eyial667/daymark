// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class AppSettings;
class GoalListModel;
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
        GoalListModel &goalModel,
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
    GoalListModel &m_goalModel;
    AppSettings &m_appSettings;
    QString m_statusMessage;
};
