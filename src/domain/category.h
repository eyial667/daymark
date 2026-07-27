// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

struct Subcategory
{
    QString id;
    QString categoryId;
    QString name;
    QString notes;
    QDateTime createdAt;
    int taskCount = 0;
};

struct Category
{
    QString id;
    QString name;
    QString notes;
    QDateTime createdAt;
    int taskCount = 0;
    QVector<Subcategory> subcategories;
};
