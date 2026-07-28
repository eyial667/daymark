// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/meetingreminderservice.h"

#include <QSignalSpy>
#include <QTest>

#include <algorithm>

class MeetingReminderServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void emitsOneReminderAtTheTenMinuteBoundary()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        const QDateTime now = QDateTime::currentDateTime();
        Meeting due;
        due.id = QStringLiteral("due-meeting");
        due.title = QStringLiteral("Design review");
        due.notes = QStringLiteral("Room 4");
        due.startsAt = now.addSecs(10 * 60);
        due.createdAt = now.addDays(-1);
        QVERIFY2(repository.addMeeting(due, &error), qPrintable(error));

        Meeting future = due;
        future.id = QStringLiteral("future-meeting");
        future.title = QStringLiteral("Later review");
        future.startsAt = now.addSecs(10 * 60 + 1);
        QVERIFY2(repository.addMeeting(future, &error), qPrintable(error));

        Meeting past = due;
        past.id = QStringLiteral("past-meeting");
        past.title = QStringLiteral("Finished review");
        past.startsAt = now.addSecs(-1);
        QVERIFY2(repository.addMeeting(past, &error), qPrintable(error));

        MeetingReminderService service(repository);
        QSignalSpy notificationSpy(
            &service,
            &MeetingReminderService::notificationRequested);
        QSignalSpy changedSpy(&service, &MeetingReminderService::remindersChanged);

        service.processDueMeetings(now);
        QCOMPARE(notificationSpy.size(), 1);
        QCOMPARE(changedSpy.size(), 1);
        QCOMPARE(
            notificationSpy.first().at(0).toString(),
            QStringLiteral("Meeting reminder"));
        QVERIFY(notificationSpy.first().at(1).toString().contains(due.title));
        QVERIFY(notificationSpy.first().at(1).toString().contains(due.notes));

        const QVector<Meeting> stored = repository.meetings(&error);
        const auto dueMeeting = std::find_if(
            stored.cbegin(),
            stored.cend(),
            [](const Meeting &meeting) { return meeting.id == QStringLiteral("due-meeting"); });
        QVERIFY(dueMeeting != stored.cend());
        QVERIFY(dueMeeting->notifiedAt.isValid());

        service.processDueMeetings(now);
        QCOMPARE(notificationSpy.size(), 1);
        QCOMPARE(changedSpy.size(), 1);

        service.processDueMeetings(now.addSecs(1));
        QCOMPARE(notificationSpy.size(), 2);
        QCOMPARE(changedSpy.size(), 2);
        QVERIFY(notificationSpy.at(1).at(1).toString().contains(future.title));
    }
};

QTEST_GUILESS_MAIN(MeetingReminderServiceTest)

#include "meetingreminderservice_test.moc"
