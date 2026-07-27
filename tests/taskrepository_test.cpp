// SPDX-License-Identifier: GPL-3.0-or-later

#include "data/taskrepository.h"

#include <QTest>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QUuid>

class TaskRepositoryTest : public QObject
{
    Q_OBJECT

private slots:
    void storesAndCompletesATask()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        Category category;
        category.id = QStringLiteral("daymark-category");
        category.name = QStringLiteral("Product");
        category.notes = QStringLiteral("Tasks that improve the Daymark product.");
        category.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addCategory(category, &error), qPrintable(error));

        Subcategory subcategory;
        subcategory.id = QStringLiteral("daymark-subcategory");
        subcategory.categoryId = category.id;
        subcategory.name = QStringLiteral("Desktop");
        subcategory.notes = QStringLiteral("Native desktop application work.");
        subcategory.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addSubcategory(subcategory, &error), qPrintable(error));

        Task task;
        task.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        task.title = QStringLiteral("Write the first test");
        task.categoryId = category.id;
        task.subcategoryId = subcategory.id;
        task.createdAt = QDateTime::currentDateTime();
        task.dueAt = task.createdAt.addDays(1);
        task.plannedDate = QDate::currentDate();
        task.importance = 4;
        task.estimatedMinutes = 25;

        QVERIFY2(repository.addTask(task, &error), qPrintable(error));

        QVector<Task> tasks = repository.openTasks(&error);
        QCOMPARE(tasks.size(), 1);
        QCOMPARE(tasks.first().title, task.title);
        QCOMPARE(tasks.first().categoryId, category.id);
        QCOMPARE(tasks.first().categoryName, category.name);
        QCOMPARE(tasks.first().subcategoryId, subcategory.id);
        QCOMPARE(tasks.first().subcategoryName, subcategory.name);
        QCOMPARE(tasks.first().importance, 4);
        QCOMPARE(tasks.first().plannedDate, QDate::currentDate());

        const QDate replannedDate = QDate::currentDate().addDays(2);
        QVERIFY2(repository.setTaskPlannedDate(task.id, replannedDate, &error), qPrintable(error));
        tasks = repository.openTasks(&error);
        QCOMPARE(tasks.first().plannedDate, replannedDate);

        QVector<Category> categories = repository.categories(&error);
        QCOMPARE(categories.size(), 1);
        QCOMPARE(categories.first().notes, category.notes);
        QCOMPARE(categories.first().taskCount, 1);
        QCOMPARE(categories.first().subcategories.size(), 1);
        QCOMPARE(categories.first().subcategories.first().notes, subcategory.notes);
        QCOMPARE(categories.first().subcategories.first().taskCount, 1);

        category.name = QStringLiteral("Daymark product");
        category.notes = QStringLiteral("Updated category notes.");
        QVERIFY2(repository.updateCategory(category, &error), qPrintable(error));
        tasks = repository.openTasks(&error);
        QCOMPARE(tasks.first().categoryName, category.name);

        subcategory.name = QStringLiteral("Qt desktop");
        subcategory.notes = QStringLiteral("Updated subcategory notes.");
        QVERIFY2(repository.updateSubcategory(subcategory, &error), qPrintable(error));
        tasks = repository.openTasks(&error);
        QCOMPARE(tasks.first().subcategoryName, subcategory.name);

        QVERIFY2(repository.setTaskCategory(task.id, {}, {}, &error), qPrintable(error));
        tasks = repository.openTasks(&error);
        QVERIFY(tasks.first().categoryId.isEmpty());

        QVERIFY2(
            repository.setTaskCategory(task.id, category.id, subcategory.id, &error),
            qPrintable(error));
        QVERIFY2(repository.setCompleted(task.id, true, &error), qPrintable(error));
        tasks = repository.openTasks(&error);
        QVERIFY(tasks.isEmpty());
        categories = repository.categories(&error);
        QCOMPARE(categories.first().taskCount, 0);
        QCOMPARE(categories.first().subcategories.first().taskCount, 0);
    }

    void persistsTasksAcrossConnections()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString databasePath = directory.filePath(QStringLiteral("daymark.sqlite3"));

        const QString taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        {
            TaskRepository repository(databasePath);
            QString error;
            QVERIFY2(repository.open(&error), qPrintable(error));

            Task task;
            task.id = taskId;
            task.title = QStringLiteral("Persistent task");
            task.createdAt = QDateTime::currentDateTime();
            task.importance = 3;
            task.estimatedMinutes = 30;
            QVERIFY2(repository.addTask(task, &error), qPrintable(error));
        }

        {
            TaskRepository repository(databasePath);
            QString error;
            QVERIFY2(repository.open(&error), qPrintable(error));
            const QVector<Task> tasks = repository.openTasks(&error);
            QCOMPARE(tasks.size(), 1);
            QCOMPARE(tasks.first().id, taskId);
            QCOMPARE(tasks.first().title, QStringLiteral("Persistent task"));
        }
    }

    void migratesVersionOneWithoutLosingTasks()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString databasePath = directory.filePath(QStringLiteral("daymark.sqlite3"));
        const QString connectionName = QStringLiteral("legacy-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

        {
            QSqlDatabase database = QSqlDatabase::addDatabase(
                QStringLiteral("QSQLITE"),
                connectionName);
            database.setDatabaseName(databasePath);
            QVERIFY2(database.open(), qPrintable(database.lastError().text()));

            QSqlQuery query(database);
            QVERIFY2(query.exec(QStringLiteral(
                "CREATE TABLE tasks ("
                "id TEXT PRIMARY KEY NOT NULL, title TEXT NOT NULL, "
                "notes TEXT NOT NULL DEFAULT '', project TEXT NOT NULL DEFAULT '', "
                "due_at TEXT, created_at TEXT NOT NULL, importance INTEGER NOT NULL, "
                "estimated_minutes INTEGER NOT NULL, is_completed INTEGER NOT NULL, "
                "completed_at TEXT)")), qPrintable(query.lastError().text()));
            QVERIFY2(query.exec(QStringLiteral(
                "INSERT INTO tasks VALUES "
                "('legacy-task', 'Keep this task', '', '', NULL, "
                "'2026-01-01T09:00:00.000Z', 3, 30, 0, NULL)")),
                qPrintable(query.lastError().text()));
            QVERIFY2(query.exec(QStringLiteral("PRAGMA user_version = 1")),
                qPrintable(query.lastError().text()));
            database.close();
        }
        QSqlDatabase::removeDatabase(connectionName);

        TaskRepository repository(databasePath);
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        const QVector<Task> tasks = repository.openTasks(&error);
        QCOMPARE(tasks.size(), 1);
        QCOMPARE(tasks.first().title, QStringLiteral("Keep this task"));

        Goal goal;
        goal.id = QStringLiteral("new-goal");
        goal.title = QStringLiteral("A migrated goal");
        goal.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addGoal(goal, &error), qPrintable(error));
        QCOMPARE(repository.goals(&error).size(), 1);

        Category category;
        category.id = QStringLiteral("post-migration-category");
        category.name = QStringLiteral("Migrated data");
        category.notes = QStringLiteral("Created after migrating a version-one database.");
        category.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addCategory(category, &error), qPrintable(error));
        QCOMPARE(repository.categories(&error).size(), 1);
    }

    void migratesLegacyProjectsIntoCategories()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString databasePath = directory.filePath(QStringLiteral("daymark.sqlite3"));
        const QString connectionName = QStringLiteral("version-three-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

        {
            QSqlDatabase database = QSqlDatabase::addDatabase(
                QStringLiteral("QSQLITE"),
                connectionName);
            database.setDatabaseName(databasePath);
            QVERIFY2(database.open(), qPrintable(database.lastError().text()));
            QSqlQuery query(database);
            QVERIFY2(query.exec(QStringLiteral(
                "CREATE TABLE categories ("
                "id TEXT PRIMARY KEY NOT NULL, name TEXT NOT NULL COLLATE NOCASE UNIQUE, "
                "notes TEXT NOT NULL DEFAULT '', created_at TEXT NOT NULL)")),
                qPrintable(query.lastError().text()));
            QVERIFY2(query.exec(QStringLiteral(
                "CREATE TABLE tasks ("
                "id TEXT PRIMARY KEY NOT NULL, title TEXT NOT NULL, notes TEXT NOT NULL, "
                "project TEXT NOT NULL, due_at TEXT, created_at TEXT NOT NULL, "
                "importance INTEGER NOT NULL, estimated_minutes INTEGER NOT NULL, "
                "is_completed INTEGER NOT NULL, completed_at TEXT, "
                "category_id TEXT REFERENCES categories(id) ON DELETE SET NULL)")),
                qPrintable(query.lastError().text()));
            QVERIFY2(query.exec(QStringLiteral(
                "INSERT INTO tasks VALUES ('legacy-project-task', 'Keep this work', '', "
                "'Client work', NULL, '2026-01-01T09:00:00.000Z', 3, 30, 0, NULL, NULL)")),
                qPrintable(query.lastError().text()));
            QVERIFY2(query.exec(QStringLiteral("PRAGMA user_version = 3")),
                qPrintable(query.lastError().text()));
            database.close();
        }
        QSqlDatabase::removeDatabase(connectionName);

        TaskRepository repository(databasePath);
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        const QVector<Category> categories = repository.categories(&error);
        QCOMPARE(categories.size(), 1);
        QCOMPARE(categories.first().name, QStringLiteral("Client work"));
        QCOMPARE(
            categories.first().notes,
            QStringLiteral("Migrated from the former Projects field."));
        const QVector<Task> tasks = repository.allTasks(&error);
        QCOMPARE(tasks.size(), 1);
        QCOMPARE(tasks.first().categoryId, categories.first().id);
        QCOMPARE(tasks.first().categoryName, QStringLiteral("Client work"));
        QVERIFY(tasks.first().project.isEmpty());

        const QDate plannedDate(2026, 2, 3);
        QVERIFY2(
            repository.setTaskPlannedDate(tasks.first().id, plannedDate, &error),
            qPrintable(error));
        QCOMPARE(repository.openTasks(&error).first().plannedDate, plannedDate);
    }
};

QTEST_GUILESS_MAIN(TaskRepositoryTest)

#include "taskrepository_test.moc"
