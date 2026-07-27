// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/appsettings.h"
#include "presentation/categorylistmodel.h"
#include "presentation/datatransferservice.h"
#include "presentation/goallistmodel.h"
#include "presentation/tasklistmodel.h"

#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QUuid>

#include <algorithm>

class DataTransferTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        QSettings::setDefaultFormat(QSettings::IniFormat);
    }

    void exportsAndMergesAllPortableData()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
        QCoreApplication::setOrganizationName(QStringLiteral("DaymarkTransferTest"));
        QCoreApplication::setApplicationName(QStringLiteral("Source"));
        QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0-test"));

        TaskRepository sourceRepository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(sourceRepository.open(&error), qPrintable(error));
        TaskListModel sourceTasks(sourceRepository);
        CategoryListModel sourceCategories(sourceRepository);
        GoalListModel sourceGoals(sourceRepository);
        AppSettings sourceSettings(directory.filePath(QStringLiteral("source-data")));
        DataTransferService sourceTransfer(
            sourceRepository,
            sourceTasks,
            sourceCategories,
            sourceGoals,
            sourceSettings);

        QVERIFY(sourceCategories.addCategory(
            QStringLiteral("Portable category"),
            QStringLiteral("These notes must move with the category.")));
        const QString sourceCategoryId = sourceCategories.idAt(0);

        QVERIFY(sourceTasks.addTask(
            QStringLiteral("Open task"),
            QStringLiteral("2030-01-01"),
            4,
            45,
            QStringLiteral("Transfer"),
            sourceCategoryId));

        Task completedTask;
        completedTask.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        completedTask.title = QStringLiteral("Completed task");
        completedTask.createdAt = QDateTime::currentDateTime().addDays(-1);
        completedTask.completedAt = QDateTime::currentDateTime();
        completedTask.completed = true;
        QVERIFY2(sourceRepository.addTask(completedTask, &error), qPrintable(error));

        QVERIFY(sourceGoals.addGoal(
            QStringLiteral("Move Daymark to another computer"),
            QStringLiteral("Prove the data is portable."),
            QStringLiteral("2030-02-01")));
        QVERIFY(sourceGoals.addMilestone(
            0,
            QStringLiteral("Create an export"),
            QStringLiteral("2030-01-15")));
        QVERIFY(sourceGoals.setMilestoneCompleted(0, 0, true));

        sourceSettings.setInterfaceStyle(AppSettings::QuietFocus);
        sourceSettings.setColorMode(AppSettings::Light);
        sourceSettings.setAccentPreset(AppSettings::Green);
        sourceSettings.setDailyCapacityMinutes(360);
        sourceSettings.setDefaultEstimatedMinutes(50);
        sourceSettings.setDefaultImportance(4);
        sourceSettings.setUse24HourClock(true);
        sourceSettings.setShowPriorityReasons(false);
        sourceSettings.setConfirmTaskCompletion(true);

        const QString exportBase = directory.filePath(QStringLiteral("portable-data"));
        QVERIFY(sourceTransfer.exportData(QUrl::fromLocalFile(exportBase)));
        const QString exportPath = exportBase + QStringLiteral(".daymark.json");
        QVERIFY(QFile::exists(exportPath));

        QCoreApplication::setApplicationName(QStringLiteral("Target"));
        TaskRepository targetRepository(QStringLiteral(":memory:"));
        QVERIFY2(targetRepository.open(&error), qPrintable(error));
        TaskListModel targetTasks(targetRepository);
        CategoryListModel targetCategories(targetRepository);
        GoalListModel targetGoals(targetRepository);
        AppSettings targetSettings(directory.filePath(QStringLiteral("target-data")));
        DataTransferService targetTransfer(
            targetRepository,
            targetTasks,
            targetCategories,
            targetGoals,
            targetSettings);

        QVERIFY(targetTasks.addTask(
            QStringLiteral("Existing target task"), {}, 2, 20, QStringLiteral("Local")));
        QVERIFY(targetTransfer.importData(QUrl::fromLocalFile(exportPath)));

        const QVector<Task> importedTasks = targetRepository.allTasks(&error);
        QCOMPARE(importedTasks.size(), 3);
        QCOMPARE(targetTasks.activeCount(), 2);
        QCOMPARE(targetCategories.categoryCount(), 1);
        QCOMPARE(
            targetCategories.data(
                targetCategories.index(0),
                CategoryListModel::NotesRole).toString(),
            QStringLiteral("These notes must move with the category."));
        const auto assignedTask = std::find_if(
            importedTasks.cbegin(),
            importedTasks.cend(),
            [](const Task &task) { return task.title == QStringLiteral("Open task"); });
        QVERIFY(assignedTask != importedTasks.cend());
        QCOMPARE(assignedTask->categoryId, sourceCategoryId);
        QCOMPARE(assignedTask->categoryName, QStringLiteral("Portable category"));
        QCOMPARE(targetGoals.goalCount(), 1);
        QCOMPARE(targetGoals.totalMilestoneCount(), 1);
        QCOMPARE(targetGoals.completedMilestoneCount(), 1);
        QCOMPARE(targetSettings.interfaceStyle(), AppSettings::QuietFocus);
        QCOMPARE(targetSettings.colorMode(), AppSettings::Light);
        QCOMPARE(targetSettings.accentPreset(), AppSettings::Green);
        QCOMPARE(targetSettings.dailyCapacityMinutes(), 360);
        QCOMPARE(targetSettings.defaultEstimatedMinutes(), 50);
        QCOMPARE(targetSettings.defaultImportance(), 4);
        QVERIFY(targetSettings.use24HourClock());
        QVERIFY(!targetSettings.showPriorityReasons());
        QVERIFY(targetSettings.confirmTaskCompletion());
        QVERIFY(targetTransfer.statusMessage().contains(QStringLiteral("Existing records were kept")));
    }

    void rejectsMalformedDataWithoutChangingTheDatabase()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString malformedPath = directory.filePath(QStringLiteral("broken.daymark.json"));
        QFile malformedFile(malformedPath);
        QVERIFY(malformedFile.open(QIODevice::WriteOnly));
        QCOMPARE(malformedFile.write("{ not valid json"), 16);
        malformedFile.close();

        QCoreApplication::setOrganizationName(QStringLiteral("DaymarkTransferTest"));
        QCoreApplication::setApplicationName(QStringLiteral("MalformedTarget"));
        TaskRepository repository(QStringLiteral(":memory:"));
        QString error;
        QVERIFY2(repository.open(&error), qPrintable(error));
        TaskListModel tasks(repository);
        CategoryListModel categories(repository);
        GoalListModel goals(repository);
        AppSettings settings(directory.filePath(QStringLiteral("data")));
        DataTransferService transfer(repository, tasks, categories, goals, settings);
        QVERIFY(tasks.addTask(QStringLiteral("Keep me"), {}, 3, 30, {}));

        QVERIFY(!transfer.importData(QUrl::fromLocalFile(malformedPath)));
        QCOMPARE(repository.allTasks(&error).size(), 1);
        QCOMPARE(tasks.activeCount(), 1);
        QVERIFY(transfer.statusMessage().contains(QStringLiteral("not valid JSON")));
    }
};

QTEST_GUILESS_MAIN(DataTransferTest)

#include "datatransfer_test.moc"
