// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/dailynotemodel.h"

#include "domain/dailynote.h"

#include <QTest>

class DailyNoteModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void keepsTodayAndYesterdayOnly()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        const QDate today = QDate::currentDate();
        DailyNote oldNote {today.addDays(-2), QStringLiteral("Too old"), QDateTime::currentDateTime()};
        DailyNote previousNote {
            today.addDays(-1), QStringLiteral("Yesterday's thought"), QDateTime::currentDateTime()};
        DailyNote todayNote {today, QStringLiteral("Today's thought"), QDateTime::currentDateTime()};
        QVERIFY2(repository.saveDailyNote(oldNote, &error), qPrintable(error));
        QVERIFY2(repository.saveDailyNote(previousNote, &error), qPrintable(error));
        QVERIFY2(repository.saveDailyNote(todayNote, &error), qPrintable(error));

        DailyNoteModel model(repository);
        QCOMPARE(model.todayText(), todayNote.text);
        QCOMPARE(model.previousText(), previousNote.text);
        QVERIFY(model.hasPreviousNote());
        QVERIFY(!repository.dailyNote(oldNote.date, &error).has_value());

        model.setTodayText(QStringLiteral("Updated thought"));
        QVERIFY(model.dirty());
        QVERIFY(model.saveNow());
        QVERIFY(!model.dirty());
        QCOMPARE(repository.dailyNote(today, &error)->text, QStringLiteral("Updated thought"));
        QVERIFY(model.deleteToday());
        QVERIFY(model.todayText().isEmpty());
        QVERIFY(!repository.dailyNote(today, &error).has_value());
        QVERIFY(model.saveNow());
        QVERIFY(!repository.dailyNote(today, &error).has_value());
    }
};

QTEST_GUILESS_MAIN(DailyNoteModelTest)

#include "dailynotemodel_test.moc"
