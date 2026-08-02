// SPDX-License-Identifier: GPL-3.0-or-later

#include "data/taskrepository.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaType>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace {

QString serializeDateTime(const QDateTime &dateTime)
{
    return dateTime.isValid() ? dateTime.toUTC().toString(Qt::ISODateWithMs) : QString();
}

QDateTime deserializeDateTime(const QString &value)
{
    if (value.isEmpty()) {
        return {};
    }

    return QDateTime::fromString(value, Qt::ISODateWithMs).toLocalTime();
}

QString serializeDate(const QDate &date)
{
    return date.isValid() ? date.toString(Qt::ISODate) : QString();
}

QDate deserializeDate(const QString &value)
{
    return value.isEmpty() ? QDate() : QDate::fromString(value, Qt::ISODate);
}

void assignError(QString *target, const QString &message)
{
    if (target != nullptr) {
        *target = message;
    }
}

void bindNullableString(QSqlQuery &query, const QString &placeholder, const QString &value)
{
    if (value.isEmpty()) {
        query.bindValue(placeholder, QVariant(QMetaType::fromType<QString>()));
    } else {
        query.bindValue(placeholder, value);
    }
}

void bindNullableDateTime(
    QSqlQuery &query,
    const QString &placeholder,
    const QDateTime &value)
{
    bindNullableString(query, placeholder, serializeDateTime(value));
}

void bindNullableDate(QSqlQuery &query, const QString &placeholder, const QDate &value)
{
    bindNullableString(query, placeholder, serializeDate(value));
}

Task taskFromQuery(const QSqlQuery &query)
{
    Task task;
    task.id = query.value(0).toString();
    task.title = query.value(1).toString();
    task.notes = query.value(2).toString();
    task.project = query.value(3).toString();
    task.dueAt = deserializeDateTime(query.value(4).toString());
    task.createdAt = deserializeDateTime(query.value(5).toString());
    task.importance = query.value(6).toInt();
    task.estimatedMinutes = query.value(7).toInt();
    task.completed = query.value(8).toBool();
    task.completedAt = deserializeDateTime(query.value(9).toString());
    task.categoryId = query.value(10).toString();
    task.categoryName = query.value(11).toString();
    task.subcategoryId = query.value(12).toString();
    task.subcategoryName = query.value(13).toString();
    task.plannedDate = deserializeDate(query.value(14).toString());
    return task;
}

void bindTask(QSqlQuery &query, const Task &task)
{
    query.bindValue(QStringLiteral(":id"), task.id);
    query.bindValue(QStringLiteral(":title"), task.title);
    query.bindValue(
        QStringLiteral(":notes"),
        task.notes.isNull() ? QStringLiteral("") : task.notes);
    query.bindValue(
        QStringLiteral(":project"),
        task.project.isNull() ? QStringLiteral("") : task.project);
    bindNullableDateTime(query, QStringLiteral(":due_at"), task.dueAt);
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(task.createdAt));
    query.bindValue(QStringLiteral(":importance"), task.importance);
    query.bindValue(QStringLiteral(":estimated_minutes"), task.estimatedMinutes);
    query.bindValue(QStringLiteral(":is_completed"), task.completed);
    bindNullableDateTime(query, QStringLiteral(":completed_at"), task.completedAt);
    bindNullableString(query, QStringLiteral(":category_id"), task.categoryId);
    bindNullableString(query, QStringLiteral(":subcategory_id"), task.subcategoryId);
    bindNullableDate(query, QStringLiteral(":planned_date"), task.plannedDate);
}

void bindCategory(QSqlQuery &query, const Category &category)
{
    query.bindValue(QStringLiteral(":id"), category.id);
    query.bindValue(QStringLiteral(":name"), category.name);
    query.bindValue(
        QStringLiteral(":notes"),
        category.notes.isNull() ? QStringLiteral("") : category.notes);
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(category.createdAt));
}

void bindSubcategory(QSqlQuery &query, const Subcategory &subcategory)
{
    query.bindValue(QStringLiteral(":id"), subcategory.id);
    query.bindValue(QStringLiteral(":category_id"), subcategory.categoryId);
    query.bindValue(QStringLiteral(":name"), subcategory.name);
    query.bindValue(
        QStringLiteral(":notes"),
        subcategory.notes.isNull() ? QStringLiteral("") : subcategory.notes);
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(subcategory.createdAt));
}

void bindGoal(QSqlQuery &query, const Goal &goal)
{
    query.bindValue(QStringLiteral(":id"), goal.id);
    query.bindValue(QStringLiteral(":title"), goal.title);
    query.bindValue(
        QStringLiteral(":description"),
        goal.description.isNull() ? QStringLiteral("") : goal.description);
    bindNullableDate(query, QStringLiteral(":target_date"), goal.targetDate);
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(goal.createdAt));
    query.bindValue(QStringLiteral(":is_completed"), goal.completed);
    bindNullableDateTime(
        query,
        QStringLiteral(":completed_at"),
        goal.completed ? QDateTime::currentDateTime() : QDateTime());
}

void bindMilestone(QSqlQuery &query, const Milestone &milestone)
{
    query.bindValue(QStringLiteral(":id"), milestone.id);
    query.bindValue(QStringLiteral(":goal_id"), milestone.goalId);
    query.bindValue(QStringLiteral(":title"), milestone.title);
    bindNullableDate(query, QStringLiteral(":target_date"), milestone.targetDate);
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(milestone.createdAt));
    query.bindValue(QStringLiteral(":is_completed"), milestone.completed);
    bindNullableDateTime(
        query,
        QStringLiteral(":completed_at"),
        milestone.completed ? QDateTime::currentDateTime() : QDateTime());
}

void bindMeeting(QSqlQuery &query, const Meeting &meeting)
{
    query.bindValue(QStringLiteral(":id"), meeting.id);
    query.bindValue(QStringLiteral(":title"), meeting.title);
    query.bindValue(
        QStringLiteral(":notes"),
        meeting.notes.isNull() ? QStringLiteral("") : meeting.notes);
    query.bindValue(QStringLiteral(":starts_at"), serializeDateTime(meeting.startsAt));
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(meeting.createdAt));
    bindNullableDateTime(query, QStringLiteral(":notified_at"), meeting.notifiedAt);
}

QString serializeChecklist(const QVector<MentalMapChecklistItem> &items)
{
    QJsonArray array;
    for (const MentalMapChecklistItem &item : items) {
        array.append(QJsonObject {
            {QStringLiteral("text"), item.text},
            {QStringLiteral("completed"), item.completed},
        });
    }
    return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QVector<MentalMapChecklistItem> deserializeChecklist(const QString &value)
{
    QVector<MentalMapChecklistItem> result;
    const QJsonDocument document = QJsonDocument::fromJson(value.toUtf8());
    if (!document.isArray()) {
        return result;
    }
    for (const QJsonValue &entry : document.array()) {
        const QJsonObject object = entry.toObject();
        const QString text = object.value(QStringLiteral("text")).toString().trimmed();
        if (!text.isEmpty()) {
            result.append({text, object.value(QStringLiteral("completed")).toBool()});
        }
    }
    return result;
}

bool deleteById(
    QSqlDatabase database,
    const QString &table,
    const QString &id,
    const QString &missingMessage,
    QString *errorMessage)
{
    QSqlQuery query(database);
    query.prepare(QStringLiteral("DELETE FROM %1 WHERE id = :id").arg(table));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, missingMessage);
        return false;
    }
    return true;
}

} // namespace

TaskRepository::TaskRepository(QString databasePath)
    : m_databasePath(std::move(databasePath))
    , m_connectionName(QStringLiteral("daymark-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)))
{
}

TaskRepository::~TaskRepository()
{
    if (m_database.isOpen()) {
        m_database.close();
    }

    m_database = {};
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool TaskRepository::open(QString *errorMessage)
{
    if (isOpen()) {
        return true;
    }

    if (m_databasePath != QStringLiteral(":memory:")) {
        const QFileInfo databaseFile(m_databasePath);
        if (!QDir().mkpath(databaseFile.absolutePath())) {
            assignError(errorMessage, QStringLiteral("Could not create the application data directory."));
            return false;
        }
    }

    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(m_databasePath);

    if (!m_database.open()) {
        assignError(errorMessage, m_database.lastError().text());
        return false;
    }

    if (!execute(QStringLiteral("PRAGMA foreign_keys = ON"), errorMessage)
        || !execute(QStringLiteral("PRAGMA busy_timeout = 5000"), errorMessage)) {
        return false;
    }

    if (m_databasePath != QStringLiteral(":memory:")
        && !execute(QStringLiteral("PRAGMA journal_mode = WAL"), errorMessage)) {
        return false;
    }

    return migrate(errorMessage);
}

bool TaskRepository::isOpen() const
{
    return m_database.isValid() && m_database.isOpen();
}

QVector<Task> TaskRepository::openTasks(QString *errorMessage) const
{
    return loadTasks(false, errorMessage);
}

QVector<Task> TaskRepository::allTasks(QString *errorMessage) const
{
    return loadTasks(true, errorMessage);
}

QVector<Task> TaskRepository::loadTasks(
    bool includeCompleted,
    QString *errorMessage) const
{
    QVector<Task> tasks;
    QSqlQuery query(m_database);
    const QString whereClause = includeCompleted
        ? QString()
        : QStringLiteral(" WHERE t.is_completed = 0");

    if (!query.exec(QStringLiteral(
            "SELECT t.id, t.title, t.notes, t.project, t.due_at, t.created_at, "
            "t.importance, t.estimated_minutes, t.is_completed, t.completed_at, "
            "t.category_id, COALESCE(c.name, ''), t.subcategory_id, "
            "COALESCE(s.name, ''), t.planned_date FROM tasks t "
            "LEFT JOIN categories c ON c.id = t.category_id "
            "LEFT JOIN subcategories s ON s.id = t.subcategory_id")
            + whereClause
            + QStringLiteral(" ORDER BY t.created_at ASC"))) {
        assignError(errorMessage, query.lastError().text());
        return tasks;
    }

    while (query.next()) {
        tasks.append(taskFromQuery(query));
    }

    return tasks;
}

bool TaskRepository::addTask(const Task &task, QString *errorMessage)
{
    if (task.categoryId.isEmpty() && !task.subcategoryId.isEmpty()) {
        assignError(errorMessage, QStringLiteral("A subcategory requires its parent category."));
        return false;
    }
    if (!task.subcategoryId.isEmpty()) {
        QSqlQuery validationQuery(m_database);
        validationQuery.prepare(QStringLiteral(
            "SELECT 1 FROM subcategories WHERE id = :id AND category_id = :category_id"));
        validationQuery.bindValue(QStringLiteral(":id"), task.subcategoryId);
        validationQuery.bindValue(QStringLiteral(":category_id"), task.categoryId);
        if (!validationQuery.exec() || !validationQuery.next()) {
            assignError(
                errorMessage,
                validationQuery.lastError().isValid()
                    ? validationQuery.lastError().text()
                    : QStringLiteral("The selected subcategory does not belong to that category."));
            return false;
        }
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO tasks "
        "(id, title, notes, project, due_at, created_at, importance, estimated_minutes, "
        "is_completed, completed_at, category_id, subcategory_id, planned_date) "
        "VALUES (:id, :title, :notes, :project, :due_at, :created_at, :importance, "
        ":estimated_minutes, :is_completed, :completed_at, :category_id, "
        ":subcategory_id, :planned_date)"));
    bindTask(query, task);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }

    return true;
}

bool TaskRepository::deleteTask(const QString &taskId, QString *errorMessage)
{
    return deleteById(
        m_database,
        QStringLiteral("tasks"),
        taskId,
        QStringLiteral("The selected task no longer exists."),
        errorMessage);
}

bool TaskRepository::setCompleted(
    const QString &taskId,
    bool completed,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE tasks SET is_completed = :completed, completed_at = :completed_at "
        "WHERE id = :id"));
    query.bindValue(QStringLiteral(":completed"), completed);
    bindNullableDateTime(
        query,
        QStringLiteral(":completed_at"),
        completed ? QDateTime::currentDateTime() : QDateTime());
    query.bindValue(QStringLiteral(":id"), taskId);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }

    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected task no longer exists."));
        return false;
    }

    return true;
}

bool TaskRepository::updateTask(
    const QString &taskId,
    const QString &title,
    const QString &notes,
    const QDateTime &dueAt,
    int importance,
    int estimatedMinutes,
    QString *errorMessage)
{
    if (title.isEmpty()) {
        assignError(errorMessage, QStringLiteral("A task needs a title."));
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE tasks SET title = :title, notes = :notes, due_at = :due_at, "
        "importance = :importance, estimated_minutes = :estimated_minutes "
        "WHERE id = :id"));
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(
        QStringLiteral(":notes"),
        notes.isNull() ? QStringLiteral("") : notes);
    bindNullableDateTime(query, QStringLiteral(":due_at"), dueAt);
    query.bindValue(QStringLiteral(":importance"), importance);
    query.bindValue(QStringLiteral(":estimated_minutes"), estimatedMinutes);
    query.bindValue(QStringLiteral(":id"), taskId);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected task no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::setTaskCategory(
    const QString &taskId,
    const QString &categoryId,
    const QString &subcategoryId,
    QString *errorMessage)
{
    if (categoryId.isEmpty() && !subcategoryId.isEmpty()) {
        assignError(errorMessage, QStringLiteral("A subcategory requires its parent category."));
        return false;
    }
    if (!subcategoryId.isEmpty()) {
        QSqlQuery validationQuery(m_database);
        validationQuery.prepare(QStringLiteral(
            "SELECT 1 FROM subcategories WHERE id = :id AND category_id = :category_id"));
        validationQuery.bindValue(QStringLiteral(":id"), subcategoryId);
        validationQuery.bindValue(QStringLiteral(":category_id"), categoryId);
        if (!validationQuery.exec() || !validationQuery.next()) {
            assignError(
                errorMessage,
                validationQuery.lastError().isValid()
                    ? validationQuery.lastError().text()
                    : QStringLiteral("The selected subcategory does not belong to that category."));
            return false;
        }
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE tasks SET category_id = :category_id, subcategory_id = :subcategory_id "
        "WHERE id = :id"));
    bindNullableString(query, QStringLiteral(":category_id"), categoryId);
    bindNullableString(query, QStringLiteral(":subcategory_id"), subcategoryId);
    query.bindValue(QStringLiteral(":id"), taskId);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected task no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::setTaskPlannedDate(
    const QString &taskId,
    const QDate &plannedDate,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE tasks SET planned_date = :planned_date WHERE id = :id"));
    bindNullableDate(query, QStringLiteral(":planned_date"), plannedDate);
    query.bindValue(QStringLiteral(":id"), taskId);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected task no longer exists."));
        return false;
    }
    return true;
}

std::optional<DailyNote> TaskRepository::dailyNote(
    const QDate &date,
    QString *errorMessage) const
{
    if (!date.isValid()) {
        assignError(errorMessage, QStringLiteral("A day note needs a valid date."));
        return std::nullopt;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT note_date, body, updated_at FROM daily_notes WHERE note_date = :note_date"));
    query.bindValue(QStringLiteral(":note_date"), serializeDate(date));
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return std::nullopt;
    }
    if (!query.next()) {
        return std::nullopt;
    }

    DailyNote note;
    note.date = deserializeDate(query.value(0).toString());
    note.text = query.value(1).toString();
    note.updatedAt = deserializeDateTime(query.value(2).toString());
    return note;
}

bool TaskRepository::saveDailyNote(
    const DailyNote &note,
    QString *errorMessage)
{
    if (!note.date.isValid()) {
        assignError(errorMessage, QStringLiteral("A day note needs a valid date."));
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO daily_notes (note_date, body, updated_at) "
        "VALUES (:note_date, :body, :updated_at) "
        "ON CONFLICT(note_date) DO UPDATE SET "
        "body = excluded.body, updated_at = excluded.updated_at"));
    query.bindValue(QStringLiteral(":note_date"), serializeDate(note.date));
    query.bindValue(QStringLiteral(":body"), note.text);
    query.bindValue(
        QStringLiteral(":updated_at"),
        serializeDateTime(note.updatedAt.isValid()
            ? note.updatedAt : QDateTime::currentDateTime()));
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::deleteDailyNote(const QDate &date, QString *errorMessage)
{
    if (!date.isValid()) {
        assignError(errorMessage, QStringLiteral("A valid day is required."));
        return false;
    }
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM daily_notes WHERE note_date = :note_date"));
    query.bindValue(QStringLiteral(":note_date"), serializeDate(date));
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::pruneDailyNotes(
    const QDate &oldestDate,
    QString *errorMessage)
{
    if (!oldestDate.isValid()) {
        assignError(errorMessage, QStringLiteral("A retention date is required."));
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "DELETE FROM daily_notes WHERE note_date < :oldest_date"));
    query.bindValue(QStringLiteral(":oldest_date"), serializeDate(oldestDate));
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

QVector<Category> TaskRepository::categories(QString *errorMessage) const
{
    QVector<Category> result;
    QHash<QString, qsizetype> categoryIndexes;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT c.id, c.name, c.notes, c.created_at, COUNT(t.id) "
            "FROM categories c LEFT JOIN tasks t "
            "ON t.category_id = c.id AND t.is_completed = 0 "
            "GROUP BY c.id, c.name, c.notes, c.created_at "
            "ORDER BY c.name COLLATE NOCASE ASC"))) {
        assignError(errorMessage, query.lastError().text());
        return result;
    }

    while (query.next()) {
        Category category;
        category.id = query.value(0).toString();
        category.name = query.value(1).toString();
        category.notes = query.value(2).toString();
        category.createdAt = deserializeDateTime(query.value(3).toString());
        category.taskCount = query.value(4).toInt();
        categoryIndexes.insert(category.id, result.size());
        result.append(category);
    }

    QSqlQuery subcategoryQuery(m_database);
    if (!subcategoryQuery.exec(QStringLiteral(
            "SELECT s.id, s.category_id, s.name, s.notes, s.created_at, COUNT(t.id) "
            "FROM subcategories s LEFT JOIN tasks t "
            "ON t.subcategory_id = s.id AND t.is_completed = 0 "
            "GROUP BY s.id, s.category_id, s.name, s.notes, s.created_at "
            "ORDER BY s.name COLLATE NOCASE ASC"))) {
        assignError(errorMessage, subcategoryQuery.lastError().text());
        return {};
    }
    while (subcategoryQuery.next()) {
        const QString categoryId = subcategoryQuery.value(1).toString();
        const auto categoryIndex = categoryIndexes.constFind(categoryId);
        if (categoryIndex == categoryIndexes.cend()) {
            continue;
        }
        Subcategory subcategory;
        subcategory.id = subcategoryQuery.value(0).toString();
        subcategory.categoryId = categoryId;
        subcategory.name = subcategoryQuery.value(2).toString();
        subcategory.notes = subcategoryQuery.value(3).toString();
        subcategory.createdAt = deserializeDateTime(subcategoryQuery.value(4).toString());
        subcategory.taskCount = subcategoryQuery.value(5).toInt();
        result[*categoryIndex].subcategories.append(subcategory);
    }
    return result;
}

bool TaskRepository::addCategory(
    const Category &category,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO categories (id, name, notes, created_at) "
        "VALUES (:id, :name, :notes, :created_at)"));
    bindCategory(query, category);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::updateCategory(
    const Category &category,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE categories SET name = :name, notes = :notes WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), category.id);
    query.bindValue(QStringLiteral(":name"), category.name);
    query.bindValue(
        QStringLiteral(":notes"),
        category.notes.isNull() ? QStringLiteral("") : category.notes);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected category no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::deleteCategory(const QString &categoryId, QString *errorMessage)
{
    return deleteById(
        m_database,
        QStringLiteral("categories"),
        categoryId,
        QStringLiteral("The selected category no longer exists."),
        errorMessage);
}

bool TaskRepository::addSubcategory(
    const Subcategory &subcategory,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO subcategories (id, category_id, name, notes, created_at) "
        "VALUES (:id, :category_id, :name, :notes, :created_at)"));
    bindSubcategory(query, subcategory);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::updateSubcategory(
    const Subcategory &subcategory,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE subcategories SET name = :name, notes = :notes "
        "WHERE id = :id AND category_id = :category_id"));
    query.bindValue(QStringLiteral(":id"), subcategory.id);
    query.bindValue(QStringLiteral(":category_id"), subcategory.categoryId);
    query.bindValue(QStringLiteral(":name"), subcategory.name);
    query.bindValue(
        QStringLiteral(":notes"),
        subcategory.notes.isNull() ? QStringLiteral("") : subcategory.notes);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected subcategory no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::deleteSubcategory(
    const QString &subcategoryId,
    QString *errorMessage)
{
    return deleteById(
        m_database,
        QStringLiteral("subcategories"),
        subcategoryId,
        QStringLiteral("The selected subcategory no longer exists."),
        errorMessage);
}

QVector<Goal> TaskRepository::goals(QString *errorMessage) const
{
    QVector<Goal> result;
    QHash<QString, qsizetype> goalIndexes;
    QSqlQuery goalQuery(m_database);

    if (!goalQuery.exec(QStringLiteral(
            "SELECT id, title, description, target_date, created_at, is_completed "
            "FROM goals "
            "ORDER BY (target_date IS NULL) ASC, target_date ASC, created_at ASC"))) {
        assignError(errorMessage, goalQuery.lastError().text());
        return result;
    }

    while (goalQuery.next()) {
        Goal goal;
        goal.id = goalQuery.value(0).toString();
        goal.title = goalQuery.value(1).toString();
        goal.description = goalQuery.value(2).toString();
        goal.targetDate = deserializeDate(goalQuery.value(3).toString());
        goal.createdAt = deserializeDateTime(goalQuery.value(4).toString());
        goal.completed = goalQuery.value(5).toBool();
        goalIndexes.insert(goal.id, result.size());
        result.append(goal);
    }

    QSqlQuery milestoneQuery(m_database);
    if (!milestoneQuery.exec(QStringLiteral(
            "SELECT id, goal_id, title, target_date, created_at, is_completed "
            "FROM milestones ORDER BY position ASC, created_at ASC"))) {
        assignError(errorMessage, milestoneQuery.lastError().text());
        return {};
    }

    while (milestoneQuery.next()) {
        const QString goalId = milestoneQuery.value(1).toString();
        const auto index = goalIndexes.constFind(goalId);
        if (index == goalIndexes.cend()) {
            continue;
        }

        Milestone milestone;
        milestone.id = milestoneQuery.value(0).toString();
        milestone.goalId = goalId;
        milestone.title = milestoneQuery.value(2).toString();
        milestone.targetDate = deserializeDate(milestoneQuery.value(3).toString());
        milestone.createdAt = deserializeDateTime(milestoneQuery.value(4).toString());
        milestone.completed = milestoneQuery.value(5).toBool();
        result[*index].milestones.append(milestone);
    }

    return result;
}

bool TaskRepository::addGoal(const Goal &goal, QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO goals "
        "(id, title, description, target_date, created_at, is_completed, completed_at) "
        "VALUES (:id, :title, :description, :target_date, :created_at, :is_completed, "
        ":completed_at)"));
    bindGoal(query, goal);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::deleteGoal(const QString &goalId, QString *errorMessage)
{
    return deleteById(
        m_database,
        QStringLiteral("goals"),
        goalId,
        QStringLiteral("The selected goal no longer exists."),
        errorMessage);
}

bool TaskRepository::addMilestone(
    const Milestone &milestone,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO milestones "
        "(id, goal_id, title, target_date, created_at, is_completed, completed_at, position) "
        "VALUES (:id, :goal_id, :title, :target_date, :created_at, :is_completed, "
        ":completed_at, (SELECT COALESCE(MAX(position), -1) + 1 FROM milestones "
        "WHERE goal_id = :goal_id))"));
    bindMilestone(query, milestone);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::deleteMilestone(
    const QString &milestoneId,
    QString *errorMessage)
{
    return deleteById(
        m_database,
        QStringLiteral("milestones"),
        milestoneId,
        QStringLiteral("The selected milestone no longer exists."),
        errorMessage);
}

bool TaskRepository::setMilestoneCompleted(
    const QString &milestoneId,
    bool completed,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE milestones SET is_completed = :completed, completed_at = :completed_at "
        "WHERE id = :id"));
    query.bindValue(QStringLiteral(":completed"), completed);
    bindNullableDateTime(
        query,
        QStringLiteral(":completed_at"),
        completed ? QDateTime::currentDateTime() : QDateTime());
    query.bindValue(QStringLiteral(":id"), milestoneId);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected milestone no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::setGoalCompleted(
    const QString &goalId,
    bool completed,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE goals SET is_completed = :completed, completed_at = :completed_at "
        "WHERE id = :id"));
    query.bindValue(QStringLiteral(":completed"), completed);
    bindNullableDateTime(
        query,
        QStringLiteral(":completed_at"),
        completed ? QDateTime::currentDateTime() : QDateTime());
    query.bindValue(QStringLiteral(":id"), goalId);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected goal no longer exists."));
        return false;
    }
    return true;
}

QVector<Meeting> TaskRepository::meetings(QString *errorMessage) const
{
    QVector<Meeting> result;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT id, title, notes, starts_at, created_at, notified_at "
            "FROM meetings ORDER BY starts_at ASC, created_at ASC"))) {
        assignError(errorMessage, query.lastError().text());
        return result;
    }

    while (query.next()) {
        Meeting meeting;
        meeting.id = query.value(0).toString();
        meeting.title = query.value(1).toString();
        meeting.notes = query.value(2).toString();
        meeting.startsAt = deserializeDateTime(query.value(3).toString());
        meeting.createdAt = deserializeDateTime(query.value(4).toString());
        meeting.notifiedAt = deserializeDateTime(query.value(5).toString());
        result.append(meeting);
    }
    return result;
}

bool TaskRepository::addMeeting(
    const Meeting &meeting,
    QString *errorMessage)
{
    if (meeting.title.trimmed().isEmpty() || !meeting.startsAt.isValid()
        || !meeting.createdAt.isValid()) {
        assignError(errorMessage, QStringLiteral("A meeting needs a title and valid start time."));
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO meetings (id, title, notes, starts_at, created_at, notified_at) "
        "VALUES (:id, :title, :notes, :starts_at, :created_at, :notified_at)"));
    bindMeeting(query, meeting);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::deleteMeeting(
    const QString &meetingId,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM meetings WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), meetingId);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected meeting no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::markMeetingNotified(
    const QString &meetingId,
    const QDateTime &notifiedAt,
    QString *errorMessage)
{
    if (!notifiedAt.isValid()) {
        assignError(errorMessage, QStringLiteral("A valid notification time is required."));
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE meetings SET notified_at = :notified_at "
        "WHERE id = :id AND notified_at IS NULL"));
    query.bindValue(QStringLiteral(":notified_at"), serializeDateTime(notifiedAt));
    query.bindValue(QStringLiteral(":id"), meetingId);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The meeting was already notified or removed."));
        return false;
    }
    return true;
}

QVector<MentalMapGroup> TaskRepository::mentalMapGroups(QString *errorMessage) const
{
    QVector<MentalMapGroup> result;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT id, kind, title, color, x, y, width, height, created_at "
            "FROM mental_map_groups ORDER BY created_at ASC"))) {
        assignError(errorMessage, query.lastError().text());
        return result;
    }
    while (query.next()) {
        MentalMapGroup group;
        group.id = query.value(0).toString();
        group.kind = query.value(1).toString();
        group.title = query.value(2).toString();
        group.color = query.value(3).toString();
        group.x = query.value(4).toDouble();
        group.y = query.value(5).toDouble();
        group.width = query.value(6).toDouble();
        group.height = query.value(7).toDouble();
        group.createdAt = deserializeDateTime(query.value(8).toString());
        result.append(group);
    }
    return result;
}

QVector<MentalMapNote> TaskRepository::mentalMapNotes(QString *errorMessage) const
{
    QVector<MentalMapNote> result;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT n.id, n.group_id, g.kind, n.title, n.body, n.color, n.tags, "
            "n.external_link, n.priority, n.x, n.y, n.is_center, n.linked_task_id, "
            "COALESCE(t.title, ''), COALESCE(t.is_completed, 0), n.checklist_json, "
            "n.created_at FROM mental_map_notes n "
            "JOIN mental_map_groups g ON g.id = n.group_id "
            "LEFT JOIN tasks t ON t.id = n.linked_task_id "
            "ORDER BY n.created_at ASC"))) {
        assignError(errorMessage, query.lastError().text());
        return result;
    }
    while (query.next()) {
        MentalMapNote note;
        note.id = query.value(0).toString();
        note.groupId = query.value(1).toString();
        note.groupKind = query.value(2).toString();
        note.title = query.value(3).toString();
        note.body = query.value(4).toString();
        note.color = query.value(5).toString();
        note.tags = query.value(6).toString();
        note.externalLink = query.value(7).toString();
        note.priority = query.value(8).toInt();
        note.x = query.value(9).toDouble();
        note.y = query.value(10).toDouble();
        note.center = query.value(11).toBool();
        note.linkedTaskId = query.value(12).toString();
        note.linkedTaskTitle = query.value(13).toString();
        note.linkedTaskCompleted = query.value(14).toBool();
        note.checklist = deserializeChecklist(query.value(15).toString());
        note.createdAt = deserializeDateTime(query.value(16).toString());
        result.append(note);
    }
    return result;
}

QVector<MentalMapConnection> TaskRepository::mentalMapConnections(
    QString *errorMessage) const
{
    QVector<MentalMapConnection> result;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT id, view_kind, source_note_id, target_note_id, label, created_at "
            "FROM mental_map_connections ORDER BY created_at ASC"))) {
        assignError(errorMessage, query.lastError().text());
        return result;
    }
    while (query.next()) {
        result.append({
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toString(),
            deserializeDateTime(query.value(5).toString()),
        });
    }
    return result;
}

bool TaskRepository::addMentalMapGroup(
    const MentalMapGroup &group,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO mental_map_groups "
        "(id, kind, title, color, x, y, width, height, created_at) "
        "VALUES (:id, :kind, :title, :color, :x, :y, :width, :height, :created_at)"));
    query.bindValue(QStringLiteral(":id"), group.id);
    query.bindValue(QStringLiteral(":kind"), group.kind);
    query.bindValue(QStringLiteral(":title"), group.title);
    query.bindValue(QStringLiteral(":color"), group.color);
    query.bindValue(QStringLiteral(":x"), group.x);
    query.bindValue(QStringLiteral(":y"), group.y);
    query.bindValue(QStringLiteral(":width"), group.width);
    query.bindValue(QStringLiteral(":height"), group.height);
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(group.createdAt));
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::updateMentalMapGroup(
    const MentalMapGroup &group,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE mental_map_groups SET title = :title, color = :color, x = :x, y = :y, "
        "width = :width, height = :height WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), group.id);
    query.bindValue(QStringLiteral(":title"), group.title);
    query.bindValue(QStringLiteral(":color"), group.color);
    query.bindValue(QStringLiteral(":x"), group.x);
    query.bindValue(QStringLiteral(":y"), group.y);
    query.bindValue(QStringLiteral(":width"), group.width);
    query.bindValue(QStringLiteral(":height"), group.height);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected map group no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::deleteMentalMapGroup(
    const QString &groupId,
    QString *errorMessage)
{
    return deleteById(
        m_database,
        QStringLiteral("mental_map_groups"),
        groupId,
        QStringLiteral("The selected map group no longer exists."),
        errorMessage);
}

bool TaskRepository::addMentalMapNote(
    const MentalMapNote &note,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO mental_map_notes "
        "(id, group_id, title, body, color, tags, external_link, priority, x, y, "
        "is_center, linked_task_id, checklist_json, created_at) VALUES "
        "(:id, :group_id, :title, :body, :color, :tags, :external_link, :priority, "
        ":x, :y, :is_center, :linked_task_id, :checklist_json, :created_at)"));
    query.bindValue(QStringLiteral(":id"), note.id);
    query.bindValue(QStringLiteral(":group_id"), note.groupId);
    query.bindValue(QStringLiteral(":title"), note.title);
    query.bindValue(
        QStringLiteral(":body"), note.body.isNull() ? QStringLiteral("") : note.body);
    query.bindValue(QStringLiteral(":color"), note.color);
    query.bindValue(
        QStringLiteral(":tags"), note.tags.isNull() ? QStringLiteral("") : note.tags);
    query.bindValue(
        QStringLiteral(":external_link"),
        note.externalLink.isNull() ? QStringLiteral("") : note.externalLink);
    query.bindValue(QStringLiteral(":priority"), note.priority);
    query.bindValue(QStringLiteral(":x"), note.x);
    query.bindValue(QStringLiteral(":y"), note.y);
    query.bindValue(QStringLiteral(":is_center"), note.center);
    bindNullableString(query, QStringLiteral(":linked_task_id"), note.linkedTaskId);
    query.bindValue(QStringLiteral(":checklist_json"), serializeChecklist(note.checklist));
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(note.createdAt));
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::updateMentalMapNote(
    const MentalMapNote &note,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE mental_map_notes SET title = :title, body = :body, color = :color, "
        "tags = :tags, external_link = :external_link, priority = :priority, x = :x, "
        "y = :y, checklist_json = :checklist_json WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), note.id);
    query.bindValue(QStringLiteral(":title"), note.title);
    query.bindValue(
        QStringLiteral(":body"), note.body.isNull() ? QStringLiteral("") : note.body);
    query.bindValue(QStringLiteral(":color"), note.color);
    query.bindValue(
        QStringLiteral(":tags"), note.tags.isNull() ? QStringLiteral("") : note.tags);
    query.bindValue(
        QStringLiteral(":external_link"),
        note.externalLink.isNull() ? QStringLiteral("") : note.externalLink);
    query.bindValue(QStringLiteral(":priority"), note.priority);
    query.bindValue(QStringLiteral(":x"), note.x);
    query.bindValue(QStringLiteral(":y"), note.y);
    query.bindValue(QStringLiteral(":checklist_json"), serializeChecklist(note.checklist));
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected map note no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::deleteMentalMapNote(
    const QString &noteId,
    QString *errorMessage)
{
    return deleteById(
        m_database,
        QStringLiteral("mental_map_notes"),
        noteId,
        QStringLiteral("The selected map note no longer exists."),
        errorMessage);
}

bool TaskRepository::addMentalMapConnection(
    const MentalMapConnection &connection,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO mental_map_connections "
        "(id, view_kind, source_note_id, target_note_id, label, created_at) "
        "VALUES (:id, :view_kind, :source_note_id, :target_note_id, :label, :created_at)"));
    query.bindValue(QStringLiteral(":id"), connection.id);
    query.bindValue(QStringLiteral(":view_kind"), connection.viewKind);
    query.bindValue(QStringLiteral(":source_note_id"), connection.sourceNoteId);
    query.bindValue(QStringLiteral(":target_note_id"), connection.targetNoteId);
    query.bindValue(
        QStringLiteral(":label"),
        connection.label.isNull() ? QStringLiteral("") : connection.label);
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(connection.createdAt));
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::deleteMentalMapConnection(
    const QString &connectionId,
    QString *errorMessage)
{
    return deleteById(
        m_database,
        QStringLiteral("mental_map_connections"),
        connectionId,
        QStringLiteral("The selected connection no longer exists."),
        errorMessage);
}

bool TaskRepository::linkMentalMapNoteToTask(
    const QString &noteId,
    const QString &taskId,
    QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE mental_map_notes SET linked_task_id = :task_id WHERE id = :id"));
    bindNullableString(query, QStringLiteral(":task_id"), taskId);
    query.bindValue(QStringLiteral(":id"), noteId);
    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        assignError(errorMessage, QStringLiteral("The selected map note no longer exists."));
        return false;
    }
    return true;
}

bool TaskRepository::mergeImportedData(
    const QVector<Category> &categoriesToMerge,
    const QVector<Task> &tasks,
    const QVector<Goal> &goalsToMerge,
    const QVector<Meeting> &meetingsToMerge,
    const QVector<MentalMapGroup> &mapGroups,
    const QVector<MentalMapNote> &mapNotes,
    const QVector<MentalMapConnection> &mapConnections,
    QString *errorMessage)
{
    if (!m_database.transaction()) {
        assignError(errorMessage, m_database.lastError().text());
        return false;
    }

    QSqlQuery categoryQuery(m_database);
    categoryQuery.prepare(QStringLiteral(
        "INSERT INTO categories (id, name, notes, created_at) "
        "VALUES (:id, :name, :notes, :created_at) "
        "ON CONFLICT(id) DO UPDATE SET name = excluded.name, notes = excluded.notes, "
        "created_at = excluded.created_at"));
    QSqlQuery subcategoryQuery(m_database);
    subcategoryQuery.prepare(QStringLiteral(
        "INSERT INTO subcategories (id, category_id, name, notes, created_at) "
        "VALUES (:id, :category_id, :name, :notes, :created_at) "
        "ON CONFLICT(id) DO UPDATE SET category_id = excluded.category_id, "
        "name = excluded.name, notes = excluded.notes, created_at = excluded.created_at"));
    for (const Category &category : categoriesToMerge) {
        bindCategory(categoryQuery, category);
        if (!categoryQuery.exec()) {
            assignError(errorMessage, categoryQuery.lastError().text());
            m_database.rollback();
            return false;
        }
        for (const Subcategory &subcategory : category.subcategories) {
            bindSubcategory(subcategoryQuery, subcategory);
            if (!subcategoryQuery.exec()) {
                assignError(errorMessage, subcategoryQuery.lastError().text());
                m_database.rollback();
                return false;
            }
        }
    }

    QSqlQuery taskQuery(m_database);
    taskQuery.prepare(QStringLiteral(
        "INSERT INTO tasks "
        "(id, title, notes, project, due_at, created_at, importance, estimated_minutes, "
        "is_completed, completed_at, category_id, subcategory_id, planned_date) "
        "VALUES (:id, :title, :notes, :project, :due_at, :created_at, :importance, "
        ":estimated_minutes, :is_completed, :completed_at, :category_id, "
        ":subcategory_id, :planned_date) "
        "ON CONFLICT(id) DO UPDATE SET title = excluded.title, notes = excluded.notes, "
        "project = excluded.project, due_at = excluded.due_at, "
        "created_at = excluded.created_at, importance = excluded.importance, "
        "estimated_minutes = excluded.estimated_minutes, "
        "is_completed = excluded.is_completed, completed_at = excluded.completed_at, "
        "category_id = excluded.category_id, subcategory_id = excluded.subcategory_id, "
        "planned_date = excluded.planned_date"));

    for (const Task &task : tasks) {
        bindTask(taskQuery, task);
        if (!taskQuery.exec()) {
            assignError(errorMessage, taskQuery.lastError().text());
            m_database.rollback();
            return false;
        }
    }

    QSqlQuery goalQuery(m_database);
    goalQuery.prepare(QStringLiteral(
        "INSERT INTO goals "
        "(id, title, description, target_date, created_at, is_completed, completed_at) "
        "VALUES (:id, :title, :description, :target_date, :created_at, :is_completed, "
        ":completed_at) "
        "ON CONFLICT(id) DO UPDATE SET title = excluded.title, "
        "description = excluded.description, target_date = excluded.target_date, "
        "created_at = excluded.created_at, is_completed = excluded.is_completed, "
        "completed_at = excluded.completed_at"));

    QSqlQuery milestoneQuery(m_database);
    milestoneQuery.prepare(QStringLiteral(
        "INSERT INTO milestones "
        "(id, goal_id, title, target_date, created_at, is_completed, completed_at, position) "
        "VALUES (:id, :goal_id, :title, :target_date, :created_at, :is_completed, "
        ":completed_at, :position) "
        "ON CONFLICT(id) DO UPDATE SET goal_id = excluded.goal_id, title = excluded.title, "
        "target_date = excluded.target_date, created_at = excluded.created_at, "
        "is_completed = excluded.is_completed, completed_at = excluded.completed_at, "
        "position = excluded.position"));

    for (const Goal &goal : goalsToMerge) {
        bindGoal(goalQuery, goal);
        if (!goalQuery.exec()) {
            assignError(errorMessage, goalQuery.lastError().text());
            m_database.rollback();
            return false;
        }

        for (qsizetype index = 0; index < goal.milestones.size(); ++index) {
            Milestone milestone = goal.milestones.at(index);
            milestone.goalId = goal.id;
            bindMilestone(milestoneQuery, milestone);
            milestoneQuery.bindValue(QStringLiteral(":position"), index);
            if (!milestoneQuery.exec()) {
                assignError(errorMessage, milestoneQuery.lastError().text());
                m_database.rollback();
                return false;
            }
        }
    }

    QSqlQuery meetingQuery(m_database);
    meetingQuery.prepare(QStringLiteral(
        "INSERT INTO meetings (id, title, notes, starts_at, created_at, notified_at) "
        "VALUES (:id, :title, :notes, :starts_at, :created_at, :notified_at) "
        "ON CONFLICT(id) DO UPDATE SET title = excluded.title, notes = excluded.notes, "
        "starts_at = excluded.starts_at, created_at = excluded.created_at, "
        "notified_at = COALESCE(meetings.notified_at, excluded.notified_at)"));
    for (const Meeting &meeting : meetingsToMerge) {
        bindMeeting(meetingQuery, meeting);
        if (!meetingQuery.exec()) {
            assignError(errorMessage, meetingQuery.lastError().text());
            m_database.rollback();
            return false;
        }
    }

    QSqlQuery mapGroupQuery(m_database);
    mapGroupQuery.prepare(QStringLiteral(
        "INSERT INTO mental_map_groups "
        "(id, kind, title, color, x, y, width, height, created_at) "
        "VALUES (:id, :kind, :title, :color, :x, :y, :width, :height, :created_at) "
        "ON CONFLICT(id) DO UPDATE SET kind = excluded.kind, title = excluded.title, "
        "color = excluded.color, x = excluded.x, y = excluded.y, width = excluded.width, "
        "height = excluded.height, created_at = excluded.created_at"));
    for (const MentalMapGroup &group : mapGroups) {
        mapGroupQuery.bindValue(QStringLiteral(":id"), group.id);
        mapGroupQuery.bindValue(QStringLiteral(":kind"), group.kind);
        mapGroupQuery.bindValue(QStringLiteral(":title"), group.title);
        mapGroupQuery.bindValue(QStringLiteral(":color"), group.color);
        mapGroupQuery.bindValue(QStringLiteral(":x"), group.x);
        mapGroupQuery.bindValue(QStringLiteral(":y"), group.y);
        mapGroupQuery.bindValue(QStringLiteral(":width"), group.width);
        mapGroupQuery.bindValue(QStringLiteral(":height"), group.height);
        mapGroupQuery.bindValue(QStringLiteral(":created_at"), serializeDateTime(group.createdAt));
        if (!mapGroupQuery.exec()) {
            assignError(errorMessage, mapGroupQuery.lastError().text());
            m_database.rollback();
            return false;
        }
    }

    QSqlQuery mapNoteQuery(m_database);
    mapNoteQuery.prepare(QStringLiteral(
        "INSERT INTO mental_map_notes "
        "(id, group_id, title, body, color, tags, external_link, priority, x, y, "
        "is_center, linked_task_id, checklist_json, created_at) VALUES "
        "(:id, :group_id, :title, :body, :color, :tags, :external_link, :priority, "
        ":x, :y, :is_center, :linked_task_id, :checklist_json, :created_at) "
        "ON CONFLICT(id) DO UPDATE SET group_id = excluded.group_id, title = excluded.title, "
        "body = excluded.body, color = excluded.color, tags = excluded.tags, "
        "external_link = excluded.external_link, priority = excluded.priority, x = excluded.x, "
        "y = excluded.y, is_center = excluded.is_center, "
        "linked_task_id = excluded.linked_task_id, checklist_json = excluded.checklist_json, "
        "created_at = excluded.created_at"));
    for (const MentalMapNote &note : mapNotes) {
        mapNoteQuery.bindValue(QStringLiteral(":id"), note.id);
        mapNoteQuery.bindValue(QStringLiteral(":group_id"), note.groupId);
        mapNoteQuery.bindValue(QStringLiteral(":title"), note.title);
        mapNoteQuery.bindValue(
            QStringLiteral(":body"), note.body.isNull() ? QStringLiteral("") : note.body);
        mapNoteQuery.bindValue(QStringLiteral(":color"), note.color);
        mapNoteQuery.bindValue(
            QStringLiteral(":tags"), note.tags.isNull() ? QStringLiteral("") : note.tags);
        mapNoteQuery.bindValue(
            QStringLiteral(":external_link"),
            note.externalLink.isNull() ? QStringLiteral("") : note.externalLink);
        mapNoteQuery.bindValue(QStringLiteral(":priority"), note.priority);
        mapNoteQuery.bindValue(QStringLiteral(":x"), note.x);
        mapNoteQuery.bindValue(QStringLiteral(":y"), note.y);
        mapNoteQuery.bindValue(QStringLiteral(":is_center"), note.center);
        bindNullableString(
            mapNoteQuery, QStringLiteral(":linked_task_id"), note.linkedTaskId);
        mapNoteQuery.bindValue(
            QStringLiteral(":checklist_json"), serializeChecklist(note.checklist));
        mapNoteQuery.bindValue(QStringLiteral(":created_at"), serializeDateTime(note.createdAt));
        if (!mapNoteQuery.exec()) {
            assignError(errorMessage, mapNoteQuery.lastError().text());
            m_database.rollback();
            return false;
        }
    }

    QSqlQuery mapConnectionQuery(m_database);
    mapConnectionQuery.prepare(QStringLiteral(
        "INSERT INTO mental_map_connections "
        "(id, view_kind, source_note_id, target_note_id, label, created_at) "
        "VALUES (:id, :view_kind, :source_note_id, :target_note_id, :label, :created_at) "
        "ON CONFLICT(id) DO UPDATE SET view_kind = excluded.view_kind, "
        "source_note_id = excluded.source_note_id, target_note_id = excluded.target_note_id, "
        "label = excluded.label, created_at = excluded.created_at"));
    for (const MentalMapConnection &connection : mapConnections) {
        mapConnectionQuery.bindValue(QStringLiteral(":id"), connection.id);
        mapConnectionQuery.bindValue(QStringLiteral(":view_kind"), connection.viewKind);
        mapConnectionQuery.bindValue(
            QStringLiteral(":source_note_id"), connection.sourceNoteId);
        mapConnectionQuery.bindValue(
            QStringLiteral(":target_note_id"), connection.targetNoteId);
        mapConnectionQuery.bindValue(
            QStringLiteral(":label"),
            connection.label.isNull() ? QStringLiteral("") : connection.label);
        mapConnectionQuery.bindValue(
            QStringLiteral(":created_at"), serializeDateTime(connection.createdAt));
        if (!mapConnectionQuery.exec()) {
            assignError(errorMessage, mapConnectionQuery.lastError().text());
            m_database.rollback();
            return false;
        }
    }

    if (!m_database.commit()) {
        assignError(errorMessage, m_database.lastError().text());
        m_database.rollback();
        return false;
    }
    return true;
}

bool TaskRepository::migrate(QString *errorMessage)
{
    QSqlQuery versionQuery(m_database);
    if (!versionQuery.exec(QStringLiteral("PRAGMA user_version")) || !versionQuery.next()) {
        assignError(errorMessage, versionQuery.lastError().text());
        return false;
    }

    const int version = versionQuery.value(0).toInt();
    if (version > 8) {
        assignError(
            errorMessage,
            QStringLiteral("This database was created by a newer Daymark version."));
        return false;
    }
    if (version == 8) {
        return true;
    }

    if (!m_database.transaction()) {
        assignError(errorMessage, m_database.lastError().text());
        return false;
    }

    bool migrated = true;
    if (version == 0) {
        migrated = execute(QStringLiteral(
            "CREATE TABLE tasks ("
            "id TEXT PRIMARY KEY NOT NULL, "
            "title TEXT NOT NULL CHECK(length(trim(title)) > 0), "
            "notes TEXT NOT NULL DEFAULT '', "
            "project TEXT NOT NULL DEFAULT '', "
            "due_at TEXT, "
            "created_at TEXT NOT NULL, "
            "importance INTEGER NOT NULL DEFAULT 3 CHECK(importance BETWEEN 1 AND 5), "
            "estimated_minutes INTEGER NOT NULL DEFAULT 30 "
            "CHECK(estimated_minutes BETWEEN 5 AND 480), "
            "is_completed INTEGER NOT NULL DEFAULT 0 CHECK(is_completed IN (0, 1)), "
            "completed_at TEXT"
            ")"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX tasks_open_due_index ON tasks(is_completed, due_at)"), errorMessage);
    }

    if (migrated && version < 2) {
        migrated = execute(QStringLiteral(
            "CREATE TABLE goals ("
            "id TEXT PRIMARY KEY NOT NULL, "
            "title TEXT NOT NULL CHECK(length(trim(title)) > 0), "
            "description TEXT NOT NULL DEFAULT '', "
            "target_date TEXT, "
            "created_at TEXT NOT NULL, "
            "is_completed INTEGER NOT NULL DEFAULT 0 CHECK(is_completed IN (0, 1)), "
            "completed_at TEXT"
            ")"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX goals_open_target_index ON goals(is_completed, target_date)"), errorMessage)
            && execute(QStringLiteral(
                "CREATE TABLE milestones ("
                "id TEXT PRIMARY KEY NOT NULL, "
                "goal_id TEXT NOT NULL REFERENCES goals(id) ON DELETE CASCADE, "
                "title TEXT NOT NULL CHECK(length(trim(title)) > 0), "
                "target_date TEXT, "
                "created_at TEXT NOT NULL, "
                "is_completed INTEGER NOT NULL DEFAULT 0 CHECK(is_completed IN (0, 1)), "
                "completed_at TEXT, "
                "position INTEGER NOT NULL DEFAULT 0"
                ")"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX milestones_goal_position_index "
                "ON milestones(goal_id, position)"), errorMessage)
            && execute(QStringLiteral("PRAGMA user_version = 2"), errorMessage);
    }

    if (migrated && version < 3) {
        migrated = execute(QStringLiteral(
            "CREATE TABLE categories ("
            "id TEXT PRIMARY KEY NOT NULL, "
            "name TEXT NOT NULL COLLATE NOCASE UNIQUE "
            "CHECK(length(trim(name)) > 0), "
            "notes TEXT NOT NULL DEFAULT '', "
            "created_at TEXT NOT NULL"
            ")"), errorMessage)
            && execute(QStringLiteral(
                "ALTER TABLE tasks ADD COLUMN category_id TEXT "
                "REFERENCES categories(id) ON DELETE SET NULL"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX tasks_category_index ON tasks(category_id)"), errorMessage)
            && execute(QStringLiteral("PRAGMA user_version = 3"), errorMessage);
    }

    if (migrated && version < 4) {
        migrated = execute(QStringLiteral(
            "CREATE TABLE subcategories ("
            "id TEXT PRIMARY KEY NOT NULL, "
            "category_id TEXT NOT NULL REFERENCES categories(id) ON DELETE CASCADE, "
            "name TEXT NOT NULL COLLATE NOCASE CHECK(length(trim(name)) > 0), "
            "notes TEXT NOT NULL DEFAULT '', "
            "created_at TEXT NOT NULL, "
            "UNIQUE(category_id, name)"
            ")"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX subcategories_category_name_index "
                "ON subcategories(category_id, name)"), errorMessage)
            && execute(QStringLiteral(
                "ALTER TABLE tasks ADD COLUMN subcategory_id TEXT "
                "REFERENCES subcategories(id) ON DELETE SET NULL"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX tasks_subcategory_index ON tasks(subcategory_id)"), errorMessage)
            && execute(QStringLiteral(
                "INSERT INTO categories (id, name, notes, created_at) "
                "SELECT 'migrated-project-' || lower(hex(randomblob(16))), "
                "legacy.project_name, 'Migrated from the former Projects field.', "
                "legacy.created_at FROM ("
                "SELECT trim(project) AS project_name, MIN(created_at) AS created_at "
                "FROM tasks WHERE length(trim(project)) > 0 "
                "GROUP BY trim(project) COLLATE NOCASE"
                ") AS legacy WHERE NOT EXISTS ("
                "SELECT 1 FROM categories c "
                "WHERE c.name = legacy.project_name COLLATE NOCASE)"), errorMessage)
            && execute(QStringLiteral(
                "UPDATE tasks SET category_id = ("
                "SELECT c.id FROM categories c "
                "WHERE c.name = trim(tasks.project) COLLATE NOCASE LIMIT 1) "
                "WHERE category_id IS NULL AND length(trim(project)) > 0"), errorMessage)
            && execute(QStringLiteral(
                "UPDATE tasks SET project = '' WHERE length(project) > 0"), errorMessage)
            && execute(QStringLiteral("PRAGMA user_version = 4"), errorMessage);
    }

    if (migrated && version < 5) {
        migrated = execute(QStringLiteral(
            "ALTER TABLE tasks ADD COLUMN planned_date TEXT"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX tasks_open_planned_index "
                "ON tasks(is_completed, planned_date)"), errorMessage)
            && execute(QStringLiteral("PRAGMA user_version = 5"), errorMessage);
    }

    if (migrated && version < 6) {
        migrated = execute(QStringLiteral(
            "CREATE TABLE daily_notes ("
            "note_date TEXT PRIMARY KEY NOT NULL, "
            "body TEXT NOT NULL DEFAULT '', "
            "updated_at TEXT NOT NULL"
            ")"), errorMessage)
            && execute(QStringLiteral("PRAGMA user_version = 6"), errorMessage);
    }

    if (migrated && version < 7) {
        migrated = execute(QStringLiteral(
            "CREATE TABLE meetings ("
            "id TEXT PRIMARY KEY NOT NULL, "
            "title TEXT NOT NULL CHECK(length(trim(title)) > 0), "
            "notes TEXT NOT NULL DEFAULT '', "
            "starts_at TEXT NOT NULL, "
            "created_at TEXT NOT NULL, "
            "notified_at TEXT"
            ")"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX meetings_start_index ON meetings(starts_at)"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX meetings_pending_reminder_index "
                "ON meetings(notified_at, starts_at)"), errorMessage)
            && execute(QStringLiteral("PRAGMA user_version = 7"), errorMessage);
    }

    if (migrated && version < 8) {
        migrated = execute(QStringLiteral(
            "CREATE TABLE mental_map_groups ("
            "id TEXT PRIMARY KEY NOT NULL, "
            "kind TEXT NOT NULL CHECK(kind IN ('cloud', 'constellation')), "
            "title TEXT NOT NULL CHECK(length(trim(title)) > 0), "
            "color TEXT NOT NULL DEFAULT 'accent', "
            "x REAL NOT NULL, y REAL NOT NULL, "
            "width REAL NOT NULL CHECK(width BETWEEN 220 AND 1200), "
            "height REAL NOT NULL CHECK(height BETWEEN 160 AND 900), "
            "created_at TEXT NOT NULL"
            ")"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX mental_map_groups_kind_index "
                "ON mental_map_groups(kind, created_at)"), errorMessage)
            && execute(QStringLiteral(
                "CREATE TABLE mental_map_notes ("
                "id TEXT PRIMARY KEY NOT NULL, "
                "group_id TEXT NOT NULL REFERENCES mental_map_groups(id) ON DELETE CASCADE, "
                "title TEXT NOT NULL CHECK(length(trim(title)) > 0), "
                "body TEXT NOT NULL DEFAULT '', "
                "color TEXT NOT NULL DEFAULT 'accent', "
                "tags TEXT NOT NULL DEFAULT '', "
                "external_link TEXT NOT NULL DEFAULT '', "
                "priority INTEGER NOT NULL DEFAULT 0 CHECK(priority BETWEEN 0 AND 5), "
                "x REAL NOT NULL, y REAL NOT NULL, "
                "is_center INTEGER NOT NULL DEFAULT 0 CHECK(is_center IN (0, 1)), "
                "linked_task_id TEXT REFERENCES tasks(id) ON DELETE SET NULL, "
                "checklist_json TEXT NOT NULL DEFAULT '[]', "
                "created_at TEXT NOT NULL"
                ")"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX mental_map_notes_group_index "
                "ON mental_map_notes(group_id, created_at)"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX mental_map_notes_task_index "
                "ON mental_map_notes(linked_task_id)"), errorMessage)
            && execute(QStringLiteral(
                "CREATE TABLE mental_map_connections ("
                "id TEXT PRIMARY KEY NOT NULL, "
                "view_kind TEXT NOT NULL CHECK(view_kind IN ('cloud', 'constellation')), "
                "source_note_id TEXT NOT NULL REFERENCES mental_map_notes(id) ON DELETE CASCADE, "
                "target_note_id TEXT NOT NULL REFERENCES mental_map_notes(id) ON DELETE CASCADE, "
                "label TEXT NOT NULL DEFAULT '', "
                "created_at TEXT NOT NULL, "
                "CHECK(source_note_id <> target_note_id), "
                "UNIQUE(source_note_id, target_note_id)"
                ")"), errorMessage)
            && execute(QStringLiteral(
                "CREATE INDEX mental_map_connections_view_index "
                "ON mental_map_connections(view_kind, created_at)"), errorMessage)
            && execute(QStringLiteral("PRAGMA user_version = 8"), errorMessage);
    }

    if (!migrated) {
        m_database.rollback();
        return false;
    }

    if (!m_database.commit()) {
        assignError(errorMessage, m_database.lastError().text());
        return false;
    }
    return true;
}

bool TaskRepository::execute(const QString &sql, QString *errorMessage) const
{
    QSqlQuery query(m_database);
    if (!query.exec(sql)) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }
    return true;
}
