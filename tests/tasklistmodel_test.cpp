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

        QVERIFY(model.addTask(
            QStringLiteral("Low priority"), {}, 1, 60, QStringLiteral("Personal")));
        QVERIFY(model.addTask(
            QStringLiteral("High priority"), {}, 5, 25, QStringLiteral("Daymark")));

        QCOMPARE(model.activeCount(), 2);
        QCOMPARE(model.totalEstimatedMinutes(), 85);
        QCOMPARE(model.topTaskTitle(), QStringLiteral("High priority"));

        const QModelIndex first = model.index(0);
        QCOMPARE(
            model.data(first, TaskListModel::TitleRole).toString(),
            QStringLiteral("High priority"));
        QVERIFY(!model.data(first, TaskListModel::PriorityReasonRole).toString().isEmpty());

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
