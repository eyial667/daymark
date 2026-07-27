// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/mentalmapmodel.h"

#include <QTest>

class MentalMapModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void buildsOneHierarchyFromActiveWork()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        Category category;
        category.id = QStringLiteral("product");
        category.name = QStringLiteral("Product");
        category.notes = QStringLiteral("Build the application.");
        category.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addCategory(category, &error), qPrintable(error));

        Subcategory subcategory;
        subcategory.id = QStringLiteral("release");
        subcategory.categoryId = category.id;
        subcategory.name = QStringLiteral("Release");
        subcategory.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addSubcategory(subcategory, &error), qPrintable(error));

        Task task;
        task.id = QStringLiteral("ship-task");
        task.title = QStringLiteral("Ship the build");
        task.categoryId = category.id;
        task.subcategoryId = subcategory.id;
        task.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addTask(task, &error), qPrintable(error));

        Goal goal;
        goal.id = QStringLiteral("launch-goal");
        goal.title = QStringLiteral("Launch Daymark");
        goal.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addGoal(goal, &error), qPrintable(error));

        Milestone milestone;
        milestone.id = QStringLiteral("beta-milestone");
        milestone.goalId = goal.id;
        milestone.title = QStringLiteral("Release beta");
        milestone.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addMilestone(milestone, &error), qPrintable(error));

        MentalMapModel model(repository);
        QCOMPARE(model.hierarchy().size(), 2);
        QCOMPARE(model.itemCount(), 5);

        QStringList kinds;
        QStringList titles;
        for (const QVariant &nodeValue : model.flatNodes()) {
            const QVariantMap node = nodeValue.toMap();
            kinds.append(node.value(QStringLiteral("kind")).toString());
            titles.append(node.value(QStringLiteral("title")).toString());
            QVERIFY(node.contains(QStringLiteral("angle")));
            QVERIFY(node.contains(QStringLiteral("radius")));
        }
        QVERIFY(kinds.contains(QStringLiteral("category")));
        QVERIFY(kinds.contains(QStringLiteral("subcategory")));
        QVERIFY(kinds.contains(QStringLiteral("task")));
        QVERIFY(kinds.contains(QStringLiteral("goal")));
        QVERIFY(kinds.contains(QStringLiteral("milestone")));
        QVERIFY(titles.contains(QStringLiteral("Ship the build")));
        QVERIFY(titles.contains(QStringLiteral("Release beta")));

        QVERIFY2(repository.setCompleted(task.id, true, &error), qPrintable(error));
        model.reload();
        QCOMPARE(model.itemCount(), 4);
    }
};

QTEST_GUILESS_MAIN(MentalMapModelTest)

#include "mentalmapmodel_test.moc"
