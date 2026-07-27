// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDateTime>
#include <QString>

struct Category
{
    QString id;
    QString name;
    QString notes;
    QDateTime createdAt;
    int taskCount = 0;
};
