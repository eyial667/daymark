// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/categorylistmodel.h"

#include <QTest>

class CategoryListModelTest : public QObject
{
    Q_OBJECT

private slots:
    void createsAndUpdatesCategoriesWithNotes()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        CategoryListModel model(repository);
        QCOMPARE(model.categoryCount(), 0);
        QVERIFY(model.addCategory(
            QStringLiteral("Health"),
            QStringLiteral("Movement, sleep, and appointments.")));
        QCOMPARE(model.categoryCount(), 1);
        QCOMPARE(model.names(), QStringList {QStringLiteral("Health")});
        QCOMPARE(
            model.data(model.index(0), CategoryListModel::NotesRole).toString(),
            QStringLiteral("Movement, sleep, and appointments."));

        QVERIFY(model.updateCategory(
            0,
            QStringLiteral("Wellbeing"),
            QStringLiteral("Health and sustainable routines.")));
        QCOMPARE(model.names(), QStringList {QStringLiteral("Wellbeing")});
        QVERIFY(!model.idAt(0).isEmpty());
        QCOMPARE(model.indexOfId(model.idAt(0)), 0);

        QVERIFY(model.addSubcategory(
            0,
            QStringLiteral("Routines"),
            QStringLiteral("Repeatable daily and weekly actions.")));
        QCOMPARE(model.subcategoryCount(), 1);
        QCOMPARE(
            model.assignmentNames(),
            QStringList({QStringLiteral("Wellbeing"), QStringLiteral("Wellbeing / Routines")}));
        QCOMPARE(model.categoryIdForAssignment(0), model.idAt(0));
        QCOMPARE(model.categoryIdForAssignment(1), model.idAt(0));
        QVERIFY(model.subcategoryIdForAssignment(0).isEmpty());
        QVERIFY(!model.subcategoryIdForAssignment(1).isEmpty());
        QCOMPARE(
            model.indexOfAssignment(model.idAt(0), model.subcategoryIdForAssignment(1)),
            1);
        const QVariantList subcategories = model.subcategoriesAt(0);
        QCOMPARE(subcategories.size(), 1);
        QCOMPARE(
            subcategories.first().toMap().value(QStringLiteral("notes")).toString(),
            QStringLiteral("Repeatable daily and weekly actions."));

        QVERIFY(model.updateSubcategory(
            0,
            0,
            QStringLiteral("Healthy routines"),
            QStringLiteral("Updated subcategory notes.")));

        Task task;
        task.id = QStringLiteral("routine-task");
        task.title = QStringLiteral("Take a walk");
        task.categoryId = model.idAt(0);
        task.subcategoryId = model.subcategoryIdForAssignment(1);
        task.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addTask(task, &error), qPrintable(error));
        model.reload();
        QVERIFY(model.hasWorkSuggestion());
        QCOMPARE(model.workSuggestionCount(), 1);
        QCOMPARE(
            model.workSuggestionName(),
            QStringLiteral("Wellbeing / Healthy routines"));
        QCOMPARE(model.workSuggestionCategoryId(), task.categoryId);
        QCOMPARE(model.workSuggestionSubcategoryId(), task.subcategoryId);
        QVERIFY(model.workSuggestionDetail().contains(QStringLiteral("1 open task")));

        QVERIFY(!model.addSubcategory(0, QStringLiteral("healthy routines"), {}));
        QVERIFY(model.statusMessage().contains(QStringLiteral("already has")));

        QVERIFY(!model.addCategory(QStringLiteral("wellbeing"), {}));
        QVERIFY(model.statusMessage().contains(QStringLiteral("already exists")));
    }

    void cyclesAvailableWorkAreaSuggestions()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));

        CategoryListModel model(repository);
        QVERIFY(model.addCategory(QStringLiteral("Home"), QStringLiteral("Household work.")));
        QVERIFY(model.addCategory(QStringLiteral("Work"), QStringLiteral("Professional work.")));
        QCOMPARE(model.workSuggestionCount(), 2);

        const QString firstSuggestion = model.workSuggestionName();
        model.advanceWorkSuggestion();
        QVERIFY(model.workSuggestionName() != firstSuggestion);
        model.advanceWorkSuggestion();
        QCOMPARE(model.workSuggestionName(), firstSuggestion);
    }

    void deletesCategoriesAndSubcategoriesWithoutDeletingTasks()
    {
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        CategoryListModel model(repository);
        QVERIFY(model.addCategory(QStringLiteral("Work"), {}));
        QVERIFY(model.addSubcategory(0, QStringLiteral("Planning"), {}));

        Task task;
        task.id = QStringLiteral("kept-task");
        task.title = QStringLiteral("Keep me");
        task.categoryId = model.idAt(0);
        task.subcategoryId = model.subcategoryIdForAssignment(1);
        task.createdAt = QDateTime::currentDateTime();
        QVERIFY2(repository.addTask(task, &error), qPrintable(error));

        QVERIFY(model.deleteSubcategory(0, 0));
        QVector<Task> tasks = repository.openTasks(&error);
        QCOMPARE(tasks.size(), 1);
        QVERIFY(tasks.first().subcategoryId.isEmpty());
        QVERIFY(!tasks.first().categoryId.isEmpty());
        QVERIFY(model.deleteCategory(0));
        tasks = repository.openTasks(&error);
        QCOMPARE(tasks.size(), 1);
        QVERIFY(tasks.first().categoryId.isEmpty());
    }
};

QTEST_GUILESS_MAIN(CategoryListModelTest)

#include "categorylistmodel_test.moc"
