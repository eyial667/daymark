// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/mentalmapmodel.h"

#include <QDateTime>
#include <QUuid>
#include <QVariantMap>

#include <algorithm>

namespace {

QString cleanColor(const QString &color)
{
    static const QStringList colors {
        QStringLiteral("accent"),
        QStringLiteral("secondary"),
        QStringLiteral("success"),
        QStringLiteral("danger"),
        QStringLiteral("neutral"),
    };
    const QString candidate = color.trimmed().toLower();
    return colors.contains(candidate) ? candidate : QStringLiteral("accent");
}

QVariantMap groupVariant(const MentalMapGroup &group, int noteCount)
{
    return {
        {QStringLiteral("groupId"), group.id},
        {QStringLiteral("kind"), group.kind},
        {QStringLiteral("title"), group.title},
        {QStringLiteral("color"), group.color},
        {QStringLiteral("x"), group.x},
        {QStringLiteral("y"), group.y},
        {QStringLiteral("width"), group.width},
        {QStringLiteral("height"), group.height},
        {QStringLiteral("noteCount"), noteCount},
    };
}

QVariantMap noteVariant(const MentalMapNote &note)
{
    QVariantList checklist;
    for (const MentalMapChecklistItem &item : note.checklist) {
        checklist.append(QVariantMap {
            {QStringLiteral("text"), item.text},
            {QStringLiteral("completed"), item.completed},
        });
    }
    return {
        {QStringLiteral("noteId"), note.id},
        {QStringLiteral("groupId"), note.groupId},
        {QStringLiteral("groupKind"), note.groupKind},
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
        {QStringLiteral("linkedTaskTitle"), note.linkedTaskTitle},
        {QStringLiteral("linkedTaskCompleted"), note.linkedTaskCompleted},
        {QStringLiteral("checklist"), checklist},
    };
}

QVariantMap connectionVariant(const MentalMapConnection &connection)
{
    return {
        {QStringLiteral("connectionId"), connection.id},
        {QStringLiteral("viewKind"), connection.viewKind},
        {QStringLiteral("sourceNoteId"), connection.sourceNoteId},
        {QStringLiteral("targetNoteId"), connection.targetNoteId},
        {QStringLiteral("label"), connection.label},
    };
}

} // namespace

MentalMapModel::MentalMapModel(TaskRepository &repository, QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
    reload();
}

QVariantList MentalMapModel::groups() const { return m_groups; }
QVariantList MentalMapModel::notes() const { return m_notes; }
QVariantList MentalMapModel::connections() const { return m_connections; }
int MentalMapModel::itemCount() const { return m_storedGroups.size() + m_storedNotes.size(); }
int MentalMapModel::revision() const { return m_revision; }
QString MentalMapModel::statusMessage() const { return m_statusMessage; }

QString MentalMapModel::addGroup(
    const QString &kind,
    const QString &title,
    double x,
    double y)
{
    const QString cleanKind = kind.trimmed().toLower();
    const QString cleanTitle = title.trimmed();
    if (cleanKind != QStringLiteral("cloud")
        && cleanKind != QStringLiteral("constellation")) {
        setStatusMessage(tr("Choose either a cloud or a constellation."));
        return {};
    }
    if (cleanTitle.isEmpty() || cleanTitle.size() > 120) {
        setStatusMessage(tr("A map group needs a short title."));
        return {};
    }

    MentalMapGroup group;
    group.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    group.kind = cleanKind;
    group.title = cleanTitle;
    group.color = QStringLiteral("accent");
    group.x = x;
    group.y = y;
    group.width = cleanKind == QStringLiteral("cloud") ? 380.0 : 520.0;
    group.height = cleanKind == QStringLiteral("cloud") ? 280.0 : 400.0;
    group.createdAt = QDateTime::currentDateTime();

    QString error;
    if (!m_repository.addMentalMapGroup(group, &error)) {
        setStatusMessage(tr("Could not create the map group: %1").arg(error));
        return {};
    }

    if (cleanKind == QStringLiteral("constellation")) {
        MentalMapNote center;
        center.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        center.groupId = group.id;
        center.groupKind = group.kind;
        center.title = cleanTitle;
        center.color = group.color;
        center.x = group.width / 2.0 - 72.0;
        center.y = group.height / 2.0 - 28.0;
        center.center = true;
        center.createdAt = group.createdAt;
        if (!m_repository.addMentalMapNote(center, &error)) {
            QString cleanupError;
            const bool cleaned = m_repository.deleteMentalMapGroup(group.id, &cleanupError);
            Q_UNUSED(cleaned);
            setStatusMessage(tr("Could not create the central idea: %1").arg(error));
            return {};
        }
    }

    reload();
    setStatusMessage(cleanKind == QStringLiteral("cloud")
            ? tr("Cloud created. Double-click inside it to capture an idea.")
            : tr("Constellation created. Add surrounding ideas and connect them."));
    return group.id;
}

bool MentalMapModel::updateGroup(
    const QString &groupId,
    const QString &title,
    const QString &color)
{
    MentalMapGroup *group = findGroup(groupId);
    const QString cleanTitle = title.trimmed();
    if (group == nullptr) {
        setStatusMessage(tr("That map group no longer exists."));
        return false;
    }
    if (cleanTitle.isEmpty() || cleanTitle.size() > 120) {
        setStatusMessage(tr("A map group needs a short title."));
        return false;
    }
    group->title = cleanTitle;
    group->color = cleanColor(color);
    return saveGroup(*group, tr("Map group updated."));
}

bool MentalMapModel::moveGroup(const QString &groupId, double x, double y)
{
    MentalMapGroup *group = findGroup(groupId);
    if (group == nullptr) {
        return false;
    }
    group->x = x;
    group->y = y;
    return saveGroup(*group, {});
}

bool MentalMapModel::resizeGroup(
    const QString &groupId,
    double width,
    double height)
{
    MentalMapGroup *group = findGroup(groupId);
    if (group == nullptr) {
        return false;
    }
    group->width = std::clamp(width, 220.0, 1200.0);
    group->height = std::clamp(height, 160.0, 900.0);
    return saveGroup(*group, {});
}

bool MentalMapModel::deleteGroup(const QString &groupId)
{
    QString error;
    if (!m_repository.deleteMentalMapGroup(groupId, &error)) {
        setStatusMessage(tr("Could not delete the map group: %1").arg(error));
        return false;
    }
    reload();
    setStatusMessage(tr("Map group and all of its notes deleted."));
    return true;
}

QString MentalMapModel::addNote(
    const QString &groupId,
    const QString &title,
    double x,
    double y)
{
    MentalMapGroup *group = findGroup(groupId);
    const QString cleanTitle = title.trimmed();
    if (group == nullptr) {
        setStatusMessage(tr("Choose a cloud or constellation first."));
        return {};
    }
    if (cleanTitle.isEmpty() || cleanTitle.size() > 160) {
        setStatusMessage(tr("An idea needs a short title."));
        return {};
    }

    MentalMapNote note;
    note.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    note.groupId = group->id;
    note.groupKind = group->kind;
    note.title = cleanTitle;
    note.color = group->color;
    note.x = x;
    note.y = y;
    note.createdAt = QDateTime::currentDateTime();
    QString error;
    if (!m_repository.addMentalMapNote(note, &error)) {
        setStatusMessage(tr("Could not save the idea: %1").arg(error));
        return {};
    }
    reload();
    setStatusMessage(tr("Idea added."));
    return note.id;
}

bool MentalMapModel::updateNote(
    const QString &noteId,
    const QString &title,
    const QString &body,
    const QString &color,
    const QString &tags,
    const QString &externalLink,
    int priority)
{
    MentalMapNote *note = findNote(noteId);
    const QString cleanTitle = title.trimmed();
    if (note == nullptr) {
        setStatusMessage(tr("That idea no longer exists."));
        return false;
    }
    if (cleanTitle.isEmpty() || cleanTitle.size() > 160 || body.size() > 10000) {
        setStatusMessage(tr("Keep the idea title short and its notes under 10,000 characters."));
        return false;
    }
    note->title = cleanTitle;
    note->body = body.trimmed();
    note->color = cleanColor(color);
    note->tags = tags.trimmed().left(1000);
    note->externalLink = externalLink.trimmed().left(2000);
    note->priority = std::clamp(priority, 0, 5);
    return saveNote(*note, tr("Idea updated."));
}

bool MentalMapModel::moveNote(const QString &noteId, double x, double y)
{
    MentalMapNote *note = findNote(noteId);
    if (note == nullptr) {
        return false;
    }
    note->x = x;
    note->y = y;
    return saveNote(*note, {});
}

bool MentalMapModel::deleteNote(const QString &noteId)
{
    const MentalMapNote *note = findNote(noteId);
    if (note == nullptr) {
        setStatusMessage(tr("That idea no longer exists."));
        return false;
    }
    if (note->center) {
        setStatusMessage(tr("Delete the constellation to remove its central idea."));
        return false;
    }
    QString error;
    if (!m_repository.deleteMentalMapNote(noteId, &error)) {
        setStatusMessage(tr("Could not delete the idea: %1").arg(error));
        return false;
    }
    reload();
    setStatusMessage(tr("Idea and its connections deleted."));
    return true;
}

bool MentalMapModel::addChecklistItem(const QString &noteId, const QString &text)
{
    MentalMapNote *note = findNote(noteId);
    const QString cleanText = text.trimmed();
    if (note == nullptr || cleanText.isEmpty()) {
        setStatusMessage(tr("Enter a checklist item first."));
        return false;
    }
    note->checklist.append({cleanText.left(300), false});
    return saveNote(*note, tr("Checklist item added."));
}

bool MentalMapModel::toggleChecklistItem(
    const QString &noteId,
    int itemIndex,
    bool completed)
{
    MentalMapNote *note = findNote(noteId);
    if (note == nullptr || itemIndex < 0 || itemIndex >= note->checklist.size()) {
        setStatusMessage(tr("That checklist item no longer exists."));
        return false;
    }
    note->checklist[itemIndex].completed = completed;
    return saveNote(*note, {});
}

bool MentalMapModel::deleteChecklistItem(const QString &noteId, int itemIndex)
{
    MentalMapNote *note = findNote(noteId);
    if (note == nullptr || itemIndex < 0 || itemIndex >= note->checklist.size()) {
        setStatusMessage(tr("That checklist item no longer exists."));
        return false;
    }
    note->checklist.removeAt(itemIndex);
    return saveNote(*note, tr("Checklist item deleted."));
}

QString MentalMapModel::addConnection(
    const QString &sourceNoteId,
    const QString &targetNoteId,
    const QString &label)
{
    const MentalMapNote *source = findNote(sourceNoteId);
    const MentalMapNote *target = findNote(targetNoteId);
    if (source == nullptr || target == nullptr || sourceNoteId == targetNoteId) {
        setStatusMessage(tr("Choose two different ideas to connect."));
        return {};
    }
    if (source->groupKind != target->groupKind) {
        setStatusMessage(tr("Connections stay within the current map view."));
        return {};
    }
    MentalMapConnection connection;
    connection.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    connection.viewKind = source->groupKind;
    connection.sourceNoteId = sourceNoteId;
    connection.targetNoteId = targetNoteId;
    connection.label = label.trimmed().left(120);
    connection.createdAt = QDateTime::currentDateTime();
    QString error;
    if (!m_repository.addMentalMapConnection(connection, &error)) {
        setStatusMessage(tr("Could not create the connection: %1").arg(error));
        return {};
    }
    reload();
    setStatusMessage(tr("Directed connection created."));
    return connection.id;
}

bool MentalMapModel::deleteConnection(const QString &connectionId)
{
    QString error;
    if (!m_repository.deleteMentalMapConnection(connectionId, &error)) {
        setStatusMessage(tr("Could not delete the connection: %1").arg(error));
        return false;
    }
    reload();
    setStatusMessage(tr("Connection deleted."));
    return true;
}

bool MentalMapModel::createTaskForNote(const QString &noteId)
{
    MentalMapNote *note = findNote(noteId);
    if (note == nullptr) {
        setStatusMessage(tr("That idea no longer exists."));
        return false;
    }
    if (!note->linkedTaskId.isEmpty()) {
        setStatusMessage(tr("This idea is already linked to a task."));
        return false;
    }
    Task task;
    task.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    task.title = note->title;
    task.notes = note->body;
    task.createdAt = QDateTime::currentDateTime();
    task.importance = note->priority > 0 ? note->priority : 3;
    task.estimatedMinutes = 30;
    QString error;
    if (!m_repository.addTask(task, &error)) {
        setStatusMessage(tr("Could not create the linked task: %1").arg(error));
        return false;
    }
    if (!m_repository.linkMentalMapNoteToTask(noteId, task.id, &error)) {
        QString cleanupError;
        const bool cleaned = m_repository.deleteTask(task.id, &cleanupError);
        Q_UNUSED(cleaned);
        setStatusMessage(tr("Could not link the new task: %1").arg(error));
        return false;
    }
    reload();
    emit taskCreated();
    setStatusMessage(tr("Task created and linked to this idea."));
    return true;
}

void MentalMapModel::reload()
{
    QString groupError;
    QString noteError;
    QString connectionError;
    m_storedGroups = m_repository.mentalMapGroups(&groupError);
    m_storedNotes = m_repository.mentalMapNotes(&noteError);
    m_storedConnections = m_repository.mentalMapConnections(&connectionError);
    rebuildVariants();
    const QString error = !groupError.isEmpty() ? groupError
        : !noteError.isEmpty() ? noteError : connectionError;
    if (!error.isEmpty()) {
        setStatusMessage(tr("Could not load the mental map: %1").arg(error));
    }
}

void MentalMapModel::clearStatus()
{
    setStatusMessage({});
}

MentalMapGroup *MentalMapModel::findGroup(const QString &groupId)
{
    const auto match = std::find_if(
        m_storedGroups.begin(), m_storedGroups.end(), [&groupId](const MentalMapGroup &group) {
            return group.id == groupId;
        });
    return match == m_storedGroups.end() ? nullptr : &*match;
}

MentalMapNote *MentalMapModel::findNote(const QString &noteId)
{
    const auto match = std::find_if(
        m_storedNotes.begin(), m_storedNotes.end(), [&noteId](const MentalMapNote &note) {
            return note.id == noteId;
        });
    return match == m_storedNotes.end() ? nullptr : &*match;
}

const MentalMapNote *MentalMapModel::findNote(const QString &noteId) const
{
    const auto match = std::find_if(
        m_storedNotes.cbegin(), m_storedNotes.cend(), [&noteId](const MentalMapNote &note) {
            return note.id == noteId;
        });
    return match == m_storedNotes.cend() ? nullptr : &*match;
}

bool MentalMapModel::saveGroup(MentalMapGroup &group, const QString &action)
{
    QString error;
    if (!m_repository.updateMentalMapGroup(group, &error)) {
        setStatusMessage(tr("Could not update the map group: %1").arg(error));
        reload();
        return false;
    }
    rebuildVariants();
    if (!action.isEmpty()) {
        setStatusMessage(action);
    }
    return true;
}

bool MentalMapModel::saveNote(MentalMapNote &note, const QString &action)
{
    QString error;
    if (!m_repository.updateMentalMapNote(note, &error)) {
        setStatusMessage(tr("Could not update the idea: %1").arg(error));
        reload();
        return false;
    }
    rebuildVariants();
    if (!action.isEmpty()) {
        setStatusMessage(action);
    }
    return true;
}

void MentalMapModel::rebuildVariants()
{
    m_groups.clear();
    m_notes.clear();
    m_connections.clear();
    for (const MentalMapGroup &group : m_storedGroups) {
        const int noteCount = static_cast<int>(std::count_if(
            m_storedNotes.cbegin(), m_storedNotes.cend(), [&group](const MentalMapNote &note) {
                return note.groupId == group.id;
            }));
        m_groups.append(groupVariant(group, noteCount));
    }
    for (const MentalMapNote &note : m_storedNotes) {
        m_notes.append(noteVariant(note));
    }
    for (const MentalMapConnection &connection : m_storedConnections) {
        m_connections.append(connectionVariant(connection));
    }
    ++m_revision;
    emit mapChanged();
}

void MentalMapModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}
