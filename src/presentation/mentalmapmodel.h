// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "data/taskrepository.h"

#include <QObject>
#include <QVariantList>

class MentalMapModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList groups READ groups NOTIFY mapChanged)
    Q_PROPERTY(QVariantList notes READ notes NOTIFY mapChanged)
    Q_PROPERTY(QVariantList connections READ connections NOTIFY mapChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY mapChanged)
    Q_PROPERTY(int revision READ revision NOTIFY mapChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit MentalMapModel(TaskRepository &repository, QObject *parent = nullptr);

    [[nodiscard]] QVariantList groups() const;
    [[nodiscard]] QVariantList notes() const;
    [[nodiscard]] QVariantList connections() const;
    [[nodiscard]] int itemCount() const;
    [[nodiscard]] int revision() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE QString addGroup(
        const QString &kind,
        const QString &title,
        double x,
        double y);
    Q_INVOKABLE bool updateGroup(
        const QString &groupId,
        const QString &title,
        const QString &color);
    Q_INVOKABLE bool moveGroup(const QString &groupId, double x, double y);
    Q_INVOKABLE bool resizeGroup(
        const QString &groupId,
        double width,
        double height);
    Q_INVOKABLE bool deleteGroup(const QString &groupId);

    Q_INVOKABLE QString addNote(
        const QString &groupId,
        const QString &title,
        double x,
        double y);
    Q_INVOKABLE bool updateNote(
        const QString &noteId,
        const QString &title,
        const QString &body,
        const QString &color,
        const QString &tags,
        const QString &externalLink,
        int priority);
    Q_INVOKABLE bool moveNote(const QString &noteId, double x, double y);
    Q_INVOKABLE bool deleteNote(const QString &noteId);
    Q_INVOKABLE bool addChecklistItem(const QString &noteId, const QString &text);
    Q_INVOKABLE bool toggleChecklistItem(
        const QString &noteId,
        int itemIndex,
        bool completed);
    Q_INVOKABLE bool deleteChecklistItem(const QString &noteId, int itemIndex);

    Q_INVOKABLE QString addConnection(
        const QString &sourceNoteId,
        const QString &targetNoteId,
        const QString &label);
    Q_INVOKABLE bool deleteConnection(const QString &connectionId);
    Q_INVOKABLE bool createTaskForNote(const QString &noteId);
    Q_INVOKABLE void reload();
    Q_INVOKABLE void clearStatus();

signals:
    void mapChanged();
    void statusMessageChanged();
    void taskCreated();

private:
    [[nodiscard]] MentalMapGroup *findGroup(const QString &groupId);
    [[nodiscard]] MentalMapNote *findNote(const QString &noteId);
    [[nodiscard]] const MentalMapNote *findNote(const QString &noteId) const;
    [[nodiscard]] bool saveGroup(MentalMapGroup &group, const QString &action);
    [[nodiscard]] bool saveNote(MentalMapNote &note, const QString &action);
    void rebuildVariants();
    void setStatusMessage(const QString &message);

    TaskRepository &m_repository;
    QVector<MentalMapGroup> m_storedGroups;
    QVector<MentalMapNote> m_storedNotes;
    QVector<MentalMapConnection> m_storedConnections;
    QVariantList m_groups;
    QVariantList m_notes;
    QVariantList m_connections;
    int m_revision = 0;
    QString m_statusMessage;
};
