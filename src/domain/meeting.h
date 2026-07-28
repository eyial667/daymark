// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDateTime>
#include <QString>

struct Meeting
{
    QString id;
    QString title;
    QString notes;
    QDateTime startsAt;
    QDateTime createdAt;
    QDateTime notifiedAt;
};
