// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/goallistmodel.h"

#include <QDateTime>
#include <QLocale>
#include <QUuid>
#include <QVariantList>
#include <QVariantMap>

GoalListModel::GoalListModel(TaskRepository &repository, QObject *parent)
    : QAbstractListModel(parent)
    , m_repository(repository)
{
    reload();
}

int GoalListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_goals.size();
}

QVariant GoalListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_goals.size()) {
        return {};
    }

    const Goal &goal = m_goals.at(index.row());
    const int completed = completedCount(goal);
    switch (role) {
    case IdRole:
        return goal.id;
    case TitleRole:
        return goal.title;
    case DescriptionRole:
        return goal.description;
    case TargetTextRole:
        return formatTargetDate(goal.targetDate);
    case ProgressRole:
        return goal.milestones.isEmpty()
            ? 0.0
            : static_cast<double>(completed) / static_cast<double>(goal.milestones.size());
    case ProgressTextRole:
        if (goal.milestones.isEmpty()) {
            return QStringLiteral("No milestones yet");
        }
        return QStringLiteral("%1 of %2 milestones")
            .arg(completed)
            .arg(goal.milestones.size());
    case MilestonesRole: {
        QVariantList milestones;
        milestones.reserve(goal.milestones.size());
        for (const Milestone &milestone : goal.milestones) {
            QVariantMap item;
            item.insert(QStringLiteral("milestoneId"), milestone.id);
            item.insert(QStringLiteral("title"), milestone.title);
            item.insert(QStringLiteral("targetText"), formatTargetDate(milestone.targetDate));
            item.insert(QStringLiteral("completed"), milestone.completed);
            milestones.append(item);
        }
        return milestones;
    }
    default:
        return {};
    }
}

QHash<int, QByteArray> GoalListModel::roleNames() const
{
    return {
        {IdRole, "goalId"},
        {TitleRole, "title"},
        {DescriptionRole, "description"},
        {TargetTextRole, "targetText"},
        {ProgressRole, "progress"},
        {ProgressTextRole, "progressText"},
        {MilestonesRole, "milestones"},
    };
}

int GoalListModel::goalCount() const
{
    return m_goals.size();
}

int GoalListModel::totalMilestoneCount() const
{
    int total = 0;
    for (const Goal &goal : m_goals) {
        total += goal.milestones.size();
    }
    return total;
}

int GoalListModel::completedMilestoneCount() const
{
    int total = 0;
    for (const Goal &goal : m_goals) {
        total += completedCount(goal);
    }
    return total;
}

QString GoalListModel::statusMessage() const
{
    return m_statusMessage;
}

bool GoalListModel::addGoal(
    const QString &title,
    const QString &description,
    const QString &targetDate)
{
    const QString cleanTitle = title.trimmed();
    if (cleanTitle.isEmpty()) {
        setStatusMessage(QStringLiteral("A goal needs a title."));
        return false;
    }

    QDate parsedTarget;
    QString dateError;
    if (!parseDate(targetDate, &parsedTarget, &dateError)) {
        setStatusMessage(dateError);
        return false;
    }

    Goal goal;
    goal.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    goal.title = cleanTitle;
    goal.description = description.trimmed();
    goal.targetDate = parsedTarget;
    goal.createdAt = QDateTime::currentDateTime();

    QString errorMessage;
    if (!m_repository.addGoal(goal, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not save the goal: %1").arg(errorMessage));
        return false;
    }

    reload();
    setStatusMessage(QStringLiteral("Goal added. Add milestones to make it actionable."));
    return true;
}

bool GoalListModel::addMilestone(
    int goalRow,
    const QString &title,
    const QString &targetDate)
{
    if (goalRow < 0 || goalRow >= m_goals.size()) {
        setStatusMessage(QStringLiteral("That goal is no longer in the list."));
        return false;
    }

    const QString cleanTitle = title.trimmed();
    if (cleanTitle.isEmpty()) {
        setStatusMessage(QStringLiteral("A milestone needs a title."));
        return false;
    }

    QDate parsedTarget;
    QString dateError;
    if (!parseDate(targetDate, &parsedTarget, &dateError)) {
        setStatusMessage(dateError);
        return false;
    }

    Milestone milestone;
    milestone.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    milestone.goalId = m_goals.at(goalRow).id;
    milestone.title = cleanTitle;
    milestone.targetDate = parsedTarget;
    milestone.createdAt = QDateTime::currentDateTime();

    QString errorMessage;
    if (!m_repository.addMilestone(milestone, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not save the milestone: %1").arg(errorMessage));
        return false;
    }

    reload();
    setStatusMessage(QStringLiteral("Milestone added."));
    return true;
}

bool GoalListModel::setMilestoneCompleted(
    int goalRow,
    int milestoneIndex,
    bool completed)
{
    if (goalRow < 0 || goalRow >= m_goals.size()
        || milestoneIndex < 0
        || milestoneIndex >= m_goals.at(goalRow).milestones.size()) {
        setStatusMessage(QStringLiteral("That milestone is no longer in the list."));
        return false;
    }

    const Milestone &milestone = m_goals.at(goalRow).milestones.at(milestoneIndex);
    QString errorMessage;
    if (!m_repository.setMilestoneCompleted(milestone.id, completed, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not update the milestone: %1").arg(errorMessage));
        return false;
    }

    reload();
    setStatusMessage(completed
        ? QStringLiteral("Milestone completed.")
        : QStringLiteral("Milestone reopened."));
    return true;
}

bool GoalListModel::completeGoal(int goalRow)
{
    if (goalRow < 0 || goalRow >= m_goals.size()) {
        setStatusMessage(QStringLiteral("That goal is no longer in the list."));
        return false;
    }

    QString errorMessage;
    if (!m_repository.setGoalCompleted(m_goals.at(goalRow).id, true, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not complete the goal: %1").arg(errorMessage));
        return false;
    }

    reload();
    setStatusMessage(QStringLiteral("Goal marked achieved."));
    return true;
}

void GoalListModel::clearStatus()
{
    setStatusMessage({});
}

void GoalListModel::reload()
{
    QString errorMessage;
    const QVector<Goal> storedGoals = m_repository.goals(&errorMessage);
    QVector<Goal> activeGoals;
    activeGoals.reserve(storedGoals.size());
    for (const Goal &goal : storedGoals) {
        if (!goal.completed) {
            activeGoals.append(goal);
        }
    }

    beginResetModel();
    m_goals = std::move(activeGoals);
    endResetModel();
    emit summaryChanged();

    if (!errorMessage.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not load goals: %1").arg(errorMessage));
    }
}

void GoalListModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}

bool GoalListModel::parseDate(
    const QString &text,
    QDate *date,
    QString *errorMessage)
{
    const QString cleanText = text.trimmed();
    if (cleanText.isEmpty()) {
        *date = {};
        return true;
    }

    const QDate parsed = QDate::fromString(cleanText, Qt::ISODate);
    if (!parsed.isValid()) {
        *errorMessage = QStringLiteral("Use YYYY-MM-DD for the target date.");
        return false;
    }
    *date = parsed;
    return true;
}

QString GoalListModel::formatTargetDate(const QDate &date)
{
    if (!date.isValid()) {
        return QStringLiteral("No target date");
    }

    const int days = QDate::currentDate().daysTo(date);
    const QString formatted = QLocale().toString(date, QLocale::ShortFormat);
    if (days < 0) {
        return QStringLiteral("Overdue · %1").arg(formatted);
    }
    if (days == 0) {
        return QStringLiteral("Due today");
    }
    return QStringLiteral("Target %1").arg(formatted);
}

int GoalListModel::completedCount(const Goal &goal)
{
    int completed = 0;
    for (const Milestone &milestone : goal.milestones) {
        if (milestone.completed) {
            ++completed;
        }
    }
    return completed;
}
