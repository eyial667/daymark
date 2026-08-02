// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/tasklistmodel.h"

#include <QTest>

#include <algorithm>

class TaskListModelTest : public QObject
{
    Q_OBJECT

private slots:
    void ordersTasksAndCompletesTheFirstOne()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel model(repository);
        QCOMPARE(model.activeCount(), 0);

        Category category;
        category.id = QStringLiteral("work-category");
        category.name = QStringLiteral("Work");
        category.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addCategory(category, &error), qPrintable(error));

        Subcategory subcategory;
        subcategory.id = QStringLiteral("deep-work-subcategory");
        subcategory.categoryId = category.id;
        subcategory.name = QStringLiteral("Deep work");
        subcategory.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addSubcategory(subcategory, &error), qPrintable(error));

        QVERIFY(model.addTask(
            QStringLiteral("Low priority"), {}, 1, 60, {}));
        QVERIFY(model.addTask(
            QStringLiteral("High priority"),
            {},
            5,
            25,
            category.id,
            subcategory.id));

        QCOMPARE(model.activeCount(), 2);
        QCOMPARE(model.totalEstimatedMinutes(), 85);
        QVERIFY(!model.topTaskId().isEmpty());
        QCOMPARE(model.topTaskTitle(), QStringLiteral("High priority"));
        QCOMPARE(model.topTaskEstimatedMinutes(), 25);

        const QModelIndex first = model.index(0);
        QCOMPARE(
            model.data(first, TaskListModel::TitleRole).toString(),
            QStringLiteral("High priority"));
        QVERIFY(!model.data(first, TaskListModel::PriorityReasonRole).toString().isEmpty());
        QCOMPARE(model.data(first, TaskListModel::CategoryNameRole).toString(), category.name);
        QCOMPARE(
            model.data(first, TaskListModel::SubcategoryNameRole).toString(),
            subcategory.name);
        QVERIFY(model.assignTaskCategory(
            model.data(first, TaskListModel::IdRole).toString(),
            {},
            {}));
        QCOMPARE(model.data(model.index(0), TaskListModel::CategoryIdRole).toString(), QString());

        QVERIFY(model.completeTask(0));
        QCOMPARE(model.activeCount(), 1);
        QCOMPARE(model.topTaskTitle(), QStringLiteral("Low priority"));
        QCOMPARE(model.completedTodayCount(), 1);
        QCOMPARE(
            model.completedToday().first().toMap().value(QStringLiteral("title")).toString(),
            QStringLiteral("High priority"));
    }

    void rejectsAnInvalidDueDate()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel model(repository);
        QVERIFY(!model.addTask(
            QStringLiteral("Invalid date"),
            QStringLiteral("tomorrow-ish"),
            3,
            30,
            {}));
        QCOMPARE(model.activeCount(), 0);
        QCOMPARE(model.statusMessage(), QStringLiteral("Use YYYY-MM-DD for the due date."));
    }

    void separatesTodayFromTodoAndPlansTheBestSuggestion()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel todoModel(repository);
        TaskListModel todayModel(repository, TaskListModel::Today);
        QVERIFY(todoModel.addTask(
            QStringLiteral("Default backlog task"), {}, 3, 30, {}, {}));
        QVERIFY(todoModel.addTask(
            QStringLiteral("Low priority backlog task"), {}, 1, 60, {}, {}, false));
        QVERIFY(todoModel.addTask(
            QStringLiteral("High priority backlog task"), {}, 5, 25, {}, {}, false));

        todayModel.reload();
        QCOMPARE(todoModel.activeCount(), 3);
        QCOMPARE(todayModel.activeCount(), 0);
        QVERIFY(todayModel.hasBacklogSuggestion());
        QCOMPARE(
            todayModel.backlogSuggestionTitle(),
            QStringLiteral("High priority backlog task"));

        QVERIFY(todayModel.planSuggestedTaskForToday());
        QCOMPARE(todayModel.activeCount(), 1);
        QCOMPARE(todayModel.topTaskTitle(), QStringLiteral("High priority backlog task"));

        const QVector<Task> tasks = repository.openTasks(&error);
        const auto plannedTask = std::find_if(
            tasks.cbegin(),
            tasks.cend(),
            [](const Task &task) {
                return task.title == QStringLiteral("High priority backlog task");
            });
        QVERIFY(plannedTask != tasks.cend());
        QCOMPARE(plannedTask->plannedDate, QDate::currentDate());
    }

    void keepsDueTasksVisibleWithoutPlanningThem()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel todoModel(repository);
        QVERIFY(todoModel.addTask(
            QStringLiteral("Due today"),
            QDate::currentDate().toString(Qt::ISODate),
            3,
            30,
            {},
            {},
            false));
        QVERIFY(todoModel.addTask(
            QStringLiteral("Overdue"),
            QDate::currentDate().addDays(-1).toString(Qt::ISODate),
            3,
            30,
            {},
            {},
            false));
        QVERIFY(todoModel.addTask(
            QStringLiteral("Due tomorrow"),
            QDate::currentDate().addDays(1).toString(Qt::ISODate),
            3,
            30,
            {},
            {},
            false));

        TaskListModel todayModel(repository, TaskListModel::Today);
        QCOMPARE(todayModel.activeCount(), 2);
        QStringList todayTitles;
        for (int row = 0; row < todayModel.rowCount(); ++row) {
            todayTitles.append(
                todayModel.data(todayModel.index(row), TaskListModel::TitleRole).toString());
        }
        QVERIFY(todayTitles.contains(QStringLiteral("Due today")));
        QVERIFY(todayTitles.contains(QStringLiteral("Overdue")));
        QVERIFY(!todayTitles.contains(QStringLiteral("Due tomorrow")));
    }

    void plansAnySelectedTodoTaskForToday()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel todoModel(repository);
        QVERIFY(todoModel.addTask(
            QStringLiteral("Higher priority task"), {}, 5, 30, {}, {}, false));
        QVERIFY(todoModel.addTask(
            QStringLiteral("Chosen lower priority task"), {}, 1, 30, {}, {}, false));

        const QModelIndex chosenIndex = todoModel.index(1);
        const QString chosenId =
            todoModel.data(chosenIndex, TaskListModel::IdRole).toString();
        QVERIFY(!todoModel.data(chosenIndex, TaskListModel::IsInTodayRole).toBool());

        QVERIFY(todoModel.planTaskForToday(chosenId));

        TaskListModel todayModel(repository, TaskListModel::Today);
        QCOMPARE(todayModel.activeCount(), 1);
        QCOMPARE(todayModel.topTaskTitle(), QStringLiteral("Chosen lower priority task"));

        const auto refreshedIndex = todoModel.index(1);
        QVERIFY(todoModel.data(refreshedIndex, TaskListModel::IsInTodayRole).toBool());
        QCOMPARE(
            todoModel.statusMessage(),
            QStringLiteral("Added “Chosen lower priority task” to Today."));
        QVERIFY(!todoModel.planTaskForToday(QStringLiteral("missing-task")));
    }

    void permanentlyDeletesATask()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        TaskListModel model(repository);
        QVERIFY(model.addTask(QStringLiteral("Disposable task"), {}, 3, 30));
        QCOMPARE(model.activeCount(), 1);
        QVERIFY(model.deleteTask(0));
        QCOMPARE(model.activeCount(), 0);
        QVERIFY(repository.allTasks(&error).isEmpty());
        QVERIFY(!model.deleteTask(0));
    }

    void editsATaskAndReRanksTheQueue()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel model(repository);
        QVERIFY(model.addTask(QStringLiteral("Already urgent"), {}, 4, 30));
        QVERIFY(model.addTask(QStringLiteral("Quiet task"), {}, 1, 60));
        QCOMPARE(model.topTaskTitle(), QStringLiteral("Already urgent"));

        const QString quietId =
            model.data(model.index(1), TaskListModel::IdRole).toString();
        QVERIFY(model.updateTask(
            quietId,
            QStringLiteral("  Suddenly critical  "),
            QDate::currentDate().toString(Qt::ISODate),
            5,
            20,
            QStringLiteral("  Escalated by the client.  ")));

        // A raised importance and a deadline must move the task to the top.
        QCOMPARE(model.topTaskTitle(), QStringLiteral("Suddenly critical"));
        QCOMPARE(model.topTaskEstimatedMinutes(), 20);
        QCOMPARE(
            model.statusMessage(),
            QStringLiteral("Task “Suddenly critical” updated."));

        const QModelIndex top = model.index(0);
        QCOMPARE(
            model.data(top, TaskListModel::NotesRole).toString(),
            QStringLiteral("Escalated by the client."));
        QCOMPARE(model.data(top, TaskListModel::ImportanceRole).toInt(), 5);

        const QVariantMap details = model.taskDetails(quietId);
        QCOMPARE(details.value(QStringLiteral("title")).toString(),
            QStringLiteral("Suddenly critical"));
        QCOMPARE(details.value(QStringLiteral("dueDate")).toString(),
            QDate::currentDate().toString(Qt::ISODate));
        QCOMPARE(details.value(QStringLiteral("estimatedMinutes")).toInt(), 20);
        QVERIFY(details.value(QStringLiteral("isInToday")).toBool());
        QVERIFY(model.taskDetails(QStringLiteral("missing-task")).isEmpty());
    }

    void rejectsInvalidEditsWithTheSameMessagesAsCapture()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel model(repository);
        QVERIFY(model.addTask(QStringLiteral("Original title"), {}, 3, 30));
        const QString taskId = model.data(model.index(0), TaskListModel::IdRole).toString();

        QVERIFY(!model.updateTask(
            taskId, QStringLiteral("Renamed"), QStringLiteral("tomorrow-ish"), 3, 30));
        QCOMPARE(model.statusMessage(), QStringLiteral("Use YYYY-MM-DD for the due date."));

        QVERIFY(!model.updateTask(taskId, QStringLiteral("   "), {}, 3, 30));
        QCOMPARE(model.statusMessage(), QStringLiteral("A task needs a title."));

        QVERIFY(!model.updateTask(
            QStringLiteral("missing-task"), QStringLiteral("Ghost"), {}, 3, 30));
        QCOMPARE(model.statusMessage(), QStringLiteral("That task is no longer in the list."));

        // Nothing may have been written by any rejected edit.
        QCOMPARE(model.topTaskTitle(), QStringLiteral("Original title"));
    }

    void postponesATaskOutOfToday()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel todoModel(repository);
        QVERIFY(todoModel.addTask(
            QStringLiteral("Slipping task"),
            QDate::currentDate().toString(Qt::ISODate),
            3,
            30,
            {},
            {},
            true));

        TaskListModel todayModel(repository, TaskListModel::Today);
        QCOMPARE(todayModel.activeCount(), 1);

        const QString taskId =
            todoModel.data(todoModel.index(0), TaskListModel::IdRole).toString();
        QVERIFY(todoModel.postponeTask(taskId, 7));

        // The deadline moves and the task leaves Today, but stays in To-do.
        todayModel.reload();
        QCOMPARE(todayModel.activeCount(), 0);
        QCOMPARE(todoModel.activeCount(), 1);
        QCOMPARE(
            todoModel.taskDetails(taskId).value(QStringLiteral("dueDate")).toString(),
            QDate::currentDate().addDays(7).toString(Qt::ISODate));

        QVERIFY(!todoModel.postponeTask(taskId, 0));
        QCOMPARE(
            todoModel.statusMessage(),
            QStringLiteral("A postponement needs at least one day."));
        QVERIFY(!todoModel.postponeTask(QStringLiteral("missing-task"), 1));
    }

    void postponesAnUndatedTaskRelativeToToday()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel model(repository);
        QVERIFY(model.addTask(QStringLiteral("Undated task"), {}, 3, 30));
        const QString taskId = model.data(model.index(0), TaskListModel::IdRole).toString();

        QVERIFY(model.postponeTask(taskId, 1));
        QCOMPARE(
            model.taskDetails(taskId).value(QStringLiteral("dueDate")).toString(),
            QDate::currentDate().addDays(1).toString(Qt::ISODate));
    }

    void restoresATaskCompletedByMistake()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        TaskListModel model(repository);
        QVERIFY(model.addTask(QStringLiteral("Completed too soon"), {}, 3, 30));
        const QString taskId = model.data(model.index(0), TaskListModel::IdRole).toString();

        QVERIFY(model.completeTask(0));
        QCOMPARE(model.activeCount(), 0);
        QCOMPARE(model.completedTodayCount(), 1);
        QCOMPARE(
            model.completedToday().first().toMap().value(QStringLiteral("taskId")).toString(),
            taskId);

        QVERIFY(model.restoreTask(taskId));
        QCOMPARE(model.activeCount(), 1);
        QCOMPARE(model.completedTodayCount(), 0);
        QCOMPARE(model.topTaskTitle(), QStringLiteral("Completed too soon"));
        QCOMPARE(model.statusMessage(), QStringLiteral("Task restored to the queue."));

        QVERIFY(!model.restoreTask(QStringLiteral("missing-task")));
    }
};

QTEST_GUILESS_MAIN(TaskListModelTest)

#include "tasklistmodel_test.moc"
