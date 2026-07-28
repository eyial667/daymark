// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/goallistmodel.h"

#include <QTest>
#include <QVariantList>

class GoalListModelTest : public QObject
{
    Q_OBJECT

private slots:
    void tracksMilestoneProgressAndCompletesAGoal()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        GoalListModel model(repository);
        QCOMPARE(model.goalCount(), 0);
        QVERIFY(model.addGoal(
            QStringLiteral("Publish Daymark 1.0"),
            QStringLiteral("Create a dependable daily command center."),
            QStringLiteral("2030-12-31")));
        QCOMPARE(model.goalCount(), 1);

        QVERIFY(model.addMilestone(
            0,
            QStringLiteral("Complete the portable backup"),
            QStringLiteral("2029-06-01")));
        QVERIFY(model.addMilestone(
            0,
            QStringLiteral("Ship signed installers"),
            QStringLiteral("2030-01-15")));
        QCOMPARE(model.totalMilestoneCount(), 2);
        QCOMPARE(model.completedMilestoneCount(), 0);
        QCOMPARE(
            model.suggestedMilestoneTitle(),
            QStringLiteral("Complete the portable backup"));

        QVERIFY(model.setMilestoneCompleted(0, 0, true));
        QCOMPARE(model.completedMilestoneCount(), 1);
        QCOMPARE(model.data(model.index(0), GoalListModel::ProgressRole).toDouble(), 0.5);
        QVERIFY(model.hasMilestoneSuggestion());
        QCOMPARE(
            model.suggestedMilestoneTitle(),
            QStringLiteral("Ship signed installers"));
        QCOMPARE(model.suggestedMilestoneGoal(), QStringLiteral("Publish Daymark 1.0"));

        QVERIFY(model.planSuggestedMilestone(4, 45));
        const QVector<Task> plannedTasks = repository.openTasks(&error);
        QCOMPARE(plannedTasks.size(), 1);
        QCOMPARE(plannedTasks.first().title, QStringLiteral("Ship signed installers"));
        QCOMPARE(plannedTasks.first().plannedDate, QDate::currentDate());
        QCOMPARE(plannedTasks.first().importance, 4);
        QCOMPARE(plannedTasks.first().estimatedMinutes, 45);
        QVERIFY(plannedTasks.first().notes.contains(QStringLiteral("Publish Daymark 1.0")));

        const QVariantList milestones = model.data(
            model.index(0),
            GoalListModel::MilestonesRole).toList();
        QCOMPARE(milestones.size(), 2);
        QVERIFY(milestones.first().toMap().value(QStringLiteral("completed")).toBool());

        QVERIFY(model.completeGoal(0));
        QCOMPARE(model.goalCount(), 0);
        const QVector<Goal> storedGoals = repository.goals(&error);
        QCOMPARE(storedGoals.size(), 1);
        QVERIFY(storedGoals.first().completed);
    }

    void rejectsInvalidTargetDates()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        GoalListModel model(repository);
        QVERIFY(!model.addGoal(
            QStringLiteral("Invalid target"),
            {},
            QStringLiteral("sometime next year")));
        QCOMPARE(model.goalCount(), 0);
        QCOMPARE(
            model.statusMessage(),
            QStringLiteral("Use YYYY-MM-DD for the target date."));
    }

    void permanentlyDeletesMilestonesAndGoals()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        GoalListModel model(repository);
        QVERIFY(model.addGoal(QStringLiteral("Temporary goal"), {}, {}));
        QVERIFY(model.addMilestone(0, QStringLiteral("Temporary milestone"), {}));
        QVERIFY(model.deleteMilestone(0, 0));
        QCOMPARE(model.totalMilestoneCount(), 0);
        QVERIFY(model.deleteGoal(0));
        QCOMPARE(model.goalCount(), 0);
        QVERIFY(repository.goals(&error).isEmpty());
    }
};

QTEST_GUILESS_MAIN(GoalListModelTest)

#include "goallistmodel_test.moc"
