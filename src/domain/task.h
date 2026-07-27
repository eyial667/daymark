// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDateTime>
#include <QString>

struct Task
{
    QString id;
    QString title;
    QString notes;
    QString project; // Legacy portable-data field; no longer exposed in the application.
    QString categoryId;
    QString categoryName;
    QString subcategoryId;
    QString subcategoryName;
    QDateTime dueAt;
    QDateTime createdAt;
    QDateTime completedAt;
    int importance = 3;
    int estimatedMinutes = 30;
    bool completed = false;
};
