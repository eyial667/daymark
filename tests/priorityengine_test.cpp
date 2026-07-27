// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/priorityengine.h"

#include <QTest>
#include <QTimeZone>

class PriorityEngineTest : public QObject
{
    Q_OBJECT

private slots:
    void dueTodayOutranksNextWeek()
    {
        const QDateTime now(QDate(2026, 7, 27), QTime(9, 0), QTimeZone::UTC);

        Task today;
        today.title = QStringLiteral("Today");
        today.importance = 3;
        today.estimatedMinutes = 60;
        today.createdAt = now;
        today.dueAt = QDateTime(QDate(2026, 7, 27), QTime(17, 0), QTimeZone::UTC);

        Task nextWeek = today;
        nextWeek.title = QStringLiteral("Next week");
        nextWeek.dueAt = QDateTime(QDate(2026, 8, 5), QTime(17, 0), QTimeZone::UTC);

        const PriorityResult todayResult = PriorityEngine::evaluate(today, now);
        const PriorityResult nextWeekResult = PriorityEngine::evaluate(nextWeek, now);

        QVERIFY(todayResult.score > nextWeekResult.score);
        QVERIFY(todayResult.reasons.contains(QStringLiteral("Due today")));
    }

    void overdueWorkGetsAnExplicitReason()
    {
        const QDateTime now(QDate(2026, 7, 27), QTime(9, 0), QTimeZone::UTC);
        Task task;
        task.title = QStringLiteral("Late task");
        task.importance = 2;
        task.estimatedMinutes = 90;
        task.createdAt = now.addDays(-5);
        task.dueAt = now.addDays(-1);

        const PriorityResult result = PriorityEngine::evaluate(task, now);

        QVERIFY(result.reasons.contains(QStringLiteral("Overdue")));
        QVERIFY(result.reasons.contains(QStringLiteral("Waiting for 5 days")));
    }

    void shortTasksReceiveOnlyABoundedBoost()
    {
        const QDateTime now(QDate(2026, 7, 27), QTime(9, 0), QTimeZone::UTC);
        Task shortTask;
        shortTask.importance = 3;
        shortTask.estimatedMinutes = 15;
        shortTask.createdAt = now;

        Task longTask = shortTask;
        longTask.estimatedMinutes = 120;

        QCOMPARE(
            PriorityEngine::evaluate(shortTask, now).score
                - PriorityEngine::evaluate(longTask, now).score,
            6);
    }
};

QTEST_APPLESS_MAIN(PriorityEngineTest)

#include "priorityengine_test.moc"
