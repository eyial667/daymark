// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "domain/task.h"

#include <QDateTime>
#include <QStringList>

struct PriorityResult
{
    int score = 0;
    QStringList reasons;
};

class PriorityEngine
{
public:
    [[nodiscard]] static PriorityResult evaluate(
        const Task &task,
        const QDateTime &now = QDateTime::currentDateTime());
};
