// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/datatransferservice.h"

#include "data/taskrepository.h"
#include "presentation/appsettings.h"
#include "presentation/categorylistmodel.h"
#include "presentation/goallistmodel.h"
#include "presentation/tasklistmodel.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>
#include <QUuid>

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
        {QStringLiteral("project"), QStringLiteral("")},
        {QStringLiteral("categoryId"), task.categoryId},
        {QStringLiteral("subcategoryId"), task.subcategoryId},
        {QStringLiteral("dueAt"), serializedDateTime(task.dueAt)},
        {QStringLiteral("plannedDate"), serializedDate(task.plannedDate)},
        {QStringLiteral("createdAt"), serializedDateTime(task.createdAt)},
        {QStringLiteral("completedAt"), serializedDateTime(task.completedAt)},
        {QStringLiteral("importance"), task.importance},
        {QStringLiteral("estimatedMinutes"), task.estimatedMinutes},
        {QStringLiteral("completed"), task.completed},
    };
}

QJsonObject subcategoryToJson(const Subcategory &subcategory)
{
    return {
        {QStringLiteral("id"), subcategory.id},
        {QStringLiteral("name"), subcategory.name},
        {QStringLiteral("notes"), subcategory.notes},
        {QStringLiteral("createdAt"), serializedDateTime(subcategory.createdAt)},
    };
}

QJsonObject categoryToJson(const Category &category)
{
    QJsonArray subcategories;
    for (const Subcategory &subcategory : category.subcategories) {
        subcategories.append(subcategoryToJson(subcategory));
    }
    return {
        {QStringLiteral("id"), category.id},
        {QStringLiteral("name"), category.name},
        {QStringLiteral("notes"), category.notes},
        {QStringLiteral("createdAt"), serializedDateTime(category.createdAt)},
        {QStringLiteral("subcategories"), subcategories},
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

bool readOptionalString(
    const QJsonObject &object,
    const QString &key,
    QString *target,
    QString *errorMessage,
    qsizetype maximumLength = 10000)
{
    if (!object.contains(key)) {
        target->clear();
        return true;
    }
    return readString(object, key, target, errorMessage, true, maximumLength);
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

bool readOptionalDate(
    const QJsonObject &object,
    const QString &key,
    QDate *target,
    QString *errorMessage)
{
    if (!object.contains(key)) {
        *target = {};
        return true;
    }
    return readDate(object, key, target, errorMessage);
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
        && readOptionalString(
            object,
            QStringLiteral("categoryId"),
            &task->categoryId,
            errorMessage,
            128)
        && readOptionalString(
            object,
            QStringLiteral("subcategoryId"),
            &task->subcategoryId,
            errorMessage,
            128)
        && readDateTime(object, QStringLiteral("dueAt"), &task->dueAt, errorMessage, false)
        && readOptionalDate(
            object,
            QStringLiteral("plannedDate"),
            &task->plannedDate,
            errorMessage)
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

bool subcategoryFromJson(
    const QJsonValue &value,
    const QString &categoryId,
    Subcategory *subcategory,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("Every subcategory must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    subcategory->categoryId = categoryId;
    if (!(readString(object, QStringLiteral("id"), &subcategory->id, errorMessage, false, 128)
        && readString(
            object,
            QStringLiteral("name"),
            &subcategory->name,
            errorMessage,
            false,
            120)
        && readString(object, QStringLiteral("notes"), &subcategory->notes, errorMessage)
        && readDateTime(
            object,
            QStringLiteral("createdAt"),
            &subcategory->createdAt,
            errorMessage,
            true))) {
        return false;
    }
    subcategory->name = subcategory->name.trimmed();
    subcategory->notes = subcategory->notes.trimmed();
    return true;
}

bool categoryFromJson(
    const QJsonValue &value,
    Category *category,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("Every category must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    if (!(readString(object, QStringLiteral("id"), &category->id, errorMessage, false, 128)
        && readString(
            object,
            QStringLiteral("name"),
            &category->name,
            errorMessage,
            false,
            120)
        && readString(object, QStringLiteral("notes"), &category->notes, errorMessage)
        && readDateTime(
            object,
            QStringLiteral("createdAt"),
            &category->createdAt,
            errorMessage,
            true))) {
        return false;
    }
    category->name = category->name.trimmed();
    category->notes = category->notes.trimmed();

    const QJsonValue subcategoriesValue = object.value(QStringLiteral("subcategories"));
    if (subcategoriesValue.isUndefined()) {
        return true;
    }
    if (!subcategoriesValue.isArray()) {
        *errorMessage = QStringLiteral("'subcategories' must be a list.");
        return false;
    }
    const QJsonArray subcategories = subcategoriesValue.toArray();
    if (subcategories.size() > MaximumRecords) {
        *errorMessage = QStringLiteral("The backup contains too many subcategories.");
        return false;
    }
    QSet<QString> names;
    for (const QJsonValue &subcategoryValue : subcategories) {
        Subcategory subcategory;
        if (!subcategoryFromJson(
                subcategoryValue,
                category->id,
                &subcategory,
                errorMessage)) {
            return false;
        }
        const QString normalizedName = subcategory.name.toCaseFolded();
        if (names.contains(normalizedName)) {
            *errorMessage = QStringLiteral("A category contains duplicate subcategory names.");
            return false;
        }
        names.insert(normalizedName);
        category->subcategories.append(subcategory);
    }
    return true;
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
    CategoryListModel &categoryModel,
    GoalListModel &goalModel,
    AppSettings &appSettings,
    QObject *parent)
    : QObject(parent)
    , m_repository(repository)
    , m_taskModel(taskModel)
    , m_categoryModel(categoryModel)
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
    const QVector<Category> categories = m_repository.categories(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not read categories: %1").arg(errorMessage));
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
    QJsonArray categoryArray;
    for (const Category &category : categories) {
        categoryArray.append(categoryToJson(category));
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
        {QStringLiteral("categories"), categoryArray},
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

    setStatusMessage(QStringLiteral(
        "Exported tasks, categories, subcategories, goals, and preferences to %1.")
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
    const QJsonValue categoriesValue = root.value(QStringLiteral("categories"));
    const QJsonValue goalsValue = root.value(QStringLiteral("goals"));
    if (!tasksValue.isArray() || !goalsValue.isArray()
        || (!categoriesValue.isUndefined() && !categoriesValue.isArray())) {
        setStatusMessage(QStringLiteral("The export is missing its task or goal list."));
        return false;
    }

    const QJsonArray taskArray = tasksValue.toArray();
    const QJsonArray categoryArray = categoriesValue.isArray()
        ? categoriesValue.toArray()
        : QJsonArray();
    const QJsonArray goalArray = goalsValue.toArray();
    if (taskArray.size() > MaximumRecords || categoryArray.size() > MaximumRecords
        || goalArray.size() > MaximumRecords) {
        setStatusMessage(QStringLiteral("The export contains too many records."));
        return false;
    }

    QVector<Task> tasks;
    QVector<Category> categories;
    QVector<Goal> goals;
    QSet<QString> taskIds;
    QSet<QString> categoryIds;
    QSet<QString> categoryNames;
    QHash<QString, QString> categoryIdsByName;
    QHash<QString, QString> existingCategoryIdsByName;
    QHash<QString, QString> subcategoryParents;
    QSet<QString> goalIds;
    QSet<QString> milestoneIds;
    qsizetype subcategoryCount = 0;
    QString validationError;

    QString existingDataError;
    const QVector<Category> existingCategories = m_repository.categories(&existingDataError);
    if (!existingDataError.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not inspect existing categories: %1")
            .arg(existingDataError));
        return false;
    }
    for (const Category &category : existingCategories) {
        existingCategoryIdsByName.insert(category.name.trimmed().toCaseFolded(), category.id);
    }

    for (const QJsonValue &value : categoryArray) {
        Category category;
        if (!categoryFromJson(value, &category, &validationError)) {
            setStatusMessage(QStringLiteral("Could not import a category: %1").arg(validationError));
            return false;
        }
        const QString normalizedName = category.name.trimmed().toCaseFolded();
        if (categoryIds.contains(category.id) || categoryNames.contains(normalizedName)) {
            setStatusMessage(QStringLiteral("The export contains a duplicate category."));
            return false;
        }
        categoryIds.insert(category.id);
        categoryNames.insert(normalizedName);
        categoryIdsByName.insert(normalizedName, category.id);
        for (const Subcategory &subcategory : category.subcategories) {
            if (subcategoryParents.contains(subcategory.id)) {
                setStatusMessage(QStringLiteral(
                    "The export contains a duplicate subcategory ID."));
                return false;
            }
            subcategoryParents.insert(subcategory.id, category.id);
            ++subcategoryCount;
            if (subcategoryCount > MaximumRecords) {
                setStatusMessage(QStringLiteral("The export contains too many records."));
                return false;
            }
        }
        categories.append(category);
    }

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
        const QString legacyProjectName = task.project.trimmed();
        if (task.categoryId.isEmpty() && !legacyProjectName.isEmpty()) {
            const QString normalizedProjectName = legacyProjectName.toCaseFolded();
            if (categoryIdsByName.contains(normalizedProjectName)) {
                task.categoryId = categoryIdsByName.value(normalizedProjectName);
            } else if (existingCategoryIdsByName.contains(normalizedProjectName)) {
                task.categoryId = existingCategoryIdsByName.value(normalizedProjectName);
                categoryIds.insert(task.categoryId);
            } else {
                Category migratedCategory;
                migratedCategory.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                migratedCategory.name = legacyProjectName;
                migratedCategory.notes = QStringLiteral(
                    "Migrated from the former Projects field during import.");
                migratedCategory.createdAt = task.createdAt;
                categories.append(migratedCategory);
                categoryIds.insert(migratedCategory.id);
                categoryNames.insert(normalizedProjectName);
                categoryIdsByName.insert(normalizedProjectName, migratedCategory.id);
                task.categoryId = migratedCategory.id;
            }
        }
        task.project.clear();
        if (!task.categoryId.isEmpty() && !categoryIds.contains(task.categoryId)) {
            setStatusMessage(QStringLiteral(
                "A task refers to a category that is missing from the export."));
            return false;
        }
        if (!task.subcategoryId.isEmpty()
            && (!subcategoryParents.contains(task.subcategoryId)
                || subcategoryParents.value(task.subcategoryId) != task.categoryId)) {
            setStatusMessage(QStringLiteral(
                "A task refers to a subcategory outside its category."));
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
    if (!m_repository.mergeImportedData(categories, tasks, goals, &databaseError)) {
        setStatusMessage(QStringLiteral("Could not merge the imported data: %1").arg(databaseError));
        return false;
    }

    applySettings(importedSettings, m_appSettings);
    m_categoryModel.reload();
    m_taskModel.reload();
    m_goalModel.reload();
    setStatusMessage(QStringLiteral(
        "Imported %1 tasks, %2 categories, %3 subcategories, and %4 goals. "
        "Existing records were kept.")
        .arg(tasks.size())
        .arg(categories.size())
        .arg(subcategoryCount)
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
