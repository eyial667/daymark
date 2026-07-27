// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "data/taskrepository.h"

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class MentalMapModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList hierarchy READ hierarchy NOTIFY mapChanged)
    Q_PROPERTY(QVariantList flatNodes READ flatNodes NOTIFY mapChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY mapChanged)
    Q_PROPERTY(int revision READ revision NOTIFY mapChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit MentalMapModel(TaskRepository &repository, QObject *parent = nullptr);

    [[nodiscard]] QVariantList hierarchy() const;
    [[nodiscard]] QVariantList flatNodes() const;
    [[nodiscard]] int itemCount() const;
    [[nodiscard]] int revision() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void clearStatus();

signals:
    void mapChanged();
    void statusMessageChanged();

private:
    static QVariantMap makeNode(
        const QString &id,
        const QString &title,
        const QString &kind,
        const QString &detail,
        int colorIndex,
        const QString &categoryId = {},
        const QString &subcategoryId = {},
        const QVariantList &children = {});
    static QString taskDetail(const Task &task);
    static QString targetDetail(const QDate &targetDate, const QString &fallback);
    void appendFlatNode(
        const QVariantMap &node,
        const QString &parentId,
        int depth,
        int branchIndex,
        int branchCount,
        int siblingIndex,
        int siblingCount,
        double angle,
        double sectorWidth);
    void setStatusMessage(const QString &message);

    TaskRepository &m_repository;
    QVariantList m_hierarchy;
    QVariantList m_flatNodes;
    int m_revision = 0;
    QString m_statusMessage;
};
