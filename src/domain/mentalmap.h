// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

struct MentalMapGroup
{
    QString id;
    QString kind;
    QString title;
    QString color;
    double x = 0.0;
    double y = 0.0;
    double width = 360.0;
    double height = 260.0;
    QDateTime createdAt;
};

struct MentalMapChecklistItem
{
    QString text;
    bool completed = false;
};

struct MentalMapNote
{
    QString id;
    QString groupId;
    QString groupKind;
    QString title;
    QString body;
    QString color;
    QString tags;
    QString externalLink;
    int priority = 0;
    double x = 0.0;
    double y = 0.0;
    bool center = false;
    QString linkedTaskId;
    QString linkedTaskTitle;
    bool linkedTaskCompleted = false;
    QVector<MentalMapChecklistItem> checklist;
    QDateTime createdAt;
};

struct MentalMapConnection
{
    QString id;
    QString viewKind;
    QString sourceNoteId;
    QString targetNoteId;
    QString label;
    QDateTime createdAt;
};
