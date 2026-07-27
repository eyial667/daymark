// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDate>
#include <QDateTime>
#include <QString>

struct DailyNote
{
    QDate date;
    QString text;
    QDateTime updatedAt;
};
