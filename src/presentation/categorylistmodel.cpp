// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/categorylistmodel.h"

#include <QDateTime>
#include <QHash>
#include <QUuid>
#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

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
    case SubcategoryCountRole:
        return category.subcategories.size();
    case SubcategoriesRole: {
        QVariantList subcategories;
        subcategories.reserve(category.subcategories.size());
        for (const Subcategory &subcategory : category.subcategories) {
            subcategories.append(QVariantMap {
                {QStringLiteral("subcategoryId"), subcategory.id},
                {QStringLiteral("name"), subcategory.name},
                {QStringLiteral("notes"), subcategory.notes},
                {QStringLiteral("taskCount"), subcategory.taskCount},
            });
        }
        return subcategories;
    }
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
        {SubcategoryCountRole, "subcategoryCount"},
        {SubcategoriesRole, "subcategories"},
    };
}

int CategoryListModel::categoryCount() const
{
    return m_categories.size();
}

int CategoryListModel::subcategoryCount() const
{
    int count = 0;
    for (const Category &category : m_categories) {
        count += category.subcategories.size();
    }
    return count;
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

QStringList CategoryListModel::assignmentNames() const
{
    QStringList result;
    for (const Category &category : m_categories) {
        result.append(category.name);
        for (const Subcategory &subcategory : category.subcategories) {
            result.append(QStringLiteral("%1 / %2").arg(category.name, subcategory.name));
        }
    }
    return result;
}

bool CategoryListModel::hasWorkSuggestion() const
{
    return !m_workSuggestions.isEmpty();
}

int CategoryListModel::workSuggestionCount() const
{
    return m_workSuggestions.size();
}

QString CategoryListModel::workSuggestionName() const
{
    return hasWorkSuggestion() ? m_workSuggestions.at(m_workSuggestionIndex).name : QString();
}

QString CategoryListModel::workSuggestionDetail() const
{
    return hasWorkSuggestion() ? m_workSuggestions.at(m_workSuggestionIndex).detail : QString();
}

QString CategoryListModel::workSuggestionCategoryId() const
{
    return hasWorkSuggestion()
        ? m_workSuggestions.at(m_workSuggestionIndex).categoryId
        : QString();
}

QString CategoryListModel::workSuggestionSubcategoryId() const
{
    return hasWorkSuggestion()
        ? m_workSuggestions.at(m_workSuggestionIndex).subcategoryId
        : QString();
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

bool CategoryListModel::addSubcategory(
    int categoryRow,
    const QString &name,
    const QString &notes)
{
    if (categoryRow < 0 || categoryRow >= m_categories.size()) {
        setStatusMessage(QStringLiteral("Choose a parent category first."));
        return false;
    }
    const QString cleanName = name.trimmed();
    const QString cleanNotes = notes.trimmed();
    if (cleanName.isEmpty()) {
        setStatusMessage(QStringLiteral("A subcategory needs a name."));
        return false;
    }
    if (cleanName.size() > 120 || cleanNotes.size() > 10000) {
        setStatusMessage(QStringLiteral("The subcategory name or notes are too long."));
        return false;
    }
    if (hasDuplicateSubcategoryName(categoryRow, cleanName, -1)) {
        setStatusMessage(QStringLiteral("That category already has this subcategory."));
        return false;
    }

    Subcategory subcategory;
    subcategory.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    subcategory.categoryId = m_categories.at(categoryRow).id;
    subcategory.name = cleanName;
    subcategory.notes = cleanNotes;
    subcategory.createdAt = QDateTime::currentDateTime();
    QString errorMessage;
    if (!m_repository.addSubcategory(subcategory, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not save the subcategory: %1").arg(errorMessage));
        return false;
    }
    reload();
    setStatusMessage(QStringLiteral("Subcategory created."));
    return true;
}

bool CategoryListModel::updateSubcategory(
    int categoryRow,
    int subcategoryRow,
    const QString &name,
    const QString &notes)
{
    if (categoryRow < 0 || categoryRow >= m_categories.size()
        || subcategoryRow < 0
        || subcategoryRow >= m_categories.at(categoryRow).subcategories.size()) {
        setStatusMessage(QStringLiteral("That subcategory is no longer in the list."));
        return false;
    }
    const QString cleanName = name.trimmed();
    const QString cleanNotes = notes.trimmed();
    if (cleanName.isEmpty()) {
        setStatusMessage(QStringLiteral("A subcategory needs a name."));
        return false;
    }
    if (cleanName.size() > 120 || cleanNotes.size() > 10000) {
        setStatusMessage(QStringLiteral("The subcategory name or notes are too long."));
        return false;
    }
    if (hasDuplicateSubcategoryName(categoryRow, cleanName, subcategoryRow)) {
        setStatusMessage(QStringLiteral("That category already has this subcategory."));
        return false;
    }

    Subcategory subcategory = m_categories.at(categoryRow).subcategories.at(subcategoryRow);
    subcategory.name = cleanName;
    subcategory.notes = cleanNotes;
    QString errorMessage;
    if (!m_repository.updateSubcategory(subcategory, &errorMessage)) {
        setStatusMessage(QStringLiteral("Could not update the subcategory: %1").arg(errorMessage));
        return false;
    }
    reload();
    setStatusMessage(QStringLiteral("Subcategory updated."));
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

QString CategoryListModel::categoryIdForAssignment(int row) const
{
    int current = 0;
    for (const Category &category : m_categories) {
        if (current == row) {
            return category.id;
        }
        ++current;
        for (qsizetype index = 0; index < category.subcategories.size(); ++index) {
            if (current == row) {
                return category.id;
            }
            ++current;
        }
    }
    return {};
}

QString CategoryListModel::subcategoryIdForAssignment(int row) const
{
    int current = 0;
    for (const Category &category : m_categories) {
        if (current == row) {
            return {};
        }
        ++current;
        for (const Subcategory &subcategory : category.subcategories) {
            if (current == row) {
                return subcategory.id;
            }
            ++current;
        }
    }
    return {};
}

int CategoryListModel::indexOfAssignment(
    const QString &categoryId,
    const QString &subcategoryId) const
{
    int current = 0;
    for (const Category &category : m_categories) {
        if (category.id == categoryId && subcategoryId.isEmpty()) {
            return current;
        }
        ++current;
        for (const Subcategory &subcategory : category.subcategories) {
            if (category.id == categoryId && subcategory.id == subcategoryId) {
                return current;
            }
            ++current;
        }
    }
    return -1;
}

QVariantList CategoryListModel::subcategoriesAt(int categoryRow) const
{
    QVariantList result;
    if (categoryRow < 0 || categoryRow >= m_categories.size()) {
        return result;
    }
    const QVector<Subcategory> &subcategories = m_categories.at(categoryRow).subcategories;
    result.reserve(subcategories.size());
    for (const Subcategory &subcategory : subcategories) {
        result.append(QVariantMap {
            {QStringLiteral("subcategoryId"), subcategory.id},
            {QStringLiteral("name"), subcategory.name},
            {QStringLiteral("notes"), subcategory.notes},
            {QStringLiteral("taskCount"), subcategory.taskCount},
        });
    }
    return result;
}

void CategoryListModel::advanceWorkSuggestion()
{
    if (m_workSuggestions.size() < 2) {
        return;
    }
    m_workSuggestionIndex = (m_workSuggestionIndex + 1) % m_workSuggestions.size();
    emit workSuggestionChanged();
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
    rebuildWorkSuggestions();
    emit categoriesChanged();
    if (!errorMessage.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not load categories: %1").arg(errorMessage));
    }
}

void CategoryListModel::rebuildWorkSuggestions()
{
    QString errorMessage;
    const QVector<Task> openTasks = m_repository.openTasks(&errorMessage);
    QHash<QString, int> categoryCounts;
    QHash<QString, int> subcategoryCounts;
    for (const Task &task : openTasks) {
        if (!task.categoryId.isEmpty()) {
            ++categoryCounts[task.categoryId];
        }
        if (!task.subcategoryId.isEmpty()) {
            ++subcategoryCounts[task.subcategoryId];
        }
    }

    QVector<WorkSuggestion> suggestions;
    for (const Category &category : m_categories) {
        int nestedTaskCount = 0;
        for (const Subcategory &subcategory : category.subcategories) {
            const int openTaskCount = subcategoryCounts.value(subcategory.id);
            nestedTaskCount += openTaskCount;
            if (openTaskCount > 0) {
                suggestions.append({
                    QStringLiteral("%1 / %2").arg(category.name, subcategory.name),
                    QStringLiteral("%1 open %2 in this area.")
                        .arg(openTaskCount)
                        .arg(openTaskCount == 1 ? QStringLiteral("task") : QStringLiteral("tasks")),
                    category.id,
                    subcategory.id,
                    openTaskCount,
                });
            }
        }

        const int directTaskCount = categoryCounts.value(category.id) - nestedTaskCount;
        if (directTaskCount > 0) {
            const int openTaskCount = directTaskCount;
            suggestions.append({
                category.name,
                QStringLiteral("%1 open %2 in this area.")
                    .arg(openTaskCount)
                    .arg(openTaskCount == 1 ? QStringLiteral("task") : QStringLiteral("tasks")),
                category.id,
                {},
                openTaskCount,
            });
        }
    }

    if (suggestions.isEmpty()) {
        for (const Category &category : m_categories) {
            if (category.subcategories.isEmpty()) {
                suggestions.append({
                    category.name,
                    category.notes.isEmpty()
                        ? QStringLiteral("Create a concrete next action in this area.")
                        : category.notes,
                    category.id,
                    {},
                    0,
                });
                continue;
            }
            for (const Subcategory &subcategory : category.subcategories) {
                suggestions.append({
                    QStringLiteral("%1 / %2").arg(category.name, subcategory.name),
                    subcategory.notes.isEmpty()
                        ? QStringLiteral("Create a concrete next action in this area.")
                        : subcategory.notes,
                    category.id,
                    subcategory.id,
                    0,
                });
            }
        }
    }

    std::stable_sort(
        suggestions.begin(),
        suggestions.end(),
        [](const WorkSuggestion &left, const WorkSuggestion &right) {
            if (left.openTaskCount != right.openTaskCount) {
                return left.openTaskCount > right.openTaskCount;
            }
            return left.name.compare(right.name, Qt::CaseInsensitive) < 0;
        });
    m_workSuggestions = std::move(suggestions);
    m_workSuggestionIndex = 0;
    emit workSuggestionChanged();

    if (!errorMessage.isEmpty()) {
        setStatusMessage(QStringLiteral("Could not load category suggestions: %1").arg(errorMessage));
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

bool CategoryListModel::hasDuplicateSubcategoryName(
    int categoryRow,
    const QString &name,
    int excludedRow) const
{
    if (categoryRow < 0 || categoryRow >= m_categories.size()) {
        return false;
    }
    const QVector<Subcategory> &subcategories = m_categories.at(categoryRow).subcategories;
    for (qsizetype index = 0; index < subcategories.size(); ++index) {
        if (index != excludedRow
            && subcategories.at(index).name.compare(name, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}
