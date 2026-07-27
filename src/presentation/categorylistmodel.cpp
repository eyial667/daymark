// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/categorylistmodel.h"

#include <QDateTime>
#include <QUuid>

CategoryListModel::CategoryListModel(TaskRepository &repository, QObject *parent)
    : QAbstractListModel(parent)
    , m_repository(repository)
{
    reload();
}

int CategoryListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_categories.size();
}

QVariant CategoryListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_categories.size()) {
        return {};
    }

    const Category &category = m_categories.at(index.row());
    switch (role) {
    case CategoryIdRole:
        return category.id;
    case NameRole:
        return category.name;
    case NotesRole:
        return category.notes;
    case TaskCountRole:
        return category.taskCount;
    default:
        return {};
    }
}

QHash<int, QByteArray> CategoryListModel::roleNames() const
{
    return {
        {CategoryIdRole, "categoryId"},
        {NameRole, "name"},
        {NotesRole, "notes"},
        {TaskCountRole, "taskCount"},
    };
}

int CategoryListModel::categoryCount() const
{
    return m_categories.size();
}

QStringList CategoryListModel::names() const
{
    QStringList result;
    result.reserve(m_categories.size());
    for (const Category &category : m_categories) {
        result.append(category.name);
    }
    return result;
}

QString CategoryListModel::statusMessage() const
{
    return m_statusMessage;
}

bool CategoryListModel::addCategory(const QString &name, const QString &notes)
{
    const QString cleanName = name.trimmed();
    const QString cleanNotes = notes.trimmed();
    if (cleanName.isEmpty()) {
        setStatusMessage(QStringLiteral("A category needs a name."));
        return false;
    }
    if (cleanName.size() > 120 || cleanNotes.size() > 10000) {
        setStatusMessage(QStringLiteral("The category name or notes are too long."));
        return false;
    }
    if (hasDuplicateName(cleanName, -1)) {
        setStatusMessage(QStringLiteral("A category with that name already exists."));
        return false;
    }

    Category category;
    category.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    category.name = cleanName;
    category.notes = cleanNotes;
    category.createdAt = QDateTime::currentDateTime();

    QString errorMessage;
    if (!m_repository.addCategory(category, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not save the category: %1").arg(errorMessage));
        return false;
    }
    reload();
    setStatusMessage(QStringLiteral("Category created."));
    return true;
}

bool CategoryListModel::updateCategory(
    int row,
    const QString &name,
    const QString &notes)
{
    if (row < 0 || row >= m_categories.size()) {
        setStatusMessage(QStringLiteral("That category is no longer in the list."));
        return false;
    }
    const QString cleanName = name.trimmed();
    const QString cleanNotes = notes.trimmed();
    if (cleanName.isEmpty()) {
        setStatusMessage(QStringLiteral("A category needs a name."));
        return false;
    }
    if (cleanName.size() > 120 || cleanNotes.size() > 10000) {
        setStatusMessage(QStringLiteral("The category name or notes are too long."));
        return false;
    }
    if (hasDuplicateName(cleanName, row)) {
        setStatusMessage(QStringLiteral("A category with that name already exists."));
        return false;
    }

    Category category = m_categories.at(row);
    category.name = cleanName;
    category.notes = cleanNotes;
    QString errorMessage;
    if (!m_repository.updateCategory(category, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not update the category: %1").arg(errorMessage));
        return false;
    }
    reload();
    setStatusMessage(QStringLiteral("Category updated."));
    return true;
}

QString CategoryListModel::idAt(int row) const
{
    return row >= 0 && row < m_categories.size() ? m_categories.at(row).id : QString();
}

int CategoryListModel::indexOfId(const QString &categoryId) const
{
    for (qsizetype index = 0; index < m_categories.size(); ++index) {
        if (m_categories.at(index).id == categoryId) {
            return index;
        }
    }
    return -1;
}

void CategoryListModel::clearStatus()
{
    setStatusMessage({});
}

void CategoryListModel::reload()
{
    QString errorMessage;
    QVector<Category> categories = m_repository.categories(&errorMessage);
    beginResetModel();
    m_categories = std::move(categories);
    endResetModel();
    emit categoriesChanged();
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not load categories: %1").arg(errorMessage));
    }
}

void CategoryListModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}

bool CategoryListModel::hasDuplicateName(const QString &name, int excludedRow) const
{
    for (qsizetype index = 0; index < m_categories.size(); ++index) {
        if (index != excludedRow
            && m_categories.at(index).name.compare(name, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}
