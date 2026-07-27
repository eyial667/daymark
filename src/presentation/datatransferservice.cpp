// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/datatransferservice.h"

#include "data/taskrepository.h"
#include "presentation/appsettings.h"
#include "presentation/goallistmodel.h"
#include "presentation/tasklistmodel.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>

namespace {

constexpr auto BackupFormat = "org.daymark.backup";
constexpr int BackupVersion = 1;
constexpr qint64 MaximumBackupBytes = 50 * 1024 * 1024;
constexpr qsizetype MaximumRecords = 100000;

struct ImportedSettings
{
    int interfaceStyle = 3;
    int colorMode = 0;
    int accentPreset = 0;
    int dailyCapacityMinutes = 480;
    int defaultEstimatedMinutes = 30;
    int defaultImportance = 3;
    bool use24HourClock = false;
    bool showPriorityReasons = true;
    bool confirmTaskCompletion = false;
};

QString serializedDateTime(const QDateTime &value)
{
    return value.isValid() ? value.toUTC().toString(Qt::ISODateWithMs) : QString();
}

QString serializedDate(const QDate &value)
{
    return value.isValid() ? value.toString(Qt::ISODate) : QString();
}

QJsonObject taskToJson(const Task &task)
{
    return {
        {QStringLiteral("id"), task.id},
        {QStringLiteral("title"), task.title},
        {QStringLiteral("notes"), task.notes},
        {QStringLiteral("project"), task.project},
        {QStringLiteral("dueAt"), serializedDateTime(task.dueAt)},
        {QStringLiteral("createdAt"), serializedDateTime(task.createdAt)},
        {QStringLiteral("completedAt"), serializedDateTime(task.completedAt)},
        {QStringLiteral("importance"), task.importance},
        {QStringLiteral("estimatedMinutes"), task.estimatedMinutes},
        {QStringLiteral("completed"), task.completed},
    };
}

QJsonObject milestoneToJson(const Milestone &milestone)
{
    return {
        {QStringLiteral("id"), milestone.id},
        {QStringLiteral("title"), milestone.title},
        {QStringLiteral("targetDate"), serializedDate(milestone.targetDate)},
        {QStringLiteral("createdAt"), serializedDateTime(milestone.createdAt)},
        {QStringLiteral("completed"), milestone.completed},
    };
}

QJsonObject goalToJson(const Goal &goal)
{
    QJsonArray milestones;
    for (const Milestone &milestone : goal.milestones) {
        milestones.append(milestoneToJson(milestone));
    }

    return {
        {QStringLiteral("id"), goal.id},
        {QStringLiteral("title"), goal.title},
        {QStringLiteral("description"), goal.description},
        {QStringLiteral("targetDate"), serializedDate(goal.targetDate)},
        {QStringLiteral("createdAt"), serializedDateTime(goal.createdAt)},
        {QStringLiteral("completed"), goal.completed},
        {QStringLiteral("milestones"), milestones},
    };
}

QJsonObject settingsToJson(const AppSettings &settings)
{
    return {
        {QStringLiteral("interfaceStyle"), static_cast<int>(settings.interfaceStyle())},
        {QStringLiteral("colorMode"), static_cast<int>(settings.colorMode())},
        {QStringLiteral("accentPreset"), static_cast<int>(settings.accentPreset())},
        {QStringLiteral("dailyCapacityMinutes"), settings.dailyCapacityMinutes()},
        {QStringLiteral("defaultEstimatedMinutes"), settings.defaultEstimatedMinutes()},
        {QStringLiteral("defaultImportance"), settings.defaultImportance()},
        {QStringLiteral("use24HourClock"), settings.use24HourClock()},
        {QStringLiteral("showPriorityReasons"), settings.showPriorityReasons()},
        {QStringLiteral("confirmTaskCompletion"), settings.confirmTaskCompletion()},
    };
}

bool readString(
    const QJsonObject &object,
    const QString &key,
    QString *target,
    QString *errorMessage,
    bool allowEmpty = true,
    qsizetype maximumLength = 10000)
{
    const QJsonValue value = object.value(key);
    if (!value.isString()) {
        *errorMessage = QStringLiteral("'%1' must be text.").arg(key);
        return false;
    }

    const QString text = value.toString();
    if ((!allowEmpty && text.trimmed().isEmpty()) || text.size() > maximumLength) {
        *errorMessage = QStringLiteral("'%1' is empty or too long.").arg(key);
        return false;
    }
    *target = text;
    return true;
}

bool readBool(
    const QJsonObject &object,
    const QString &key,
    bool *target,
    QString *errorMessage)
{
    const QJsonValue value = object.value(key);
    if (!value.isBool()) {
        *errorMessage = QStringLiteral("'%1' must be true or false.").arg(key);
        return false;
    }
    *target = value.toBool();
    return true;
}

bool readBoundedInt(
    const QJsonObject &object,
    const QString &key,
    int minimum,
    int maximum,
    int *target,
    QString *errorMessage)
{
    const QJsonValue value = object.value(key);
    if (!value.isDouble()) {
        *errorMessage = QStringLiteral("'%1' must be a number.").arg(key);
        return false;
    }

    const double number = value.toDouble();
    const int integer = value.toInt(minimum - 1);
    if (number != static_cast<double>(integer) || integer < minimum || integer > maximum) {
        *errorMessage = QStringLiteral("'%1' is outside the supported range.").arg(key);
        return false;
    }
    *target = integer;
    return true;
}

bool readDateTime(
    const QJsonObject &object,
    const QString &key,
    QDateTime *target,
    QString *errorMessage,
    bool required)
{
    QString serialized;
    if (!readString(object, key, &serialized, errorMessage)) {
        return false;
    }
    if (serialized.isEmpty()) {
        if (required) {
            *errorMessage = QStringLiteral("'%1' must contain a date and time.").arg(key);
            return false;
        }
        *target = {};
        return true;
    }

    const QDateTime parsed = QDateTime::fromString(serialized, Qt::ISODateWithMs);
    if (!parsed.isValid()) {
        *errorMessage = QStringLiteral("'%1' contains an invalid date and time.").arg(key);
        return false;
    }
    *target = parsed.toLocalTime();
    return true;
}

bool readDate(
    const QJsonObject &object,
    const QString &key,
    QDate *target,
    QString *errorMessage)
{
    QString serialized;
    if (!readString(object, key, &serialized, errorMessage)) {
        return false;
    }
    if (serialized.isEmpty()) {
        *target = {};
        return true;
    }

    const QDate parsed = QDate::fromString(serialized, Qt::ISODate);
    if (!parsed.isValid()) {
        *errorMessage = QStringLiteral("'%1' contains an invalid date.").arg(key);
        return false;
    }
    *target = parsed;
    return true;
}

bool taskFromJson(
    const QJsonValue &value,
    Task *task,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("Every task must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    return readString(object, QStringLiteral("id"), &task->id, errorMessage, false, 128)
        && readString(object, QStringLiteral("title"), &task->title, errorMessage, false)
        && readString(object, QStringLiteral("notes"), &task->notes, errorMessage)
        && readString(object, QStringLiteral("project"), &task->project, errorMessage)
        && readDateTime(object, QStringLiteral("dueAt"), &task->dueAt, errorMessage, false)
        && readDateTime(object, QStringLiteral("createdAt"), &task->createdAt, errorMessage, true)
        && readDateTime(object, QStringLiteral("completedAt"), &task->completedAt, errorMessage, false)
        && readBoundedInt(object, QStringLiteral("importance"), 1, 5, &task->importance, errorMessage)
        && readBoundedInt(
            object,
            QStringLiteral("estimatedMinutes"),
            5,
            480,
            &task->estimatedMinutes,
            errorMessage)
        && readBool(object, QStringLiteral("completed"), &task->completed, errorMessage);
}

bool milestoneFromJson(
    const QJsonValue &value,
    const QString &goalId,
    Milestone *milestone,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("Every milestone must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    milestone->goalId = goalId;
    return readString(object, QStringLiteral("id"), &milestone->id, errorMessage, false, 128)
        && readString(object, QStringLiteral("title"), &milestone->title, errorMessage, false)
        && readDate(object, QStringLiteral("targetDate"), &milestone->targetDate, errorMessage)
        && readDateTime(
            object,
            QStringLiteral("createdAt"),
            &milestone->createdAt,
            errorMessage,
            true)
        && readBool(object, QStringLiteral("completed"), &milestone->completed, errorMessage);
}

bool goalFromJson(
    const QJsonValue &value,
    Goal *goal,
    QSet<QString> *milestoneIds,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("Every goal must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    if (!readString(object, QStringLiteral("id"), &goal->id, errorMessage, false, 128)
        || !readString(object, QStringLiteral("title"), &goal->title, errorMessage, false)
        || !readString(object, QStringLiteral("description"), &goal->description, errorMessage)
        || !readDate(object, QStringLiteral("targetDate"), &goal->targetDate, errorMessage)
        || !readDateTime(
            object,
            QStringLiteral("createdAt"),
            &goal->createdAt,
            errorMessage,
            true)
        || !readBool(object, QStringLiteral("completed"), &goal->completed, errorMessage)) {
        return false;
    }

    const QJsonValue milestonesValue = object.value(QStringLiteral("milestones"));
    if (!milestonesValue.isArray()) {
        *errorMessage = QStringLiteral("'milestones' must be a list.");
        return false;
    }
    const QJsonArray milestones = milestonesValue.toArray();
    if (milestones.size() > MaximumRecords) {
        *errorMessage = QStringLiteral("The backup contains too many milestones.");
        return false;
    }

    for (const QJsonValue &milestoneValue : milestones) {
        Milestone milestone;
        if (!milestoneFromJson(milestoneValue, goal->id, &milestone, errorMessage)) {
            return false;
        }
        if (milestoneIds->contains(milestone.id)) {
            *errorMessage = QStringLiteral("The backup contains a duplicate milestone ID.");
            return false;
        }
        milestoneIds->insert(milestone.id);
        goal->milestones.append(milestone);
    }
    return true;
}

bool settingsFromJson(
    const QJsonValue &value,
    ImportedSettings *settings,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("'settings' must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    return readBoundedInt(
               object,
               QStringLiteral("interfaceStyle"),
               0,
               3,
               &settings->interfaceStyle,
               errorMessage)
        && readBoundedInt(
            object,
            QStringLiteral("colorMode"),
            0,
            1,
            &settings->colorMode,
            errorMessage)
        && readBoundedInt(
            object,
            QStringLiteral("accentPreset"),
            0,
            4,
            &settings->accentPreset,
            errorMessage)
        && readBoundedInt(
            object,
            QStringLiteral("dailyCapacityMinutes"),
            60,
            960,
            &settings->dailyCapacityMinutes,
            errorMessage)
        && readBoundedInt(
            object,
            QStringLiteral("defaultEstimatedMinutes"),
            5,
            480,
            &settings->defaultEstimatedMinutes,
            errorMessage)
        && readBoundedInt(
            object,
            QStringLiteral("defaultImportance"),
            1,
            5,
            &settings->defaultImportance,
            errorMessage)
        && readBool(
            object,
            QStringLiteral("use24HourClock"),
            &settings->use24HourClock,
            errorMessage)
        && readBool(
            object,
            QStringLiteral("showPriorityReasons"),
            &settings->showPriorityReasons,
            errorMessage)
        && readBool(
            object,
            QStringLiteral("confirmTaskCompletion"),
            &settings->confirmTaskCompletion,
            errorMessage);
}

void applySettings(const ImportedSettings &source, AppSettings &target)
{
    target.setInterfaceStyle(static_cast<AppSettings::InterfaceStyle>(source.interfaceStyle));
    target.setColorMode(static_cast<AppSettings::ColorMode>(source.colorMode));
    target.setAccentPreset(static_cast<AppSettings::AccentPreset>(source.accentPreset));
    target.setDailyCapacityMinutes(source.dailyCapacityMinutes);
    target.setDefaultEstimatedMinutes(source.defaultEstimatedMinutes);
    target.setDefaultImportance(source.defaultImportance);
    target.setUse24HourClock(source.use24HourClock);
    target.setShowPriorityReasons(source.showPriorityReasons);
    target.setConfirmTaskCompletion(source.confirmTaskCompletion);
}

} // namespace

DataTransferService::DataTransferService(
    TaskRepository &repository,
    TaskListModel &taskModel,
    GoalListModel &goalModel,
    AppSettings &appSettings,
    QObject *parent)
    : QObject(parent)
    , m_repository(repository)
    , m_taskModel(taskModel)
    , m_goalModel(goalModel)
    , m_appSettings(appSettings)
{
}

QString DataTransferService::statusMessage() const
{
    return m_statusMessage;
}

bool DataTransferService::exportData(const QUrl &destination)
{
    if (!destination.isLocalFile()) {
        setStatusMessage(QStringLiteral("Choose a local file for the export."));
        return false;
    }

    QString errorMessage;
    const QVector<Task> tasks = m_repository.allTasks(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not read tasks: %1").arg(errorMessage));
        return false;
    }
    const QVector<Goal> goals = m_repository.goals(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not read goals: %1").arg(errorMessage));
        return false;
    }

    QJsonArray taskArray;
    for (const Task &task : tasks) {
        taskArray.append(taskToJson(task));
    }
    QJsonArray goalArray;
    for (const Goal &goal : goals) {
        goalArray.append(goalToJson(goal));
    }

    const QJsonObject root {
        {QStringLiteral("format"), QString::fromLatin1(BackupFormat)},
        {QStringLiteral("version"), BackupVersion},
        {QStringLiteral("exportedAt"), serializedDateTime(QDateTime::currentDateTime())},
        {QStringLiteral("applicationVersion"), QCoreApplication::applicationVersion()},
        {QStringLiteral("tasks"), taskArray},
        {QStringLiteral("goals"), goalArray},
        {QStringLiteral("settings"), settingsToJson(m_appSettings)},
    };

    QString path = destination.toLocalFile();
    if (!path.endsWith(QStringLiteral(".daymark.json"), Qt::CaseInsensitive)) {
        path += QStringLiteral(".daymark.json");
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        setStatusMessage(QStringLiteral("Could not create the export: %1").arg(file.errorString()));
        return false;
    }
    const QByteArray payload = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size() || !file.commit()) {
        setStatusMessage(QStringLiteral("Could not finish the export: %1").arg(file.errorString()));
        return false;
    }

    setStatusMessage(QStringLiteral("Exported tasks, goals, and preferences to %1.")
        .arg(QFileInfo(path).fileName()));
    return true;
}

bool DataTransferService::importData(const QUrl &source)
{
    if (!source.isLocalFile()) {
        setStatusMessage(QStringLiteral("Choose a local Daymark data file."));
        return false;
    }

    QFile file(source.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        setStatusMessage(QStringLiteral("Could not open the import: %1").arg(file.errorString()));
        return false;
    }
    if (file.size() > MaximumBackupBytes) {
        setStatusMessage(QStringLiteral("The selected file is too large to be a Daymark export."));
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setStatusMessage(QStringLiteral("The selected file is not valid JSON: %1")
            .arg(parseError.errorString()));
        return false;
    }

    const QJsonObject root = document.object();
    if (root.value(QStringLiteral("format")).toString() != QString::fromLatin1(BackupFormat)) {
        setStatusMessage(QStringLiteral("The selected file is not a Daymark export."));
        return false;
    }
    if (!root.value(QStringLiteral("version")).isDouble()
        || root.value(QStringLiteral("version")).toDouble() != BackupVersion) {
        setStatusMessage(QStringLiteral("This Daymark export version is not supported."));
        return false;
    }

    const QJsonValue tasksValue = root.value(QStringLiteral("tasks"));
    const QJsonValue goalsValue = root.value(QStringLiteral("goals"));
    if (!tasksValue.isArray() || !goalsValue.isArray()) {
        setStatusMessage(QStringLiteral("The export is missing its task or goal list."));
        return false;
    }

    const QJsonArray taskArray = tasksValue.toArray();
    const QJsonArray goalArray = goalsValue.toArray();
    if (taskArray.size() > MaximumRecords || goalArray.size() > MaximumRecords) {
        setStatusMessage(QStringLiteral("The export contains too many records."));
        return false;
    }

    QVector<Task> tasks;
    QVector<Goal> goals;
    QSet<QString> taskIds;
    QSet<QString> goalIds;
    QSet<QString> milestoneIds;
    QString validationError;

    for (const QJsonValue &value : taskArray) {
        Task task;
        if (!taskFromJson(value, &task, &validationError)) {
            setStatusMessage(QStringLiteral("Could not import a task: %1").arg(validationError));
            return false;
        }
        if (taskIds.contains(task.id)) {
            setStatusMessage(QStringLiteral("The export contains a duplicate task ID."));
            return false;
        }
        taskIds.insert(task.id);
        tasks.append(task);
    }

    for (const QJsonValue &value : goalArray) {
        Goal goal;
        if (!goalFromJson(value, &goal, &milestoneIds, &validationError)) {
            setStatusMessage(QStringLiteral("Could not import a goal: %1").arg(validationError));
            return false;
        }
        if (goalIds.contains(goal.id)) {
            setStatusMessage(QStringLiteral("The export contains a duplicate goal ID."));
            return false;
        }
        goalIds.insert(goal.id);
        goals.append(goal);
    }

    ImportedSettings importedSettings;
    if (!settingsFromJson(
            root.value(QStringLiteral("settings")),
            &importedSettings,
            &validationError)) {
        setStatusMessage(QStringLiteral("Could not import preferences: %1").arg(validationError));
        return false;
    }

    QString databaseError;
    if (!m_repository.mergeImportedData(tasks, goals, &databaseError)) {
        setStatusMessage(QStringLiteral("Could not merge the imported data: %1").arg(databaseError));
        return false;
    }

    applySettings(importedSettings, m_appSettings);
    m_taskModel.reload();
    m_goalModel.reload();
    setStatusMessage(QStringLiteral(
        "Imported %1 tasks and %2 goals. Existing records were kept.")
        .arg(tasks.size())
        .arg(goals.size()));
    return true;
}

void DataTransferService::clearStatus()
{
    setStatusMessage({});
}

void DataTransferService::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}
