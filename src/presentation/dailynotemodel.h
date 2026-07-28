// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "data/taskrepository.h"

#include <QObject>
#include <QTimer>

class DailyNoteModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString todayText READ todayText WRITE setTodayText NOTIFY todayTextChanged)
    Q_PROPERTY(QString previousText READ previousText NOTIFY previousNoteChanged)
    Q_PROPERTY(QString previousDayLabel READ previousDayLabel NOTIFY previousNoteChanged)
    Q_PROPERTY(bool hasPreviousNote READ hasPreviousNote NOTIFY previousNoteChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit DailyNoteModel(TaskRepository &repository, QObject *parent = nullptr);

    [[nodiscard]] QString todayText() const;
    void setTodayText(const QString &text);
    [[nodiscard]] QString previousText() const;
    [[nodiscard]] QString previousDayLabel() const;
    [[nodiscard]] bool hasPreviousNote() const;
    [[nodiscard]] bool dirty() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool saveNow();
    Q_INVOKABLE bool deleteToday();
    Q_INVOKABLE void refreshDay();
    Q_INVOKABLE void clearStatus();

signals:
    void todayTextChanged();
    void previousNoteChanged();
    void dirtyChanged();
    void statusMessageChanged();

private:
    bool persist(bool announce);
    void setDirty(bool dirty);
    void setStatusMessage(const QString &message);

    TaskRepository &m_repository;
    QTimer m_saveTimer;
    QDate m_currentDate;
    QString m_todayText;
    QString m_previousText;
    bool m_dirty = false;
    QString m_statusMessage;
};
