// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/priorityengine.h"
#include "data/taskrepository.h"

#include <QAbstractListModel>
#include <QVector>

class TaskListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int activeCount READ activeCount NOTIFY summaryChanged)
    Q_PROPERTY(int totalEstimatedMinutes READ totalEstimatedMinutes NOTIFY summaryChanged)
    Q_PROPERTY(QString plannedDuration READ plannedDuration NOTIFY summaryChanged)
    Q_PROPERTY(QString topTaskTitle READ topTaskTitle NOTIFY summaryChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        NotesRole,
        ProjectRole,
        DueTextRole,
        ImportanceRole,
        EstimatedMinutesRole,
        PriorityScoreRole,
        PriorityReasonRole
    };
    Q_ENUM(Role)

    explicit TaskListModel(TaskRepository &repository, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int activeCount() const;
    [[nodiscard]] int totalEstimatedMinutes() const;
    [[nodiscard]] QString plannedDuration() const;
    [[nodiscard]] QString topTaskTitle() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool addTask(
        const QString &title,
        const QString &dueDate,
        int importance,
        int estimatedMinutes,
        const QString &project);
    Q_INVOKABLE bool completeTask(int row);
    Q_INVOKABLE void clearStatus();
    Q_INVOKABLE void reload();

signals:
    void summaryChanged();
    void statusMessageChanged();

private:
    struct Item {
        Task task;
        PriorityResult priority;
    };

    void setStatusMessage(const QString &message);
    [[nodiscard]] static QString formatDueDate(const QDateTime &dueAt);

    TaskRepository &m_repository;
    QVector<Item> m_items;
    QString m_statusMessage;
};
