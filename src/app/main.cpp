// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/desktopnotificationservice.h"
#include "data/taskrepository.h"
#include "presentation/appsettings.h"
#include "presentation/categorylistmodel.h"
#include "presentation/dailynotemodel.h"
#include "presentation/datatransferservice.h"
#include "presentation/focussessionmodel.h"
#include "presentation/goallistmodel.h"
#include "presentation/languagemanager.h"
#include "presentation/mentalmapmodel.h"
#include "presentation/meetinglistmodel.h"
#include "presentation/meetingreminderservice.h"
#include "presentation/tasklistmodel.h"
#include "presentation/updateservice.h"

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QTimer>
#include <QVariant>

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Daymark"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("daymark.org"));
    QCoreApplication::setApplicationName(QStringLiteral("Daymark"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(DAYMARK_VERSION));
    application.setDesktopFileName(QStringLiteral("org.daymark.dashboard"));
    const QIcon applicationIcon(
        QStringLiteral(":/assets/icons/org.daymark.dashboard.svg"));
    application.setWindowIcon(applicationIcon);

    const QString dataDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    TaskRepository repository(dataDirectory + QStringLiteral("/daymark.sqlite3"));

    QString databaseError;
    if (!repository.open(&databaseError)) {
        qCritical("Unable to open the Daymark database: %s", qPrintable(databaseError));
        return EXIT_FAILURE;
    }

    AppSettings appSettings(dataDirectory);
    LanguageManager languageManager;
    if (!languageManager.apply(appSettings.language())) {
        qWarning("Unable to load the selected Daymark translation.");
    }

    TaskListModel taskModel(repository);
    TaskListModel todayTaskModel(repository, TaskListModel::Today);
    CategoryListModel categoryModel(repository);
    GoalListModel goalModel(repository);
    MeetingListModel meetingModel(repository, appSettings);
    MeetingReminderService meetingReminderService(repository);
    DesktopNotificationService notificationService(applicationIcon);
    MentalMapModel mentalMapModel(repository);
    DailyNoteModel dailyNoteModel(repository);
    FocusSessionModel focusSession;
    DataTransferService dataTransfer(
        repository,
        taskModel,
        categoryModel,
        goalModel,
        meetingModel,
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
        &meetingModel,
        &MeetingListModel::meetingsChanged,
        &meetingReminderService,
        &MeetingReminderService::refresh);
    QObject::connect(
        &meetingReminderService,
        &MeetingReminderService::remindersChanged,
        &meetingModel,
        &MeetingListModel::reload);
    QObject::connect(
        &meetingReminderService,
        &MeetingReminderService::notificationRequested,
        &notificationService,
        &DesktopNotificationService::showNotification);

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
    QObject::connect(
        &dayBoundaryRefresh,
        &QTimer::timeout,
        &meetingModel,
        &MeetingListModel::reload);
    dayBoundaryRefresh.start();

    QObject::connect(
        &application,
        &QCoreApplication::aboutToQuit,
        &dailyNoteModel,
        &DailyNoteModel::saveNow);

    QQmlApplicationEngine engine;
    QObject::connect(
        &appSettings,
        &AppSettings::languageChanged,
        &engine,
        [&] {
            if (!languageManager.apply(appSettings.language())) {
                qWarning("Unable to load the selected Daymark translation.");
            }
            engine.retranslate();
            dataTransfer.clearStatus();
            taskModel.reload();
            todayTaskModel.reload();
            categoryModel.reload();
            goalModel.reload();
            meetingModel.reload();
            mentalMapModel.reload();
            dailyNoteModel.refreshDay();
            updateService.retranslate();
        });
    engine.setInitialProperties({
        {QStringLiteral("taskModel"), QVariant::fromValue(&taskModel)},
        {QStringLiteral("todayTaskModel"), QVariant::fromValue(&todayTaskModel)},
        {QStringLiteral("categoryModel"), QVariant::fromValue(&categoryModel)},
        {QStringLiteral("goalModel"), QVariant::fromValue(&goalModel)},
        {QStringLiteral("meetingModel"), QVariant::fromValue(&meetingModel)},
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

    meetingReminderService.start();

    QTimer::singleShot(1200, &updateService, [&updateService] {
        updateService.checkForUpdates();
    });

    return application.exec();
}
