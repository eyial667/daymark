// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/tasklistmodel.h"

#include <QDate>
#include <QLocale>
#include <QTime>
#include <QTimeZone>
#include <QUuid>

#include <algorithm>

TaskListModel::TaskListModel(TaskRepository &repository, Scope scope, QObject *parent)
    : QAbstractListModel(parent)
    , m_repository(repository)
    , m_scope(scope)
{
    reload();
}

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant TaskListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }

    const Item &item = m_items.at(index.row());
    switch (role) {
    case IdRole:
        return item.task.id;
    case TitleRole:
        return item.task.title;
    case NotesRole:
        return item.task.notes;
    case DueTextRole:
        return formatDueDate(item.task.dueAt);
    case ImportanceRole:
        return item.task.importance;
    case EstimatedMinutesRole:
        return item.task.estimatedMinutes;
    case PriorityScoreRole:
        return item.priority.score;
    case PriorityReasonRole:
        return item.priority.reasons.join(QStringLiteral(" · "));
    case CategoryIdRole:
        return item.task.categoryId;
    case CategoryNameRole:
        return item.task.categoryName;
    case SubcategoryIdRole:
        return item.task.subcategoryId;
    case SubcategoryNameRole:
        return item.task.subcategoryName;
    default:
        return {};
    }
}

QHash<int, QByteArray> TaskListModel::roleNames() const
{
    return {
        {IdRole, "taskId"},
        {TitleRole, "title"},
        {NotesRole, "notes"},
        {DueTextRole, "dueText"},
        {ImportanceRole, "importance"},
        {EstimatedMinutesRole, "estimatedMinutes"},
        {PriorityScoreRole, "priorityScore"},
        {PriorityReasonRole, "priorityReason"},
        {CategoryIdRole, "categoryId"},
        {CategoryNameRole, "categoryName"},
        {SubcategoryIdRole, "subcategoryId"},
        {SubcategoryNameRole, "subcategoryName"},
    };
}

int TaskListModel::activeCount() const
{
    return m_items.size();
}

int TaskListModel::totalEstimatedMinutes() const
{
    int total = 0;
    for (const Item &item : m_items) {
        total += item.task.estimatedMinutes;
    }
    return total;
}

QString TaskListModel::plannedDuration() const
{
    const int total = totalEstimatedMinutes();
    const int hours = total / 60;
    const int minutes = total % 60;

    if (hours == 0) {
        return QStringLiteral("%1m").arg(minutes);
    }
    if (minutes == 0) {
        return QStringLiteral("%1h").arg(hours);
    }
    return QStringLiteral("%1h %2m").arg(hours).arg(minutes);
}

QString TaskListModel::topTaskTitle() const
{
    return m_items.isEmpty() ? QStringLiteral("Nothing queued") : m_items.first().task.title;
}

bool TaskListModel::hasBacklogSuggestion() const
{
    return !m_backlogSuggestionTaskId.isEmpty();
}

QString TaskListModel::backlogSuggestionTitle() const
{
    return m_backlogSuggestionTitle;
}

QString TaskListModel::backlogSuggestionDetail() const
{
    return m_backlogSuggestionDetail;
}

QString TaskListModel::statusMessage() const
{
    return m_statusMessage;
}

bool TaskListModel::addTask(
    const QString &title,
    const QString &dueDate,
    int importance,
    int estimatedMinutes,
    const QString &categoryId,
    const QString &subcategoryId,
    bool planForToday)
{
    const QString cleanTitle = title.trimmed();
    if (cleanTitle.isEmpty()) {
        setStatusMessage(QStringLiteral("A task needs a title."));
        return false;
    }

    QDateTime dueAt;
    if (!dueDate.trimmed().isEmpty()) {
        const QDate parsedDate = QDate::fromString(dueDate.trimmed(), Qt::ISODate);
        if (!parsedDate.isValid()) {
            setStatusMessage(QStringLiteral("Use YYYY-MM-DD for the due date."));
            return false;
        }
        dueAt = QDateTime(parsedDate, QTime(23, 59), QTimeZone::LocalTime);
    }

    Task task;
    task.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    task.title = cleanTitle;
    task.categoryId = categoryId.trimmed();
    task.subcategoryId = subcategoryId.trimmed();
    task.dueAt = dueAt;
    task.plannedDate = planForToday ? QDate::currentDate() : QDate();
    task.createdAt = QDateTime::currentDateTime();
    task.importance = std::clamp(importance, 1, 5);
    task.estimatedMinutes = std::clamp(estimatedMinutes, 5, 480);

    QString errorMessage;
    if (!m_repository.addTask(task, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not save the task: %1").arg(errorMessage));
        return false;
    }

    reload();
    emit tasksChanged();
    setStatusMessage(QStringLiteral("Task added."));
    return true;
}

bool TaskListModel::assignTaskCategory(
    const QString &taskId,
    const QString &categoryId,
    const QString &subcategoryId)
{
    const auto item = std::find_if(
        m_items.cbegin(),
        m_items.cend(),
        [&taskId](const Item &candidate) { return candidate.task.id == taskId; });
    if (item == m_items.cend()) {
        setStatusMessage(QStringLiteral("That task is no longer in the list."));
        return false;
    }

    QString errorMessage;
    if (!m_repository.setTaskCategory(
            taskId,
            categoryId.trimmed(),
            subcategoryId.trimmed(),
            &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not assign the category: %1").arg(errorMessage));
        return false;
    }
    reload();
    emit tasksChanged();
    setStatusMessage(categoryId.trimmed().isEmpty()
            ? QStringLiteral("Category removed from task.")
            : QStringLiteral("Task category updated."));
    return true;
}

bool TaskListModel::completeTask(int row)
{
    if (row < 0 || row >= m_items.size()) {
        setStatusMessage(QStringLiteral("That task is no longer in the list."));
        return false;
    }

    QString errorMessage;
    if (!m_repository.setCompleted(m_items.at(row).task.id, true, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not complete the task: %1").arg(errorMessage));
        return false;
    }

    reload();
    emit tasksChanged();
    setStatusMessage(QStringLiteral("Task completed."));
    return true;
}

bool TaskListModel::planSuggestedTaskForToday()
{
    if (m_backlogSuggestionTaskId.isEmpty()) {
        setStatusMessage(QStringLiteral("There is no To-do task available to plan."));
        return false;
    }

    const QString suggestionTitle = m_backlogSuggestionTitle;
    QString errorMessage;
    if (!m_repository.setTaskPlannedDate(
            m_backlogSuggestionTaskId,
            QDate::currentDate(),
            &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not plan the task: %1").arg(errorMessage));
        return false;
    }

    reload();
    emit tasksChanged();
    setStatusMessage(QStringLiteral("Added “%1” to Today.").arg(suggestionTitle));
    return true;
}

void TaskListModel::clearStatus()
{
    setStatusMessage({});
}

void TaskListModel::reload()
{
    QString errorMessage;
    const QVector<Task> tasks = m_repository.openTasks(&errorMessage);

    QVector<Item> ranked;
    ranked.reserve(tasks.size());
    const QDateTime now = QDateTime::currentDateTime();
    for (const Task &task : tasks) {
        ranked.append({task, PriorityEngine::evaluate(task, now)});
    }

    std::stable_sort(ranked.begin(), ranked.end(), [](const Item &left, const Item &right) {
        if (left.priority.score != right.priority.score) {
            return left.priority.score > right.priority.score;
        }
        if (left.task.dueAt.isValid() != right.task.dueAt.isValid()) {
            return left.task.dueAt.isValid();
        }
        if (left.task.dueAt.isValid() && left.task.dueAt != right.task.dueAt) {
            return left.task.dueAt < right.task.dueAt;
        }
        return left.task.createdAt < right.task.createdAt;
    });

    QVector<Item> refreshed;
    m_backlogSuggestionTaskId.clear();
    m_backlogSuggestionTitle.clear();
    m_backlogSuggestionDetail.clear();
    if (m_scope == Today) {
        refreshed.reserve(ranked.size());
        const QDate today = QDate::currentDate();
        for (const Item &item : ranked) {
            if (belongsToToday(item.task, today)) {
                refreshed.append(item);
                continue;
            }
            if (m_backlogSuggestionTaskId.isEmpty()) {
                m_backlogSuggestionTaskId = item.task.id;
                m_backlogSuggestionTitle = item.task.title;
                m_backlogSuggestionDetail = QStringLiteral("%1 · %2")
                    .arg(
                        formatDueDate(item.task.dueAt),
                        item.priority.reasons.join(QStringLiteral(" · ")));
            }
        }
    } else {
        refreshed = std::move(ranked);
    }

    beginResetModel();
    m_items = std::move(refreshed);
    endResetModel();
    emit summaryChanged();

    if (!errorMessage.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not load tasks: %1").arg(errorMessage));
    }
}

bool TaskListModel::belongsToToday(const Task &task, const QDate &today)
{
    return task.plannedDate == today
        || (task.dueAt.isValid() && task.dueAt.date() <= today);
}

void TaskListModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}

QString TaskListModel::formatDueDate(const QDateTime &dueAt)
{
    if (!dueAt.isValid()) {
        return QStringLiteral("No deadline");
    }

    const QDate today = QDate::currentDate();
    const int days = today.daysTo(dueAt.date());
    if (days < 0) {
        return QStringLiteral("Overdue · %1")
            .arg(QLocale().toString(dueAt.date(), QLocale::ShortFormat));
    }
    if (days == 0) {
        return QStringLiteral("Due today");
    }
    if (days == 1) {
        return QStringLiteral("Due tomorrow");
    }

    return QStringLiteral("Due %1")
        .arg(QLocale().toString(dueAt.date(), QLocale::ShortFormat));
}
