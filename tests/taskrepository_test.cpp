// SPDX-License-Identifier: GPL-3.0-or-later

#include "data/taskrepository.h"

#include <QTest>
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

        Task task;
        task.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        task.title = QStringLiteral("Write the first test");
        task.project = QStringLiteral("Daymark");
        task.createdAt = QDateTime::currentDateTime();
        task.dueAt = task.createdAt.addDays(1);
        task.importance = 4;
        task.estimatedMinutes = 25;

        QVERIFY2(repository.addTask(task, &error), qPrintable(error));

        QVector<Task> tasks = repository.openTasks(&error);
        QCOMPARE(tasks.size(), 1);
        QCOMPARE(tasks.first().title, task.title);
        QCOMPARE(tasks.first().project, task.project);
        QCOMPARE(tasks.first().importance, 4);

        QVERIFY2(repository.setCompleted(task.id, true, &error), qPrintable(error));
        tasks = repository.openTasks(&error);
        QVERIFY(tasks.isEmpty());
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
};

QTEST_GUILESS_MAIN(TaskRepositoryTest)

#include "taskrepository_test.moc"
