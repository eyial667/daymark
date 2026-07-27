// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QVector>

struct Milestone
{
    QString id;
    QString goalId;
    QString title;
    QDate targetDate;
    QDateTime createdAt;
    bool completed = false;
};

struct Goal
{
    QString id;
    QString title;
    QString description;
    QDate targetDate;
    QDateTime createdAt;
    bool completed = false;
    QVector<Milestone> milestones;
};
