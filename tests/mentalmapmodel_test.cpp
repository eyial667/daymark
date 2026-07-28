// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/mentalmapmodel.h"

#include <QSignalSpy>
#include <QTest>

class MentalMapModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void persistsMovableGroupsNotesAndDirectedConnections()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        MentalMapModel model(repository);

        const QString firstGroup = model.addGroup(
            QStringLiteral("cloud"), QStringLiteral("Customer needs"), 80, 120);
        const QString secondGroup = model.addGroup(
            QStringLiteral("cloud"), QStringLiteral("Open questions"), 620, 160);
        QVERIFY(!firstGroup.isEmpty());
        QVERIFY(!secondGroup.isEmpty());

        const QString firstNote = model.addNote(
            firstGroup, QStringLiteral("Faster onboarding"), 20, 72);
        const QString secondNote = model.addNote(
            secondGroup, QStringLiteral("Which segment?"), 24, 76);
        QVERIFY2(!firstNote.isEmpty(), qPrintable(model.statusMessage()));
        QVERIFY2(!secondNote.isEmpty(), qPrintable(model.statusMessage()));
        QVERIFY(model.updateNote(
            firstNote,
            QStringLiteral("Faster onboarding"),
            QStringLiteral("Remove setup friction."),
            QStringLiteral("success"),
            QStringLiteral("research, customer"),
            QStringLiteral("https://example.com"),
            4));
        QVERIFY(model.addChecklistItem(firstNote, QStringLiteral("Validate with users")));
        QVERIFY(model.toggleChecklistItem(firstNote, 0, true));
        QVERIFY(!model.addConnection(
            firstNote, secondNote, QStringLiteral("raises question")).isEmpty());
        QVERIFY(model.moveGroup(firstGroup, -240.5, 410.25));
        QVERIFY(model.moveNote(firstNote, 42.0, 96.0));

        MentalMapModel reloaded(repository);
        QCOMPARE(reloaded.groups().size(), 2);
        QCOMPARE(reloaded.notes().size(), 2);
        QCOMPARE(reloaded.connections().size(), 1);
        const QVariantMap storedNote = reloaded.notes().first().toMap();
        QCOMPARE(storedNote.value(QStringLiteral("priority")).toInt(), 4);
        QCOMPARE(storedNote.value(QStringLiteral("checklist")).toList().size(), 1);
    }

    void constellationOwnsItsCenterAndDeletesAsOneUnit()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        MentalMapModel model(repository);

        const QString constellation = model.addGroup(
            QStringLiteral("constellation"), QStringLiteral("Launch plan"), 100, 100);
        QVERIFY2(!constellation.isEmpty(), qPrintable(model.statusMessage()));
        QCOMPARE(model.groups().size(), 1);
        QCOMPARE(model.notes().size(), 1);
        const QVariantMap center = model.notes().first().toMap();
        QVERIFY(center.value(QStringLiteral("center")).toBool());
        QVERIFY(!model.deleteNote(center.value(QStringLiteral("noteId")).toString()));

        const QString orbiting = model.addNote(
            constellation, QStringLiteral("Beta cohort"), 20, 20);
        QVERIFY(!orbiting.isEmpty());
        QVERIFY(model.deleteGroup(constellation));
        QCOMPARE(model.groups().size(), 0);
        QCOMPARE(model.notes().size(), 0);
    }

    void linkedTaskSurvivesNoteDeletionAndTaskDeletionUnlinksNote()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        MentalMapModel model(repository);
        QSignalSpy taskSpy(&model, &MentalMapModel::taskCreated);

        const QString group = model.addGroup(
            QStringLiteral("cloud"), QStringLiteral("Actions"), 0, 0);
        const QString firstNote = model.addNote(
            group, QStringLiteral("Call the venue"), 20, 72);
        QVERIFY2(model.createTaskForNote(firstNote), qPrintable(model.statusMessage()));
        QCOMPARE(taskSpy.count(), 1);
        const QString firstTask = model.notes().first().toMap()
            .value(QStringLiteral("linkedTaskId")).toString();
        QVERIFY(!firstTask.isEmpty());
        QVERIFY(model.deleteNote(firstNote));
        QCOMPARE(repository.allTasks(&error).size(), 1);

        const QString secondNote = model.addNote(
            group, QStringLiteral("Send the agenda"), 20, 72);
        QVERIFY(model.createTaskForNote(secondNote));
        model.reload();
        QString secondTask;
        for (const QVariant &value : model.notes()) {
            const QVariantMap note = value.toMap();
            if (note.value(QStringLiteral("noteId")).toString() == secondNote)
                secondTask = note.value(QStringLiteral("linkedTaskId")).toString();
        }
        QVERIFY(repository.deleteTask(secondTask, &error));
        model.reload();
        QCOMPARE(model.notes().first().toMap()
            .value(QStringLiteral("linkedTaskId")).toString(), QString());
    }
};

QTEST_GUILESS_MAIN(MentalMapModelTest)

#include "mentalmapmodel_test.moc"
