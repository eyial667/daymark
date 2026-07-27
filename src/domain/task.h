// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDateTime>
#include <QString>

struct Task
{
    QString id;
    QString title;
    QString notes;
    QString project;
    QDateTime dueAt;
    QDateTime createdAt;
    int importance = 3;
    int estimatedMinutes = 30;
    bool completed = false;
};
