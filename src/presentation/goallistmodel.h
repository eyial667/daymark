// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "data/taskrepository.h"

#include <QAbstractListModel>
#include <QVector>

class GoalListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int goalCount READ goalCount NOTIFY summaryChanged)
    Q_PROPERTY(int totalMilestoneCount READ totalMilestoneCount NOTIFY summaryChanged)
    Q_PROPERTY(int completedMilestoneCount READ completedMilestoneCount NOTIFY summaryChanged)
    Q_PROPERTY(bool hasMilestoneSuggestion READ hasMilestoneSuggestion NOTIFY summaryChanged)
    Q_PROPERTY(QString suggestedMilestoneTitle READ suggestedMilestoneTitle NOTIFY summaryChanged)
    Q_PROPERTY(QString suggestedMilestoneGoal READ suggestedMilestoneGoal NOTIFY summaryChanged)
    Q_PROPERTY(QString suggestedMilestoneTarget READ suggestedMilestoneTarget NOTIFY summaryChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        TargetTextRole,
        ProgressRole,
        ProgressTextRole,
        MilestonesRole
    };
    Q_ENUM(Role)

    explicit GoalListModel(TaskRepository &repository, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int goalCount() const;
    [[nodiscard]] int totalMilestoneCount() const;
    [[nodiscard]] int completedMilestoneCount() const;
    [[nodiscard]] bool hasMilestoneSuggestion() const;
    [[nodiscard]] QString suggestedMilestoneTitle() const;
    [[nodiscard]] QString suggestedMilestoneGoal() const;
    [[nodiscard]] QString suggestedMilestoneTarget() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool addGoal(
        const QString &title,
        const QString &description,
        const QString &targetDate);
    Q_INVOKABLE bool addMilestone(
        int goalRow,
        const QString &title,
        const QString &targetDate);
    Q_INVOKABLE bool setMilestoneCompleted(
        int goalRow,
        int milestoneIndex,
        bool completed);
    Q_INVOKABLE bool completeGoal(int goalRow);
    Q_INVOKABLE bool planSuggestedMilestone(int importance, int estimatedMinutes);
    Q_INVOKABLE void clearStatus();
    Q_INVOKABLE void reload();

signals:
    void summaryChanged();
    void statusMessageChanged();
    void taskCreated();

private:
    void setStatusMessage(const QString &message);
    void refreshMilestoneSuggestion();
    [[nodiscard]] static bool parseDate(
        const QString &text,
        QDate *date,
        QString *errorMessage);
    [[nodiscard]] static QString formatTargetDate(const QDate &date);
    [[nodiscard]] static int completedCount(const Goal &goal);

    TaskRepository &m_repository;
    QVector<Goal> m_goals;
    QString m_suggestedMilestoneTitle;
    QString m_suggestedMilestoneGoal;
    QString m_suggestedMilestoneTarget;
    QString m_statusMessage;
};
