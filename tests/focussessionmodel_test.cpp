// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/focussessionmodel.h"

#include <QTest>

class FocusSessionModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void preparesRunsPausesAndResetsATask()
    {
        FocusSessionModel model;
        QVERIFY(!model.start());
        QVERIFY(model.statusMessage().contains(QStringLiteral("Choose a task")));

        QVERIFY(model.prepareTask(
            QStringLiteral("task-id"),
            QStringLiteral("Write the report"),
            1));
        QVERIFY(model.hasTask());
        QCOMPARE(model.taskTitle(), QStringLiteral("Write the report"));
        QCOMPARE(model.totalSeconds(), 60);
        QCOMPARE(model.remainingText(), QStringLiteral("01:00"));

        QVERIFY(model.start());
        QVERIFY(model.running());
        QTRY_COMPARE_WITH_TIMEOUT(model.remainingSeconds(), 59, 1500);
        model.pause();
        QVERIFY(!model.running());

        model.reset();
        QCOMPARE(model.remainingSeconds(), 60);
        QVERIFY(!model.finished());
        model.clear();
        QVERIFY(!model.hasTask());
    }
};

QTEST_GUILESS_MAIN(FocusSessionModelTest)

#include "focussessionmodel_test.moc"
