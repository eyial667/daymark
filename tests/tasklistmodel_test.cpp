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
            model.completedTodayTitles(),
            QStringList {QStringLiteral("High priority")});
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
};

QTEST_GUILESS_MAIN(TaskListModelTest)

#include "tasklistmodel_test.moc"
