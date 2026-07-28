// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/appsettings.h"
#include "presentation/meetinglistmodel.h"

#include <QDate>
#include <QTemporaryDir>
#include <QTest>

class MeetingListModelTest : public QObject
{
    Q_OBJECT

private slots:
    void addsAndRemovesAnUpcomingMeeting()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        AppSettings settings(directory.path());
        MeetingListModel model(repository, settings);

        const QString date = QDate::currentDate().addDays(1).toString(Qt::ISODate);
        QVERIFY(model.addMeeting(
            QStringLiteral("Product review"),
            date,
            QStringLiteral("09:30"),
            QStringLiteral("Bring the release checklist.")));

        QCOMPARE(model.meetingCount(), 1);
        QCOMPARE(model.nextMeetingTitle(), QStringLiteral("Product review"));
        QCOMPARE(
            model.data(model.index(0), MeetingListModel::NotesRole).toString(),
            QStringLiteral("Bring the release checklist."));
        QVERIFY(model.data(model.index(0), MeetingListModel::StartsAtRole)
                    .toDateTime().isValid());
        QVERIFY(model.data(model.index(0), MeetingListModel::ReminderTextRole)
                    .toString().contains(QStringLiteral("Reminder")));
        QCOMPARE(repository.meetings(&error).size(), 1);

        QVERIFY(model.removeMeeting(0));
        QCOMPARE(model.meetingCount(), 0);
        QVERIFY(repository.meetings(&error).isEmpty());
    }

    void rejectsInvalidOrPastTimes()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        AppSettings settings(directory.path());
        MeetingListModel model(repository, settings);

        QVERIFY(!model.addMeeting(
            QStringLiteral("Bad date"),
            QStringLiteral("next week"),
            QStringLiteral("09:30")));
        QVERIFY(model.statusMessage().contains(QStringLiteral("YYYY-MM-DD")));

        QVERIFY(!model.addMeeting(
            QStringLiteral("Bad time"),
            QDate::currentDate().addDays(1).toString(Qt::ISODate),
            QStringLiteral("9:30")));
        QVERIFY(model.statusMessage().contains(QStringLiteral("HH:MM")));

        QVERIFY(!model.addMeeting(
            QStringLiteral("Past meeting"),
            QDate::currentDate().addDays(-1).toString(Qt::ISODate),
            QStringLiteral("09:30")));
        QVERIFY(model.statusMessage().contains(QStringLiteral("future")));
        QCOMPARE(model.meetingCount(), 0);
    }
};

QTEST_GUILESS_MAIN(MeetingListModelTest)

#include "meetinglistmodel_test.moc"
