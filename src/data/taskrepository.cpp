// SPDX-License-Identifier: GPL-3.0-or-later

#include "data/taskrepository.h"

#include <QDir>
#include <QFileInfo>
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

void assignError(QString *target, const QString &message)
{
    if (target != nullptr) {
        *target = message;
    }
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
    QVector<Task> tasks;
    QSqlQuery query(m_database);

    if (!query.exec(QStringLiteral(
            "SELECT id, title, notes, project, due_at, created_at, importance, "
            "estimated_minutes, is_completed "
            "FROM tasks WHERE is_completed = 0 ORDER BY created_at ASC"))) {
        assignError(errorMessage, query.lastError().text());
        return tasks;
    }

    while (query.next()) {
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
        tasks.append(task);
    }

    return tasks;
}

bool TaskRepository::addTask(const Task &task, QString *errorMessage)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO tasks "
        "(id, title, notes, project, due_at, created_at, importance, estimated_minutes, is_completed) "
        "VALUES (:id, :title, :notes, :project, :due_at, :created_at, :importance, "
        ":estimated_minutes, :is_completed)"));

    query.bindValue(QStringLiteral(":id"), task.id);
    query.bindValue(QStringLiteral(":title"), task.title);
    query.bindValue(
        QStringLiteral(":notes"),
        task.notes.isNull() ? QStringLiteral("") : task.notes);
    query.bindValue(
        QStringLiteral(":project"),
        task.project.isNull() ? QStringLiteral("") : task.project);
    if (task.dueAt.isValid()) {
        query.bindValue(QStringLiteral(":due_at"), serializeDateTime(task.dueAt));
    } else {
        query.bindValue(
            QStringLiteral(":due_at"),
            QVariant(QMetaType::fromType<QString>()));
    }
    query.bindValue(QStringLiteral(":created_at"), serializeDateTime(task.createdAt));
    query.bindValue(QStringLiteral(":importance"), task.importance);
    query.bindValue(QStringLiteral(":estimated_minutes"), task.estimatedMinutes);
    query.bindValue(QStringLiteral(":is_completed"), task.completed);

    if (!query.exec()) {
        assignError(errorMessage, query.lastError().text());
        return false;
    }

    return true;
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
    if (completed) {
        query.bindValue(
            QStringLiteral(":completed_at"),
            serializeDateTime(QDateTime::currentDateTime()));
    } else {
        query.bindValue(
            QStringLiteral(":completed_at"),
            QVariant(QMetaType::fromType<QString>()));
    }
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

bool TaskRepository::migrate(QString *errorMessage)
{
    QSqlQuery versionQuery(m_database);
    if (!versionQuery.exec(QStringLiteral("PRAGMA user_version")) || !versionQuery.next()) {
        assignError(errorMessage, versionQuery.lastError().text());
        return false;
    }

    const int version = versionQuery.value(0).toInt();
    if (version > 1) {
        assignError(
            errorMessage,
            QStringLiteral("This database was created by a newer Daymark version."));
        return false;
    }

    if (version == 1) {
        return true;
    }

    if (!m_database.transaction()) {
        assignError(errorMessage, m_database.lastError().text());
        return false;
    }

    const bool created = execute(QStringLiteral(
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
            "CREATE INDEX tasks_open_due_index ON tasks(is_completed, due_at)"), errorMessage)
        && execute(QStringLiteral("PRAGMA user_version = 1"), errorMessage);

    if (!created) {
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
