// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "domain/category.h"
#include "domain/dailynote.h"
#include "domain/goal.h"
#include "domain/task.h"

#include <QSqlDatabase>
#include <QString>
#include <QVector>

#include <optional>

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
    [[nodiscard]] QVector<Task> allTasks(QString *errorMessage = nullptr) const;
    [[nodiscard]] bool addTask(const Task &task, QString *errorMessage = nullptr);
    [[nodiscard]] bool setCompleted(
        const QString &taskId,
        bool completed,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool setTaskCategory(
        const QString &taskId,
        const QString &categoryId,
        const QString &subcategoryId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool setTaskPlannedDate(
        const QString &taskId,
        const QDate &plannedDate,
        QString *errorMessage = nullptr);
    [[nodiscard]] std::optional<DailyNote> dailyNote(
        const QDate &date,
        QString *errorMessage = nullptr) const;
    [[nodiscard]] bool saveDailyNote(
        const DailyNote &note,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool pruneDailyNotes(
        const QDate &oldestDate,
        QString *errorMessage = nullptr);
    [[nodiscard]] QVector<Category> categories(QString *errorMessage = nullptr) const;
    [[nodiscard]] bool addCategory(
        const Category &category,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool updateCategory(
        const Category &category,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool addSubcategory(
        const Subcategory &subcategory,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool updateSubcategory(
        const Subcategory &subcategory,
        QString *errorMessage = nullptr);
    [[nodiscard]] QVector<Goal> goals(QString *errorMessage = nullptr) const;
    [[nodiscard]] bool addGoal(const Goal &goal, QString *errorMessage = nullptr);
    [[nodiscard]] bool addMilestone(
        const Milestone &milestone,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool setMilestoneCompleted(
        const QString &milestoneId,
        bool completed,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool setGoalCompleted(
        const QString &goalId,
        bool completed,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool mergeImportedData(
        const QVector<Category> &categories,
        const QVector<Task> &tasks,
        const QVector<Goal> &goals,
        QString *errorMessage = nullptr);

private:
    [[nodiscard]] bool migrate(QString *errorMessage);
    [[nodiscard]] bool execute(const QString &sql, QString *errorMessage) const;
    [[nodiscard]] QVector<Task> loadTasks(
        bool includeCompleted,
        QString *errorMessage) const;

    QString m_databasePath;
    QString m_connectionName;
    QSqlDatabase m_database;
};
