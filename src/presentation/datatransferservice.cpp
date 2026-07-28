// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/datatransferservice.h"

#include "data/taskrepository.h"
#include "presentation/appsettings.h"
#include "presentation/categorylistmodel.h"
#include "presentation/goallistmodel.h"
#include "presentation/meetinglistmodel.h"
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

#include <cmath>

namespace {

constexpr auto BackupFormat = "org.daymark.backup";
constexpr int BackupVersion = 1;
constexpr qint64 MaximumBackupBytes = 50 * 1024 * 1024;
constexpr qsizetype MaximumRecords = 100000;

struct ImportedSettings
{
    int interfaceStyle = 3;
    int language = 0;
    int colorMode = 0;
    int accentPreset = 0;
    int dailyCapacityMinutes = 480;
    int defaultEstimatedMinutes = 30;
    int defaultImportance = 3;
    bool use24HourClock = false;
    bool showTimeline = false;
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

QJsonObject meetingToJson(const Meeting &meeting)
{
    return {
        {QStringLiteral("id"), meeting.id},
        {QStringLiteral("title"), meeting.title},
        {QStringLiteral("notes"), meeting.notes},
        {QStringLiteral("startsAt"), serializedDateTime(meeting.startsAt)},
        {QStringLiteral("createdAt"), serializedDateTime(meeting.createdAt)},
        {QStringLiteral("notifiedAt"), serializedDateTime(meeting.notifiedAt)},
    };
}

QJsonObject mapGroupToJson(const MentalMapGroup &group)
{
    return {
        {QStringLiteral("id"), group.id},
        {QStringLiteral("kind"), group.kind},
        {QStringLiteral("title"), group.title},
        {QStringLiteral("color"), group.color},
        {QStringLiteral("x"), group.x},
        {QStringLiteral("y"), group.y},
        {QStringLiteral("width"), group.width},
        {QStringLiteral("height"), group.height},
        {QStringLiteral("createdAt"), serializedDateTime(group.createdAt)},
    };
}

QJsonObject mapNoteToJson(const MentalMapNote &note)
{
    QJsonArray checklist;
    for (const MentalMapChecklistItem &item : note.checklist) {
        checklist.append(QJsonObject {
            {QStringLiteral("text"), item.text},
            {QStringLiteral("completed"), item.completed},
        });
    }
    return {
        {QStringLiteral("id"), note.id},
        {QStringLiteral("groupId"), note.groupId},
        {QStringLiteral("title"), note.title},
        {QStringLiteral("body"), note.body},
        {QStringLiteral("color"), note.color},
        {QStringLiteral("tags"), note.tags},
        {QStringLiteral("externalLink"), note.externalLink},
        {QStringLiteral("priority"), note.priority},
        {QStringLiteral("x"), note.x},
        {QStringLiteral("y"), note.y},
        {QStringLiteral("center"), note.center},
        {QStringLiteral("linkedTaskId"), note.linkedTaskId},
        {QStringLiteral("checklist"), checklist},
        {QStringLiteral("createdAt"), serializedDateTime(note.createdAt)},
    };
}

QJsonObject mapConnectionToJson(const MentalMapConnection &connection)
{
    return {
        {QStringLiteral("id"), connection.id},
        {QStringLiteral("viewKind"), connection.viewKind},
        {QStringLiteral("sourceNoteId"), connection.sourceNoteId},
        {QStringLiteral("targetNoteId"), connection.targetNoteId},
        {QStringLiteral("label"), connection.label},
        {QStringLiteral("createdAt"), serializedDateTime(connection.createdAt)},
    };
}

QJsonObject settingsToJson(const AppSettings &settings)
{
    return {
        {QStringLiteral("interfaceStyle"), static_cast<int>(settings.interfaceStyle())},
        {QStringLiteral("language"), static_cast<int>(settings.language())},
        {QStringLiteral("colorMode"), static_cast<int>(settings.colorMode())},
        {QStringLiteral("accentPreset"), static_cast<int>(settings.accentPreset())},
        {QStringLiteral("dailyCapacityMinutes"), settings.dailyCapacityMinutes()},
        {QStringLiteral("defaultEstimatedMinutes"), settings.defaultEstimatedMinutes()},
        {QStringLiteral("defaultImportance"), settings.defaultImportance()},
        {QStringLiteral("use24HourClock"), settings.use24HourClock()},
        {QStringLiteral("showTimeline"), settings.showTimeline()},
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "'%1' must be text.").arg(key);
        return false;
    }

    const QString text = value.toString();
    if ((!allowEmpty && text.trimmed().isEmpty()) || text.size() > maximumLength) {
        *errorMessage = QCoreApplication::translate("DataTransferService", "'%1' is empty or too long.").arg(key);
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "'%1' must be true or false.").arg(key);
        return false;
    }
    *target = value.toBool();
    return true;
}

bool readOptionalBool(
    const QJsonObject &object,
    const QString &key,
    bool *target,
    QString *errorMessage)
{
    if (!object.contains(key)) {
        return true;
    }
    return readBool(object, key, target, errorMessage);
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "'%1' must be a number.").arg(key);
        return false;
    }

    const double number = value.toDouble();
    const int integer = value.toInt(minimum - 1);
    if (number != static_cast<double>(integer) || integer < minimum || integer > maximum) {
        *errorMessage = QCoreApplication::translate("DataTransferService", "'%1' is outside the supported range.").arg(key);
        return false;
    }
    *target = integer;
    return true;
}

bool readOptionalBoundedInt(
    const QJsonObject &object,
    const QString &key,
    int minimum,
    int maximum,
    int *target,
    QString *errorMessage)
{
    if (!object.contains(key)) {
        return true;
    }
    return readBoundedInt(object, key, minimum, maximum, target, errorMessage);
}

bool readBoundedDouble(
    const QJsonObject &object,
    const QString &key,
    double minimum,
    double maximum,
    double *target,
    QString *errorMessage)
{
    const QJsonValue value = object.value(key);
    if (!value.isDouble() || !std::isfinite(value.toDouble())
        || value.toDouble() < minimum || value.toDouble() > maximum) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService", "'%1' is outside the supported range.").arg(key);
        return false;
    }
    *target = value.toDouble();
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
            *errorMessage = QCoreApplication::translate("DataTransferService", "'%1' must contain a date and time.").arg(key);
            return false;
        }
        *target = {};
        return true;
    }

    const QDateTime parsed = QDateTime::fromString(serialized, Qt::ISODateWithMs);
    if (!parsed.isValid()) {
        *errorMessage = QCoreApplication::translate("DataTransferService", "'%1' contains an invalid date and time.").arg(key);
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "'%1' contains an invalid date.").arg(key);
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "Every task must be an object.");
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "Every subcategory must be an object.");
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "Every category must be an object.");
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "'subcategories' must be a list.");
        return false;
    }
    const QJsonArray subcategories = subcategoriesValue.toArray();
    if (subcategories.size() > MaximumRecords) {
        *errorMessage = QCoreApplication::translate("DataTransferService", "The backup contains too many subcategories.");
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
            *errorMessage = QCoreApplication::translate("DataTransferService", "A category contains duplicate subcategory names.");
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "Every milestone must be an object.");
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "Every goal must be an object.");
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
        *errorMessage = QCoreApplication::translate("DataTransferService", "'milestones' must be a list.");
        return false;
    }
    const QJsonArray milestones = milestonesValue.toArray();
    if (milestones.size() > MaximumRecords) {
        *errorMessage = QCoreApplication::translate("DataTransferService", "The backup contains too many milestones.");
        return false;
    }

    for (const QJsonValue &milestoneValue : milestones) {
        Milestone milestone;
        if (!milestoneFromJson(milestoneValue, goal->id, &milestone, errorMessage)) {
            return false;
        }
        if (milestoneIds->contains(milestone.id)) {
            *errorMessage = QCoreApplication::translate("DataTransferService", "The backup contains a duplicate milestone ID.");
            return false;
        }
        milestoneIds->insert(milestone.id);
        goal->milestones.append(milestone);
    }
    return true;
}

bool meetingFromJson(
    const QJsonValue &value,
    Meeting *meeting,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService",
            "Every meeting must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    return readString(object, QStringLiteral("id"), &meeting->id, errorMessage, false, 128)
        && readString(object, QStringLiteral("title"), &meeting->title, errorMessage, false)
        && readString(object, QStringLiteral("notes"), &meeting->notes, errorMessage)
        && readDateTime(
            object,
            QStringLiteral("startsAt"),
            &meeting->startsAt,
            errorMessage,
            true)
        && readDateTime(
            object,
            QStringLiteral("createdAt"),
            &meeting->createdAt,
            errorMessage,
            true)
        && readDateTime(
            object,
            QStringLiteral("notifiedAt"),
            &meeting->notifiedAt,
            errorMessage,
            false);
}

bool validMapColor(const QString &color)
{
    static const QSet<QString> colors {
        QStringLiteral("accent"),
        QStringLiteral("secondary"),
        QStringLiteral("success"),
        QStringLiteral("danger"),
        QStringLiteral("neutral"),
    };
    return colors.contains(color);
}

bool mapGroupFromJson(
    const QJsonValue &value,
    MentalMapGroup *group,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService", "Every map group must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    if (!readString(object, QStringLiteral("id"), &group->id, errorMessage, false, 128)
        || !readString(object, QStringLiteral("kind"), &group->kind, errorMessage, false, 32)
        || !readString(object, QStringLiteral("title"), &group->title, errorMessage, false, 120)
        || !readString(object, QStringLiteral("color"), &group->color, errorMessage, false, 32)
        || !readBoundedDouble(object, QStringLiteral("x"), -1000000000, 1000000000,
            &group->x, errorMessage)
        || !readBoundedDouble(object, QStringLiteral("y"), -1000000000, 1000000000,
            &group->y, errorMessage)
        || !readBoundedDouble(object, QStringLiteral("width"), 220, 1200,
            &group->width, errorMessage)
        || !readBoundedDouble(object, QStringLiteral("height"), 160, 900,
            &group->height, errorMessage)
        || !readDateTime(object, QStringLiteral("createdAt"), &group->createdAt,
            errorMessage, true)) {
        return false;
    }
    if ((group->kind != QStringLiteral("cloud")
            && group->kind != QStringLiteral("constellation"))
        || !validMapColor(group->color)) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService", "A map group has an unsupported kind or color.");
        return false;
    }
    group->title = group->title.trimmed();
    return true;
}

bool mapNoteFromJson(
    const QJsonValue &value,
    MentalMapNote *note,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService", "Every map note must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    if (!readString(object, QStringLiteral("id"), &note->id, errorMessage, false, 128)
        || !readString(object, QStringLiteral("groupId"), &note->groupId, errorMessage, false, 128)
        || !readString(object, QStringLiteral("title"), &note->title, errorMessage, false, 160)
        || !readString(object, QStringLiteral("body"), &note->body, errorMessage)
        || !readString(object, QStringLiteral("color"), &note->color, errorMessage, false, 32)
        || !readString(object, QStringLiteral("tags"), &note->tags, errorMessage, true, 1000)
        || !readString(object, QStringLiteral("externalLink"), &note->externalLink,
            errorMessage, true, 2000)
        || !readBoundedInt(object, QStringLiteral("priority"), 0, 5,
            &note->priority, errorMessage)
        || !readBoundedDouble(object, QStringLiteral("x"), -1000000000, 1000000000,
            &note->x, errorMessage)
        || !readBoundedDouble(object, QStringLiteral("y"), -1000000000, 1000000000,
            &note->y, errorMessage)
        || !readBool(object, QStringLiteral("center"), &note->center, errorMessage)
        || !readOptionalString(object, QStringLiteral("linkedTaskId"),
            &note->linkedTaskId, errorMessage, 128)
        || !readDateTime(object, QStringLiteral("createdAt"), &note->createdAt,
            errorMessage, true)) {
        return false;
    }
    if (!validMapColor(note->color)) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService", "A map note has an unsupported color.");
        return false;
    }
    const QJsonValue checklistValue = object.value(QStringLiteral("checklist"));
    if (!checklistValue.isArray() || checklistValue.toArray().size() > MaximumRecords) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService", "A map checklist must be a supported list.");
        return false;
    }
    for (const QJsonValue &itemValue : checklistValue.toArray()) {
        if (!itemValue.isObject()) {
            *errorMessage = QCoreApplication::translate(
                "DataTransferService", "Every checklist item must be an object.");
            return false;
        }
        const QJsonObject itemObject = itemValue.toObject();
        MentalMapChecklistItem item;
        if (!readString(itemObject, QStringLiteral("text"), &item.text,
                errorMessage, false, 300)
            || !readBool(itemObject, QStringLiteral("completed"),
                &item.completed, errorMessage)) {
            return false;
        }
        item.text = item.text.trimmed();
        note->checklist.append(item);
    }
    note->title = note->title.trimmed();
    return true;
}

bool mapConnectionFromJson(
    const QJsonValue &value,
    MentalMapConnection *connection,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService", "Every map connection must be an object.");
        return false;
    }
    const QJsonObject object = value.toObject();
    if (!readString(object, QStringLiteral("id"), &connection->id, errorMessage, false, 128)
        || !readString(object, QStringLiteral("viewKind"), &connection->viewKind,
            errorMessage, false, 32)
        || !readString(object, QStringLiteral("sourceNoteId"),
            &connection->sourceNoteId, errorMessage, false, 128)
        || !readString(object, QStringLiteral("targetNoteId"),
            &connection->targetNoteId, errorMessage, false, 128)
        || !readString(object, QStringLiteral("label"), &connection->label,
            errorMessage, true, 120)
        || !readDateTime(object, QStringLiteral("createdAt"), &connection->createdAt,
            errorMessage, true)) {
        return false;
    }
    if ((connection->viewKind != QStringLiteral("cloud")
            && connection->viewKind != QStringLiteral("constellation"))
        || connection->sourceNoteId == connection->targetNoteId) {
        *errorMessage = QCoreApplication::translate(
            "DataTransferService", "A map connection has invalid endpoints or view.");
        return false;
    }
    return true;
}

bool settingsFromJson(
    const QJsonValue &value,
    ImportedSettings *settings,
    QString *errorMessage)
{
    if (!value.isObject()) {
        *errorMessage = QCoreApplication::translate("DataTransferService", "'settings' must be an object.");
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
        && readOptionalBoundedInt(
            object,
            QStringLiteral("language"),
            0,
            1,
            &settings->language,
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
        && readOptionalBool(
            object,
            QStringLiteral("showTimeline"),
            &settings->showTimeline,
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
    target.setLanguage(static_cast<AppSettings::Language>(source.language));
    target.setColorMode(static_cast<AppSettings::ColorMode>(source.colorMode));
    target.setAccentPreset(static_cast<AppSettings::AccentPreset>(source.accentPreset));
    target.setDailyCapacityMinutes(source.dailyCapacityMinutes);
    target.setDefaultEstimatedMinutes(source.defaultEstimatedMinutes);
    target.setDefaultImportance(source.defaultImportance);
    target.setUse24HourClock(source.use24HourClock);
    target.setShowTimeline(source.showTimeline);
    target.setShowPriorityReasons(source.showPriorityReasons);
    target.setConfirmTaskCompletion(source.confirmTaskCompletion);
}

} // namespace

DataTransferService::DataTransferService(
    TaskRepository &repository,
    TaskListModel &taskModel,
    CategoryListModel &categoryModel,
    GoalListModel &goalModel,
    MeetingListModel &meetingModel,
    AppSettings &appSettings,
    QObject *parent)
    : QObject(parent)
    , m_repository(repository)
    , m_taskModel(taskModel)
    , m_categoryModel(categoryModel)
    , m_goalModel(goalModel)
    , m_meetingModel(meetingModel)
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
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Choose a local file for the export."));
        return false;
    }

    QString errorMessage;
    const QVector<Task> tasks = m_repository.allTasks(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not read tasks: %1").arg(errorMessage));
        return false;
    }
    const QVector<Category> categories = m_repository.categories(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not read categories: %1").arg(errorMessage));
        return false;
    }
    const QVector<Goal> goals = m_repository.goals(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not read goals: %1").arg(errorMessage));
        return false;
    }
    const QVector<Meeting> meetings = m_repository.meetings(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not read meetings: %1").arg(errorMessage));
        return false;
    }
    const QVector<MentalMapGroup> mapGroups = m_repository.mentalMapGroups(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QCoreApplication::translate(
            "DataTransferService", "Could not read map groups: %1").arg(errorMessage));
        return false;
    }
    const QVector<MentalMapNote> mapNotes = m_repository.mentalMapNotes(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QCoreApplication::translate(
            "DataTransferService", "Could not read map notes: %1").arg(errorMessage));
        return false;
    }
    const QVector<MentalMapConnection> mapConnections =
        m_repository.mentalMapConnections(&errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QCoreApplication::translate(
            "DataTransferService", "Could not read map connections: %1").arg(errorMessage));
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
    QJsonArray meetingArray;
    for (const Meeting &meeting : meetings) {
        meetingArray.append(meetingToJson(meeting));
    }
    QJsonArray mapGroupArray;
    for (const MentalMapGroup &group : mapGroups) {
        mapGroupArray.append(mapGroupToJson(group));
    }
    QJsonArray mapNoteArray;
    for (const MentalMapNote &note : mapNotes) {
        mapNoteArray.append(mapNoteToJson(note));
    }
    QJsonArray mapConnectionArray;
    for (const MentalMapConnection &connection : mapConnections) {
        mapConnectionArray.append(mapConnectionToJson(connection));
    }
    const QJsonObject mentalMap {
        {QStringLiteral("groups"), mapGroupArray},
        {QStringLiteral("notes"), mapNoteArray},
        {QStringLiteral("connections"), mapConnectionArray},
    };

    const QJsonObject root {
        {QStringLiteral("format"), QString::fromLatin1(BackupFormat)},
        {QStringLiteral("version"), BackupVersion},
        {QStringLiteral("exportedAt"), serializedDateTime(QDateTime::currentDateTime())},
        {QStringLiteral("applicationVersion"), QCoreApplication::applicationVersion()},
        {QStringLiteral("tasks"), taskArray},
        {QStringLiteral("categories"), categoryArray},
        {QStringLiteral("goals"), goalArray},
        {QStringLiteral("meetings"), meetingArray},
        {QStringLiteral("mentalMap"), mentalMap},
        {QStringLiteral("settings"), settingsToJson(m_appSettings)},
    };

    QString path = destination.toLocalFile();
    if (!path.endsWith(QStringLiteral(".daymark.json"), Qt::CaseInsensitive)) {
        path += QStringLiteral(".daymark.json");
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not create the export: %1").arg(file.errorString()));
        return false;
    }
    const QByteArray payload = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size() || !file.commit()) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not finish the export: %1").arg(file.errorString()));
        return false;
    }

    setStatusMessage(QCoreApplication::translate("DataTransferService",
        "Exported tasks, categories, goals, meetings, brainstorming maps, and preferences to %1.")
        .arg(QFileInfo(path).fileName()));
    return true;
}

bool DataTransferService::importData(const QUrl &source)
{
    if (!source.isLocalFile()) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Choose a local Daymark data file."));
        return false;
    }

    QFile file(source.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not open the import: %1").arg(file.errorString()));
        return false;
    }
    if (file.size() > MaximumBackupBytes) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "The selected file is too large to be a Daymark export."));
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "The selected file is not valid JSON: %1")
            .arg(parseError.errorString()));
        return false;
    }

    const QJsonObject root = document.object();
    if (root.value(QStringLiteral("format")).toString() != QString::fromLatin1(BackupFormat)) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "The selected file is not a Daymark export."));
        return false;
    }
    if (!root.value(QStringLiteral("version")).isDouble()
        || root.value(QStringLiteral("version")).toDouble() != BackupVersion) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "This Daymark export version is not supported."));
        return false;
    }

    const QJsonValue tasksValue = root.value(QStringLiteral("tasks"));
    const QJsonValue categoriesValue = root.value(QStringLiteral("categories"));
    const QJsonValue goalsValue = root.value(QStringLiteral("goals"));
    const QJsonValue meetingsValue = root.value(QStringLiteral("meetings"));
    const QJsonValue mentalMapValue = root.value(QStringLiteral("mentalMap"));
    if (!tasksValue.isArray() || !goalsValue.isArray()
        || (!categoriesValue.isUndefined() && !categoriesValue.isArray())
        || (!meetingsValue.isUndefined() && !meetingsValue.isArray())
        || (!mentalMapValue.isUndefined() && !mentalMapValue.isObject())) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "The export is missing its task or goal list."));
        return false;
    }

    const QJsonArray taskArray = tasksValue.toArray();
    const QJsonArray categoryArray = categoriesValue.isArray()
        ? categoriesValue.toArray()
        : QJsonArray();
    const QJsonArray goalArray = goalsValue.toArray();
    const QJsonArray meetingArray = meetingsValue.isArray()
        ? meetingsValue.toArray()
        : QJsonArray();
    const QJsonObject mentalMap = mentalMapValue.isObject()
        ? mentalMapValue.toObject() : QJsonObject();
    const QJsonValue mapGroupsValue = mentalMap.value(QStringLiteral("groups"));
    const QJsonValue mapNotesValue = mentalMap.value(QStringLiteral("notes"));
    const QJsonValue mapConnectionsValue = mentalMap.value(QStringLiteral("connections"));
    if ((!mapGroupsValue.isUndefined() && !mapGroupsValue.isArray())
        || (!mapNotesValue.isUndefined() && !mapNotesValue.isArray())
        || (!mapConnectionsValue.isUndefined() && !mapConnectionsValue.isArray())) {
        setStatusMessage(QCoreApplication::translate(
            "DataTransferService", "The export contains an invalid mental map."));
        return false;
    }
    const QJsonArray mapGroupArray = mapGroupsValue.isArray()
        ? mapGroupsValue.toArray() : QJsonArray();
    const QJsonArray mapNoteArray = mapNotesValue.isArray()
        ? mapNotesValue.toArray() : QJsonArray();
    const QJsonArray mapConnectionArray = mapConnectionsValue.isArray()
        ? mapConnectionsValue.toArray() : QJsonArray();
    if (taskArray.size() > MaximumRecords || categoryArray.size() > MaximumRecords
        || goalArray.size() > MaximumRecords || meetingArray.size() > MaximumRecords
        || mapGroupArray.size() > MaximumRecords || mapNoteArray.size() > MaximumRecords
        || mapConnectionArray.size() > MaximumRecords) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "The export contains too many records."));
        return false;
    }

    QVector<Task> tasks;
    QVector<Category> categories;
    QVector<Goal> goals;
    QVector<Meeting> meetings;
    QVector<MentalMapGroup> mapGroups;
    QVector<MentalMapNote> mapNotes;
    QVector<MentalMapConnection> mapConnections;
    QSet<QString> taskIds;
    QSet<QString> categoryIds;
    QSet<QString> categoryNames;
    QHash<QString, QString> categoryIdsByName;
    QHash<QString, QString> existingCategoryIdsByName;
    QHash<QString, QString> subcategoryParents;
    QSet<QString> goalIds;
    QSet<QString> milestoneIds;
    QSet<QString> meetingIds;
    QHash<QString, QString> mapGroupKinds;
    QHash<QString, QString> mapNoteKinds;
    QSet<QString> mapConnectionIds;
    QSet<QString> mapConnectionPairs;
    qsizetype subcategoryCount = 0;
    QString validationError;

    QString existingDataError;
    const QVector<Category> existingCategories = m_repository.categories(&existingDataError);
    if (!existingDataError.isEmpty()) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not inspect existing categories: %1")
            .arg(existingDataError));
        return false;
    }
    for (const Category &category : existingCategories) {
        existingCategoryIdsByName.insert(category.name.trimmed().toCaseFolded(), category.id);
    }

    for (const QJsonValue &value : categoryArray) {
        Category category;
        if (!categoryFromJson(value, &category, &validationError)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not import a category: %1").arg(validationError));
            return false;
        }
        const QString normalizedName = category.name.trimmed().toCaseFolded();
        if (categoryIds.contains(category.id) || categoryNames.contains(normalizedName)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService", "The export contains a duplicate category."));
            return false;
        }
        categoryIds.insert(category.id);
        categoryNames.insert(normalizedName);
        categoryIdsByName.insert(normalizedName, category.id);
        for (const Subcategory &subcategory : category.subcategories) {
            if (subcategoryParents.contains(subcategory.id)) {
                setStatusMessage(QCoreApplication::translate("DataTransferService",
                    "The export contains a duplicate subcategory ID."));
                return false;
            }
            subcategoryParents.insert(subcategory.id, category.id);
            ++subcategoryCount;
            if (subcategoryCount > MaximumRecords) {
                setStatusMessage(QCoreApplication::translate("DataTransferService", "The export contains too many records."));
                return false;
            }
        }
        categories.append(category);
    }

    for (const QJsonValue &value : taskArray) {
        Task task;
        if (!taskFromJson(value, &task, &validationError)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not import a task: %1").arg(validationError));
            return false;
        }
        if (taskIds.contains(task.id)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService", "The export contains a duplicate task ID."));
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
            setStatusMessage(QCoreApplication::translate("DataTransferService",
                "A task refers to a category that is missing from the export."));
            return false;
        }
        if (!task.subcategoryId.isEmpty()
            && (!subcategoryParents.contains(task.subcategoryId)
                || subcategoryParents.value(task.subcategoryId) != task.categoryId)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService",
                "A task refers to a subcategory outside its category."));
            return false;
        }
        taskIds.insert(task.id);
        tasks.append(task);
    }

    for (const QJsonValue &value : goalArray) {
        Goal goal;
        if (!goalFromJson(value, &goal, &milestoneIds, &validationError)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not import a goal: %1").arg(validationError));
            return false;
        }
        if (goalIds.contains(goal.id)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService", "The export contains a duplicate goal ID."));
            return false;
        }
        goalIds.insert(goal.id);
        goals.append(goal);
    }

    for (const QJsonValue &value : meetingArray) {
        Meeting meeting;
        if (!meetingFromJson(value, &meeting, &validationError)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not import a meeting: %1").arg(validationError));
            return false;
        }
        if (meetingIds.contains(meeting.id)) {
            setStatusMessage(QCoreApplication::translate("DataTransferService", "The export contains a duplicate meeting ID."));
            return false;
        }
        meetingIds.insert(meeting.id);
        meetings.append(meeting);
    }

    for (const QJsonValue &value : mapGroupArray) {
        MentalMapGroup group;
        if (!mapGroupFromJson(value, &group, &validationError)) {
            setStatusMessage(QCoreApplication::translate(
                "DataTransferService", "Could not import a map group: %1")
                .arg(validationError));
            return false;
        }
        if (mapGroupKinds.contains(group.id)) {
            setStatusMessage(QCoreApplication::translate(
                "DataTransferService", "The export contains a duplicate map group ID."));
            return false;
        }
        mapGroupKinds.insert(group.id, group.kind);
        mapGroups.append(group);
    }

    QSet<QString> mapNoteIds;
    QSet<QString> constellationCenters;
    for (const QJsonValue &value : mapNoteArray) {
        MentalMapNote note;
        if (!mapNoteFromJson(value, &note, &validationError)) {
            setStatusMessage(QCoreApplication::translate(
                "DataTransferService", "Could not import a map note: %1")
                .arg(validationError));
            return false;
        }
        if (mapNoteIds.contains(note.id) || !mapGroupKinds.contains(note.groupId)) {
            setStatusMessage(QCoreApplication::translate(
                "DataTransferService", "A map note has a duplicate ID or missing group."));
            return false;
        }
        note.groupKind = mapGroupKinds.value(note.groupId);
        if (note.center) {
            if (note.groupKind != QStringLiteral("constellation")
                || constellationCenters.contains(note.groupId)) {
                setStatusMessage(QCoreApplication::translate(
                    "DataTransferService", "A constellation has an invalid central idea."));
                return false;
            }
            constellationCenters.insert(note.groupId);
        }
        if (!note.linkedTaskId.isEmpty() && !taskIds.contains(note.linkedTaskId)) {
            setStatusMessage(QCoreApplication::translate(
                "DataTransferService", "A map note refers to a missing linked task."));
            return false;
        }
        mapNoteIds.insert(note.id);
        mapNoteKinds.insert(note.id, note.groupKind);
        mapNotes.append(note);
    }
    for (const MentalMapGroup &group : mapGroups) {
        if (group.kind == QStringLiteral("constellation")
            && !constellationCenters.contains(group.id)) {
            setStatusMessage(QCoreApplication::translate(
                "DataTransferService", "A constellation is missing its central idea."));
            return false;
        }
    }

    for (const QJsonValue &value : mapConnectionArray) {
        MentalMapConnection connection;
        if (!mapConnectionFromJson(value, &connection, &validationError)) {
            setStatusMessage(QCoreApplication::translate(
                "DataTransferService", "Could not import a map connection: %1")
                .arg(validationError));
            return false;
        }
        const QString pair = connection.sourceNoteId
            + QLatin1Char('\n') + connection.targetNoteId;
        if (mapConnectionIds.contains(connection.id) || mapConnectionPairs.contains(pair)
            || !mapNoteKinds.contains(connection.sourceNoteId)
            || !mapNoteKinds.contains(connection.targetNoteId)
            || mapNoteKinds.value(connection.sourceNoteId) != connection.viewKind
            || mapNoteKinds.value(connection.targetNoteId) != connection.viewKind) {
            setStatusMessage(QCoreApplication::translate(
                "DataTransferService", "A map connection has duplicate or invalid endpoints."));
            return false;
        }
        mapConnectionIds.insert(connection.id);
        mapConnectionPairs.insert(pair);
        mapConnections.append(connection);
    }

    ImportedSettings importedSettings;
    if (!settingsFromJson(
            root.value(QStringLiteral("settings")),
            &importedSettings,
            &validationError)) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not import preferences: %1").arg(validationError));
        return false;
    }

    QString databaseError;
    if (!m_repository.mergeImportedData(
            categories,
            tasks,
            goals,
            meetings,
            mapGroups,
            mapNotes,
            mapConnections,
            &databaseError)) {
        setStatusMessage(QCoreApplication::translate("DataTransferService", "Could not merge the imported data: %1").arg(databaseError));
        return false;
    }

    applySettings(importedSettings, m_appSettings);
    m_categoryModel.reload();
    m_taskModel.reload();
    m_goalModel.reload();
    m_meetingModel.reload();
    emit dataImported();
    setStatusMessage(QCoreApplication::translate("DataTransferService",
        "Imported %1 tasks, %2 categories, %3 subcategories, %4 goals, %5 meetings, "
        "and %6 map groups. "
        "Existing records were kept.")
        .arg(tasks.size())
        .arg(categories.size())
        .arg(subcategoryCount)
        .arg(goals.size())
        .arg(meetings.size())
        .arg(mapGroups.size()));
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
