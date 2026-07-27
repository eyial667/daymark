// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/tasklistmodel.h"

#include <QTest>

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
        QCOMPARE(model.topTaskTitle(), QStringLiteral("High priority"));

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
};

QTEST_GUILESS_MAIN(TaskListModelTest)

#include "tasklistmodel_test.moc"
