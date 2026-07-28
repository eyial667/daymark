// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/goallistmodel.h"

#include <QDateTime>
#include <QLocale>
#include <QUuid>
#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

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
            return tr("No milestones yet");
        }
        return tr("%1 of %2 milestones")
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

bool GoalListModel::hasMilestoneSuggestion() const
{
    return !m_suggestedMilestoneTitle.isEmpty();
}

QString GoalListModel::suggestedMilestoneTitle() const
{
    return m_suggestedMilestoneTitle;
}

QString GoalListModel::suggestedMilestoneGoal() const
{
    return m_suggestedMilestoneGoal;
}

QString GoalListModel::suggestedMilestoneTarget() const
{
    return m_suggestedMilestoneTarget;
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
        setStatusMessage(tr("A goal needs a title."));
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
        setStatusMessage(tr("Could not save the goal: %1").arg(errorMessage));
        return false;
    }

    reload();
    setStatusMessage(tr("Goal added. Add milestones to make it actionable."));
    return true;
}

bool GoalListModel::addMilestone(
    int goalRow,
    const QString &title,
    const QString &targetDate)
{
    if (goalRow < 0 || goalRow >= m_goals.size()) {
        setStatusMessage(tr("That goal is no longer in the list."));
        return false;
    }

    const QString cleanTitle = title.trimmed();
    if (cleanTitle.isEmpty()) {
        setStatusMessage(tr("A milestone needs a title."));
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
        setStatusMessage(tr("Could not save the milestone: %1").arg(errorMessage));
        return false;
    }

    reload();
    setStatusMessage(tr("Milestone added."));
    return true;
}

bool GoalListModel::deleteGoal(int goalRow)
{
    if (goalRow < 0 || goalRow >= m_goals.size()) {
        setStatusMessage(tr("That goal is no longer in the list."));
        return false;
    }
    const QString title = m_goals.at(goalRow).title;
    QString errorMessage;
    if (!m_repository.deleteGoal(m_goals.at(goalRow).id, &errorMessage)) {
        setStatusMessage(tr("Could not delete the goal: %1").arg(errorMessage));
        return false;
    }
    reload();
    setStatusMessage(tr("Goal “%1” and its milestones deleted.").arg(title));
    return true;
}

bool GoalListModel::deleteMilestone(int goalRow, int milestoneIndex)
{
    if (goalRow < 0 || goalRow >= m_goals.size()
        || milestoneIndex < 0
        || milestoneIndex >= m_goals.at(goalRow).milestones.size()) {
        setStatusMessage(tr("That milestone is no longer in the list."));
        return false;
    }
    const Milestone &milestone = m_goals.at(goalRow).milestones.at(milestoneIndex);
    const QString title = milestone.title;
    QString errorMessage;
    if (!m_repository.deleteMilestone(milestone.id, &errorMessage)) {
        setStatusMessage(tr("Could not delete the milestone: %1").arg(errorMessage));
        return false;
    }
    reload();
    setStatusMessage(tr("Milestone “%1” deleted.").arg(title));
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
        setStatusMessage(tr("That milestone is no longer in the list."));
        return false;
    }

    const Milestone &milestone = m_goals.at(goalRow).milestones.at(milestoneIndex);
    QString errorMessage;
    if (!m_repository.setMilestoneCompleted(milestone.id, completed, &errorMessage)) {
        setStatusMessage(tr("Could not update the milestone: %1").arg(errorMessage));
        return false;
    }

    reload();
    setStatusMessage(completed
        ? tr("Milestone completed.")
        : tr("Milestone reopened."));
    return true;
}

bool GoalListModel::completeGoal(int goalRow)
{
    if (goalRow < 0 || goalRow >= m_goals.size()) {
        setStatusMessage(tr("That goal is no longer in the list."));
        return false;
    }

    QString errorMessage;
    if (!m_repository.setGoalCompleted(m_goals.at(goalRow).id, true, &errorMessage)) {
        setStatusMessage(tr("Could not complete the goal: %1").arg(errorMessage));
        return false;
    }

    reload();
    setStatusMessage(tr("Goal marked achieved."));
    return true;
}

bool GoalListModel::planSuggestedMilestone(int importance, int estimatedMinutes)
{
    if (!hasMilestoneSuggestion()) {
        setStatusMessage(tr("There is no unfinished milestone to suggest."));
        return false;
    }

    Task task;
    task.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    task.title = m_suggestedMilestoneTitle;
    task.notes = tr("Milestone for goal: %1").arg(m_suggestedMilestoneGoal);
    task.plannedDate = QDate::currentDate();
    task.createdAt = QDateTime::currentDateTime();
    task.importance = std::clamp(importance, 1, 5);
    task.estimatedMinutes = std::clamp(estimatedMinutes, 5, 480);

    QString errorMessage;
    if (!m_repository.addTask(task, &errorMessage)) {
        setStatusMessage(tr("Could not plan the milestone: %1").arg(errorMessage));
        return false;
    }

    emit taskCreated();
    setStatusMessage(tr("Added “%1” to Today.").arg(task.title));
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
    refreshMilestoneSuggestion();
    endResetModel();
    emit summaryChanged();

    if (!errorMessage.isEmpty()) {
        setStatusMessage(tr("Could not load goals: %1").arg(errorMessage));
    }
}

void GoalListModel::refreshMilestoneSuggestion()
{
    m_suggestedMilestoneTitle.clear();
    m_suggestedMilestoneGoal.clear();
    m_suggestedMilestoneTarget.clear();

    QDate selectedTarget;
    bool found = false;
    for (const Goal &goal : m_goals) {
        for (const Milestone &milestone : goal.milestones) {
            if (milestone.completed) {
                continue;
            }

            const QDate effectiveTarget = milestone.targetDate.isValid()
                ? milestone.targetDate
                : goal.targetDate;
            if (found
                && (!effectiveTarget.isValid()
                    || (selectedTarget.isValid() && effectiveTarget >= selectedTarget))) {
                continue;
            }

            found = true;
            selectedTarget = effectiveTarget;
            m_suggestedMilestoneTitle = milestone.title;
            m_suggestedMilestoneGoal = goal.title;
            m_suggestedMilestoneTarget = formatTargetDate(effectiveTarget);
        }
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
        *errorMessage = tr("Use YYYY-MM-DD for the target date.");
        return false;
    }
    *date = parsed;
    return true;
}

QString GoalListModel::formatTargetDate(const QDate &date)
{
    if (!date.isValid()) {
        return tr("No target date");
    }

    const int days = QDate::currentDate().daysTo(date);
    const QString formatted = QLocale().toString(date, QLocale::ShortFormat);
    if (days < 0) {
        return tr("Overdue · %1").arg(formatted);
    }
    if (days == 0) {
        return tr("Due today");
    }
    return tr("Target %1").arg(formatted);
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
