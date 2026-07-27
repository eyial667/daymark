// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/priorityengine.h"

#include <algorithm>

PriorityResult PriorityEngine::evaluate(const Task &task, const QDateTime &now)
{
    PriorityResult result;

    const int importance = std::clamp(task.importance, 1, 5);
    result.score = importance * 10;

    if (importance == 5) {
        result.reasons.append(QStringLiteral("Critical importance"));
    } else if (importance == 4) {
        result.reasons.append(QStringLiteral("High importance"));
    } else {
        result.reasons.append(QStringLiteral("Importance %1 of 5").arg(importance));
    }

    if (task.dueAt.isValid()) {
        const qint64 secondsUntilDue = now.secsTo(task.dueAt);
        const int calendarDays = now.date().daysTo(task.dueAt.date());

        if (secondsUntilDue < 0) {
            result.score += 55;
            result.reasons.prepend(QStringLiteral("Overdue"));
        } else if (calendarDays == 0) {
            result.score += 45;
            result.reasons.prepend(QStringLiteral("Due today"));
        } else if (calendarDays == 1) {
            result.score += 28;
            result.reasons.prepend(QStringLiteral("Due tomorrow"));
        } else if (calendarDays <= 3) {
            result.score += 18;
            result.reasons.prepend(QStringLiteral("Due within three days"));
        } else if (calendarDays <= 7) {
            result.score += 8;
            result.reasons.prepend(QStringLiteral("Due this week"));
        }
    }

    if (task.createdAt.isValid()) {
        const qint64 ageInDays = std::max<qint64>(0, task.createdAt.date().daysTo(now.date()));
        result.score += static_cast<int>(std::min<qint64>(ageInDays, 14));
        if (ageInDays >= 3) {
            result.reasons.append(QStringLiteral("Waiting for %1 days").arg(ageInDays));
        }
    }

    if (task.estimatedMinutes <= 25) {
        result.score += 6;
        result.reasons.append(QStringLiteral("Quick to finish"));
    } else if (task.estimatedMinutes <= 60) {
        result.score += 3;
        result.reasons.append(QStringLiteral("Fits a one-hour focus block"));
    }

    return result;
}
