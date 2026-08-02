// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "domain/category.h"
#include "domain/dailynote.h"
#include "domain/goal.h"
#include "domain/meeting.h"
#include "domain/mentalmap.h"
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
    [[nodiscard]] bool deleteTask(
        const QString &taskId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool setCompleted(
        const QString &taskId,
        bool completed,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool updateTask(
        const QString &taskId,
        const QString &title,
        const QString &notes,
        const QDateTime &dueAt,
        int importance,
        int estimatedMinutes,
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
    [[nodiscard]] bool deleteDailyNote(
        const QDate &date,
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
    [[nodiscard]] bool deleteCategory(
        const QString &categoryId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool addSubcategory(
        const Subcategory &subcategory,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool updateSubcategory(
        const Subcategory &subcategory,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool deleteSubcategory(
        const QString &subcategoryId,
        QString *errorMessage = nullptr);
    [[nodiscard]] QVector<Goal> goals(QString *errorMessage = nullptr) const;
    [[nodiscard]] bool addGoal(const Goal &goal, QString *errorMessage = nullptr);
    [[nodiscard]] bool deleteGoal(
        const QString &goalId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool addMilestone(
        const Milestone &milestone,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool deleteMilestone(
        const QString &milestoneId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool setMilestoneCompleted(
        const QString &milestoneId,
        bool completed,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool setGoalCompleted(
        const QString &goalId,
        bool completed,
        QString *errorMessage = nullptr);
    [[nodiscard]] QVector<Meeting> meetings(QString *errorMessage = nullptr) const;
    [[nodiscard]] bool addMeeting(
        const Meeting &meeting,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool deleteMeeting(
        const QString &meetingId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool markMeetingNotified(
        const QString &meetingId,
        const QDateTime &notifiedAt,
        QString *errorMessage = nullptr);
    [[nodiscard]] QVector<MentalMapGroup> mentalMapGroups(
        QString *errorMessage = nullptr) const;
    [[nodiscard]] QVector<MentalMapNote> mentalMapNotes(
        QString *errorMessage = nullptr) const;
    [[nodiscard]] QVector<MentalMapConnection> mentalMapConnections(
        QString *errorMessage = nullptr) const;
    [[nodiscard]] bool addMentalMapGroup(
        const MentalMapGroup &group,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool updateMentalMapGroup(
        const MentalMapGroup &group,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool deleteMentalMapGroup(
        const QString &groupId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool addMentalMapNote(
        const MentalMapNote &note,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool updateMentalMapNote(
        const MentalMapNote &note,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool deleteMentalMapNote(
        const QString &noteId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool addMentalMapConnection(
        const MentalMapConnection &connection,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool deleteMentalMapConnection(
        const QString &connectionId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool linkMentalMapNoteToTask(
        const QString &noteId,
        const QString &taskId,
        QString *errorMessage = nullptr);
    [[nodiscard]] bool mergeImportedData(
        const QVector<Category> &categories,
        const QVector<Task> &tasks,
        const QVector<Goal> &goals,
        const QVector<Meeting> &meetings,
        const QVector<MentalMapGroup> &mapGroups,
        const QVector<MentalMapNote> &mapNotes,
        const QVector<MentalMapConnection> &mapConnections,
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
