// SPDX-License-Identifier: GPL-3.0-or-later

#include "data/taskrepository.h"
#include "presentation/appsettings.h"
#include "presentation/categorylistmodel.h"
#include "presentation/dailynotemodel.h"
#include "presentation/datatransferservice.h"
#include "presentation/focussessionmodel.h"
#include "presentation/goallistmodel.h"
#include "presentation/mentalmapmodel.h"
#include "presentation/tasklistmodel.h"
#include "presentation/updateservice.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QTimer>
#include <QVariant>

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QGuiApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Daymark"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("daymark.org"));
    QCoreApplication::setApplicationName(QStringLiteral("Daymark"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(DAYMARK_VERSION));
    application.setDesktopFileName(QStringLiteral("org.daymark.dashboard"));
    application.setWindowIcon(
        QIcon(QStringLiteral(":/assets/icons/org.daymark.dashboard.svg")));

    const QString dataDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    TaskRepository repository(dataDirectory + QStringLiteral("/daymark.sqlite3"));

    QString databaseError;
    if (!repository.open(&databaseError)) {
        qCritical("Unable to open the Daymark database: %s", qPrintable(databaseError));
        return EXIT_FAILURE;
    }

    TaskListModel taskModel(repository);
    TaskListModel todayTaskModel(repository, TaskListModel::Today);
    CategoryListModel categoryModel(repository);
    GoalListModel goalModel(repository);
    MentalMapModel mentalMapModel(repository);
    DailyNoteModel dailyNoteModel(repository);
    FocusSessionModel focusSession;
    AppSettings appSettings(dataDirectory);
    DataTransferService dataTransfer(
        repository,
        taskModel,
        categoryModel,
        goalModel,
        appSettings);
    UpdateService updateService;

    QObject::connect(
        &updateService,
        &UpdateService::quitRequested,
        &application,
        &QCoreApplication::quit);

    QObject::connect(
        &taskModel,
        &QAbstractItemModel::modelReset,
        &todayTaskModel,
        &TaskListModel::reload);
    QObject::connect(
        &todayTaskModel,
        &TaskListModel::tasksChanged,
        &taskModel,
        &TaskListModel::reload);
    QObject::connect(
        &taskModel,
        &TaskListModel::tasksChanged,
        &categoryModel,
        &CategoryListModel::reload);
    QObject::connect(
        &todayTaskModel,
        &TaskListModel::tasksChanged,
        &categoryModel,
        &CategoryListModel::reload);
    QObject::connect(
        &goalModel,
        &GoalListModel::taskCreated,
        &taskModel,
        &TaskListModel::reload);

    QObject::connect(
        &categoryModel,
        &CategoryListModel::categoriesChanged,
        &taskModel,
        &TaskListModel::reload);
    QObject::connect(
        &taskModel,
        &QAbstractItemModel::modelReset,
        &mentalMapModel,
        &MentalMapModel::reload);
    QObject::connect(
        &categoryModel,
        &CategoryListModel::categoriesChanged,
        &mentalMapModel,
        &MentalMapModel::reload);
    QObject::connect(
        &goalModel,
        &QAbstractItemModel::modelReset,
        &mentalMapModel,
        &MentalMapModel::reload);

    QTimer dayBoundaryRefresh;
    dayBoundaryRefresh.setInterval(60 * 1000);
    QObject::connect(
        &dayBoundaryRefresh,
        &QTimer::timeout,
        &taskModel,
        &TaskListModel::reload);
    QObject::connect(
        &dayBoundaryRefresh,
        &QTimer::timeout,
        &dailyNoteModel,
        &DailyNoteModel::refreshDay);
    dayBoundaryRefresh.start();

    QObject::connect(
        &application,
        &QCoreApplication::aboutToQuit,
        &dailyNoteModel,
        &DailyNoteModel::saveNow);

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        {QStringLiteral("taskModel"), QVariant::fromValue(&taskModel)},
        {QStringLiteral("todayTaskModel"), QVariant::fromValue(&todayTaskModel)},
        {QStringLiteral("categoryModel"), QVariant::fromValue(&categoryModel)},
        {QStringLiteral("goalModel"), QVariant::fromValue(&goalModel)},
        {QStringLiteral("mentalMapModel"), QVariant::fromValue(&mentalMapModel)},
        {QStringLiteral("dailyNoteModel"), QVariant::fromValue(&dailyNoteModel)},
        {QStringLiteral("focusSession"), QVariant::fromValue(&focusSession)},
        {QStringLiteral("appSettings"), QVariant::fromValue(&appSettings)},
        {QStringLiteral("dataTransfer"), QVariant::fromValue(&dataTransfer)},
        {QStringLiteral("updateService"), QVariant::fromValue(&updateService)},
    });
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &application,
        [] { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("Daymark"), QStringLiteral("Main"));

    QTimer::singleShot(1200, &updateService, [&updateService] {
        updateService.checkForUpdates();
    });

    return application.exec();
}
