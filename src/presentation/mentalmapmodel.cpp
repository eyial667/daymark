// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/mentalmapmodel.h"

#include <QHash>
#include <QLocale>
#include <algorithm>

MentalMapModel::MentalMapModel(TaskRepository &repository, QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
    reload();
}

QVariantList MentalMapModel::hierarchy() const
{
    return m_hierarchy;
}

QVariantList MentalMapModel::flatNodes() const
{
    return m_flatNodes;
}

int MentalMapModel::itemCount() const
{
    return m_flatNodes.size();
}

int MentalMapModel::revision() const
{
    return m_revision;
}

QString MentalMapModel::statusMessage() const
{
    return m_statusMessage;
}

void MentalMapModel::reload()
{
    QString taskError;
    QString categoryError;
    QString goalError;
    const QVector<Task> tasks = m_repository.openTasks(&taskError);
    const QVector<Category> categories = m_repository.categories(&categoryError);
    const QVector<Goal> goals = m_repository.goals(&goalError);

    QHash<QString, QVector<Task>> directTasks;
    QHash<QString, QVector<Task>> subcategoryTasks;
    QVector<Task> uncategorizedTasks;
    for (const Task &task : tasks) {
        if (task.categoryId.isEmpty()) {
            uncategorizedTasks.append(task);
        } else if (task.subcategoryId.isEmpty()) {
            directTasks[task.categoryId].append(task);
        } else {
            subcategoryTasks[task.subcategoryId].append(task);
        }
    }

    QVariantList hierarchy;
    int colorIndex = 0;
    for (const Category &category : categories) {
        QVariantList children;
        for (const Subcategory &subcategory : category.subcategories) {
            QVariantList taskChildren;
            const QVector<Task> assignedTasks = subcategoryTasks.value(subcategory.id);
            for (const Task &task : assignedTasks) {
                taskChildren.append(makeNode(
                    QStringLiteral("task:%1").arg(task.id),
                    task.title,
                    QStringLiteral("task"),
                    taskDetail(task),
                    colorIndex,
                    category.id,
                    subcategory.id));
            }
            const QString detail = subcategory.notes.isEmpty()
                ? tr("%1 open %2")
                    .arg(taskChildren.size())
                    .arg(taskChildren.size() == 1
                        ? tr("task") : tr("tasks"))
                : subcategory.notes;
            children.append(makeNode(
                QStringLiteral("subcategory:%1").arg(subcategory.id),
                subcategory.name,
                QStringLiteral("subcategory"),
                detail,
                colorIndex,
                category.id,
                subcategory.id,
                taskChildren));
        }

        const QVector<Task> categoryTasks = directTasks.value(category.id);
        for (const Task &task : categoryTasks) {
            children.append(makeNode(
                QStringLiteral("task:%1").arg(task.id),
                task.title,
                QStringLiteral("task"),
                taskDetail(task),
                colorIndex,
                category.id));
        }
        const QString detail = category.notes.isEmpty()
            ? tr("%1 open %2")
                .arg(category.taskCount)
                .arg(category.taskCount == 1
                    ? tr("task") : tr("tasks"))
            : category.notes;
        hierarchy.append(makeNode(
            QStringLiteral("category:%1").arg(category.id),
            category.name,
            QStringLiteral("category"),
            detail,
            colorIndex,
            category.id,
            {},
            children));
        ++colorIndex;
    }

    if (!uncategorizedTasks.isEmpty()) {
        QVariantList children;
        for (const Task &task : uncategorizedTasks) {
            children.append(makeNode(
                QStringLiteral("task:%1").arg(task.id),
                task.title,
                QStringLiteral("task"),
                taskDetail(task),
                colorIndex));
        }
        hierarchy.append(makeNode(
            QStringLiteral("uncategorized"),
            tr("Uncategorized"),
            QStringLiteral("category"),
            tr("Tasks that have not been placed in a work area yet."),
            colorIndex,
            {},
            {},
            children));
        ++colorIndex;
    }

    for (const Goal &goal : goals) {
        if (goal.completed) {
            continue;
        }
        QVariantList children;
        for (const Milestone &milestone : goal.milestones) {
            if (milestone.completed) {
                continue;
            }
            children.append(makeNode(
                QStringLiteral("milestone:%1").arg(milestone.id),
                milestone.title,
                QStringLiteral("milestone"),
                targetDetail(milestone.targetDate, tr("No target date")),
                colorIndex));
        }
        hierarchy.append(makeNode(
            QStringLiteral("goal:%1").arg(goal.id),
            goal.title,
            QStringLiteral("goal"),
            goal.targetDate.isValid() && !goal.description.isEmpty()
                ? tr("%1 · %2").arg(
                    goal.description,
                    targetDetail(goal.targetDate, {}))
                : targetDetail(goal.targetDate, goal.description),
            colorIndex,
            {},
            {},
            children));
        ++colorIndex;
    }

    m_hierarchy = std::move(hierarchy);
    m_flatNodes.clear();
    const int branchCount = m_hierarchy.size();
    constexpr double pi = 3.14159265358979323846;
    const double fullCircle = 2.0 * pi;
    const double sectorWidth = branchCount > 0
        ? std::min(1.15, fullCircle / static_cast<double>(branchCount) * 0.78)
        : 0.0;
    for (int index = 0; index < branchCount; ++index) {
        const double angle = -pi / 2.0
            + fullCircle * static_cast<double>(index) / static_cast<double>(branchCount);
        appendFlatNode(
            m_hierarchy.at(index).toMap(),
            {},
            1,
            index,
            branchCount,
            index,
            branchCount,
            angle,
            sectorWidth);
    }
    ++m_revision;
    emit mapChanged();

    const QString error = !taskError.isEmpty() ? taskError
        : !categoryError.isEmpty() ? categoryError : goalError;
    if (!error.isEmpty()) {
        setStatusMessage(tr("Could not load the mental map: %1").arg(error));
    }
}

void MentalMapModel::clearStatus()
{
    setStatusMessage({});
}

QVariantMap MentalMapModel::makeNode(
    const QString &id,
    const QString &title,
    const QString &kind,
    const QString &detail,
    int colorIndex,
    const QString &categoryId,
    const QString &subcategoryId,
    const QVariantList &children)
{
    return {
        {QStringLiteral("nodeId"), id},
        {QStringLiteral("title"), title},
        {QStringLiteral("kind"), kind},
        {QStringLiteral("detail"), detail},
        {QStringLiteral("colorIndex"), colorIndex % 5},
        {QStringLiteral("categoryId"), categoryId},
        {QStringLiteral("subcategoryId"), subcategoryId},
        {QStringLiteral("children"), children},
        {QStringLiteral("childCount"), children.size()},
    };
}

QString MentalMapModel::taskDetail(const Task &task)
{
    QString dueText = tr("No deadline");
    if (task.dueAt.isValid()) {
        const int days = QDate::currentDate().daysTo(task.dueAt.date());
        if (days < 0) {
            dueText = tr("Overdue");
        } else if (days == 0) {
            dueText = tr("Due today");
        } else if (days == 1) {
            dueText = tr("Due tomorrow");
        } else {
            dueText = tr("Due %1").arg(
                QLocale().toString(task.dueAt.date(), QLocale::ShortFormat));
        }
    }
    return tr("%1 · %2 min").arg(dueText).arg(task.estimatedMinutes);
}

QString MentalMapModel::targetDetail(const QDate &targetDate, const QString &fallback)
{
    if (targetDate.isValid()) {
        return tr("Target %1").arg(
            QLocale().toString(targetDate, QLocale::ShortFormat));
    }
    return fallback.isEmpty() ? tr("No target date") : fallback;
}

void MentalMapModel::appendFlatNode(
    const QVariantMap &node,
    const QString &parentId,
    int depth,
    int branchIndex,
    int branchCount,
    int siblingIndex,
    int siblingCount,
    double angle,
    double sectorWidth)
{
    QVariantMap flatNode = node;
    const QVariantList children = flatNode.take(QStringLiteral("children")).toList();
    flatNode.insert(QStringLiteral("parentId"), parentId);
    flatNode.insert(QStringLiteral("depth"), depth);
    flatNode.insert(QStringLiteral("branchIndex"), branchIndex);
    flatNode.insert(QStringLiteral("branchCount"), branchCount);
    flatNode.insert(QStringLiteral("siblingIndex"), siblingIndex);
    flatNode.insert(QStringLiteral("siblingCount"), siblingCount);
    flatNode.insert(QStringLiteral("angle"), angle);
    const double radius = depth == 1 ? 0.28
        : depth == 2 ? 0.41
        : 0.50 + (siblingIndex % 2 == 0 ? 0.0 : 0.045);
    flatNode.insert(QStringLiteral("radius"), radius);
    m_flatNodes.append(flatNode);

    const int childCount = children.size();
    for (int index = 0; index < childCount; ++index) {
        const double offset = childCount > 1
            ? (static_cast<double>(index) - static_cast<double>(childCount - 1) / 2.0)
                * sectorWidth / static_cast<double>(childCount - 1)
            : 0.0;
        appendFlatNode(
            children.at(index).toMap(),
            node.value(QStringLiteral("nodeId")).toString(),
            depth + 1,
            branchIndex,
            branchCount,
            index,
            childCount,
            angle + offset,
            sectorWidth / std::max(2, childCount));
    }
}

void MentalMapModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}
