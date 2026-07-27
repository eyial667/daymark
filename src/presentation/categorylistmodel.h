// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "data/taskrepository.h"

#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

class CategoryListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int categoryCount READ categoryCount NOTIFY categoriesChanged)
    Q_PROPERTY(QStringList names READ names NOTIFY categoriesChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    enum Role {
        CategoryIdRole = Qt::UserRole + 1,
        NameRole,
        NotesRole,
        TaskCountRole
    };
    Q_ENUM(Role)

    explicit CategoryListModel(TaskRepository &repository, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int categoryCount() const;
    [[nodiscard]] QStringList names() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool addCategory(const QString &name, const QString &notes);
    Q_INVOKABLE bool updateCategory(int row, const QString &name, const QString &notes);
    Q_INVOKABLE QString idAt(int row) const;
    Q_INVOKABLE int indexOfId(const QString &categoryId) const;
    Q_INVOKABLE void clearStatus();
    Q_INVOKABLE void reload();

signals:
    void categoriesChanged();
    void statusMessageChanged();

private:
    void setStatusMessage(const QString &message);
    [[nodiscard]] bool hasDuplicateName(const QString &name, int excludedRow) const;

    TaskRepository &m_repository;
    QVector<Category> m_categories;
    QString m_statusMessage;
};
