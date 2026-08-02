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

    void storesNotifiesAndRemovesMeetings()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        Meeting meeting;
        meeting.id = QStringLiteral("meeting-1");
        meeting.title = QStringLiteral("Release planning");
        meeting.notes = QStringLiteral("Confirm the package matrix.");
        meeting.startsAt = QDateTime::currentDateTime().addDays(1);
        meeting.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addMeeting(meeting, &error), qPrintable(error));

        QVector<Meeting> meetings = repository.meetings(&error);
        QCOMPARE(meetings.size(), 1);
        QCOMPARE(meetings.first().title, meeting.title);
        QCOMPARE(meetings.first().notes, meeting.notes);
        QVERIFY(!meetings.first().notifiedAt.isValid());

        const QDateTime notifiedAt = QDateTime::currentDateTime();
        QVERIFY2(
            repository.markMeetingNotified(meeting.id, notifiedAt, &error),
            qPrintable(error));
        QVERIFY(!repository.markMeetingNotified(meeting.id, notifiedAt, &error));
        meetings = repository.meetings(&error);
        QVERIFY(meetings.first().notifiedAt.isValid());

        QVERIFY2(repository.deleteMeeting(meeting.id, &error), qPrintable(error));
        QVERIFY(repository.meetings(&error).isEmpty());
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

        DailyNote note;
        note.date = QDate::currentDate();
        note.text = QStringLiteral("Migration kept working.");
        note.updatedAt = QDateTime::currentDateTime();
        QVERIFY2(repository.saveDailyNote(note, &error), qPrintable(error));
        QCOMPARE(repository.dailyNote(note.date, &error)->text, note.text);

        Meeting meeting;
        meeting.id = QStringLiteral("post-migration-meeting");
        meeting.title = QStringLiteral("Migration review");
        meeting.startsAt = QDateTime::currentDateTime().addDays(1);
        meeting.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addMeeting(meeting, &error), qPrintable(error));
        QCOMPARE(repository.meetings(&error).size(), 1);

        MentalMapGroup mapGroup;
        mapGroup.id = QStringLiteral("post-migration-cloud");
        mapGroup.kind = QStringLiteral("cloud");
        mapGroup.title = QStringLiteral("Migrated brainstorm");
        mapGroup.color = QStringLiteral("accent");
        mapGroup.width = 380;
        mapGroup.height = 280;
        mapGroup.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addMentalMapGroup(mapGroup, &error), qPrintable(error));
        QCOMPARE(repository.mentalMapGroups(&error).size(), 1);
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

    void updatesEditableTaskFieldsWithoutTouchingIdentity()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        Category category;
        category.id = QStringLiteral("editable-category");
        category.name = QStringLiteral("Writing");
        category.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addCategory(category, &error), qPrintable(error));

        Task task;
        task.id = QStringLiteral("editable-task");
        task.title = QStringLiteral("Draft the annoncement");
        task.notes = QStringLiteral("First pass only.");
        task.categoryId = category.id;
        task.createdAt = QDateTime::currentDateTime().addDays(-4);
        task.dueAt = QDateTime::currentDateTime().addDays(1);
        task.plannedDate = QDate::currentDate();
        task.importance = 2;
        task.estimatedMinutes = 60;
        QVERIFY2(repository.addTask(task, &error), qPrintable(error));

        const QDateTime newDueAt = QDateTime::currentDateTime().addDays(6);
        QVERIFY2(
            repository.updateTask(
                task.id,
                QStringLiteral("Draft the announcement"),
                QStringLiteral("Include the migration notes."),
                newDueAt,
                5,
                90,
                &error),
            qPrintable(error));

        const QVector<Task> tasks = repository.openTasks(&error);
        QCOMPARE(tasks.size(), 1);
        const Task &updated = tasks.first();
        QCOMPARE(updated.title, QStringLiteral("Draft the announcement"));
        QCOMPARE(updated.notes, QStringLiteral("Include the migration notes."));
        QCOMPARE(updated.importance, 5);
        QCOMPARE(updated.estimatedMinutes, 90);
        QCOMPARE(updated.dueAt.toSecsSinceEpoch(), newDueAt.toSecsSinceEpoch());

        // Editing must never disturb identity, age, placement, or planning.
        QCOMPARE(updated.id, task.id);
        QCOMPARE(updated.createdAt.toSecsSinceEpoch(), task.createdAt.toSecsSinceEpoch());
        QCOMPARE(updated.categoryId, category.id);
        QCOMPARE(updated.categoryName, category.name);
        QCOMPARE(updated.plannedDate, QDate::currentDate());
    }

    void clearsADeadlineAndRejectsUnknownOrUntitledEdits()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        Task task;
        task.id = QStringLiteral("deadline-task");
        task.title = QStringLiteral("Book the venue");
        task.createdAt = QDateTime::currentDateTime();
        task.dueAt = QDateTime::currentDateTime().addDays(3);
        QVERIFY2(repository.addTask(task, &error), qPrintable(error));

        QVERIFY2(
            repository.updateTask(task.id, task.title, {}, {}, 3, 30, &error),
            qPrintable(error));
        QVERIFY(!repository.openTasks(&error).first().dueAt.isValid());

        QVERIFY(!repository.updateTask(
            QStringLiteral("missing-task"), QStringLiteral("Ghost"), {}, {}, 3, 30, &error));
        QCOMPARE(error, QStringLiteral("The selected task no longer exists."));

        QVERIFY(!repository.updateTask(task.id, {}, {}, {}, 3, 30, &error));
        QCOMPARE(error, QStringLiteral("A task needs a title."));
        QCOMPARE(repository.openTasks(&error).first().title, task.title);
    }

    void restoresACompletedTask()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        Task task;
        task.id = QStringLiteral("restorable-task");
        task.title = QStringLiteral("Send the invoice");
        task.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addTask(task, &error), qPrintable(error));

        QVERIFY2(repository.setCompleted(task.id, true, &error), qPrintable(error));
        QVERIFY(repository.openTasks(&error).isEmpty());
        QVERIFY(repository.allTasks(&error).first().completedAt.isValid());

        QVERIFY2(repository.setCompleted(task.id, false, &error), qPrintable(error));
        const QVector<Task> restored = repository.openTasks(&error);
        QCOMPARE(restored.size(), 1);
        QVERIFY(!restored.first().completed);
        QVERIFY(!restored.first().completedAt.isValid());
    }
};

QTEST_GUILESS_MAIN(TaskRepositoryTest)

#include "taskrepository_test.moc"
