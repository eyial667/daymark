// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "domain/task.h"

#include <QSqlDatabase>
#include <QString>
#include <QVector>

class TaskRepository
{
public:
    explicit TaskRepository(QString databasePath);
    ~TaskRepository();

    TaskRepository(const TaskRepository &) = delete;
    TaskRepository &operator=(const TaskRepository &) = delete;

    [[nodiscard]] bool open(QString *errorMessage = nullptr);
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] QVector<Task> openTasks(QString *errorMessage = nullptr) const;
    [[nodiscard]] bool addTask(const Task &task, QString *errorMessage = nullptr);
    [[nodiscard]] bool setCompleted(
        const QString &taskId,
        bool completed,
        QString *errorMessage = nullptr);

private:
    [[nodiscard]] bool migrate(QString *errorMessage);
    [[nodiscard]] bool execute(const QString &sql, QString *errorMessage) const;

    QString m_databasePath;
    QString m_connectionName;
    QSqlDatabase m_database;
};
