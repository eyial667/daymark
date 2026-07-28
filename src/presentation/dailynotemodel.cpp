// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/dailynotemodel.h"

#include "domain/dailynote.h"

#include <QLocale>

DailyNoteModel::DailyNoteModel(TaskRepository &repository, QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(650);
    connect(&m_saveTimer, &QTimer::timeout, this, [this] { persist(false); });
    refreshDay();
}

QString DailyNoteModel::todayText() const
{
    return m_todayText;
}

void DailyNoteModel::setTodayText(const QString &text)
{
    if (m_todayText == text) {
        return;
    }
    m_todayText = text;
    setDirty(true);
    m_saveTimer.start();
    emit todayTextChanged();
}

QString DailyNoteModel::previousText() const
{
    return m_previousText;
}

QString DailyNoteModel::previousDayLabel() const
{
    return QLocale().toString(m_currentDate.addDays(-1), tr("dddd, MMMM d"));
}

bool DailyNoteModel::hasPreviousNote() const
{
    return !m_previousText.trimmed().isEmpty();
}

bool DailyNoteModel::dirty() const
{
    return m_dirty;
}

QString DailyNoteModel::statusMessage() const
{
    return m_statusMessage;
}

bool DailyNoteModel::saveNow()
{
    m_saveTimer.stop();
    return persist(true);
}

bool DailyNoteModel::deleteToday()
{
    m_saveTimer.stop();
    QString errorMessage;
    const QDate date = m_currentDate.isValid() ? m_currentDate : QDate::currentDate();
    if (!m_repository.deleteDailyNote(date, &errorMessage)) {
        setStatusMessage(tr("Could not delete the day note: %1").arg(errorMessage));
        return false;
    }
    m_todayText.clear();
    setDirty(false);
    emit todayTextChanged();
    setStatusMessage(tr("Today's note deleted."));
    return true;
}

void DailyNoteModel::refreshDay()
{
    if (m_dirty && !persist(false)) {
        return;
    }

    const QDate today = QDate::currentDate();
    QString errorMessage;
    if (!m_repository.pruneDailyNotes(today.addDays(-1), &errorMessage)) {
        setStatusMessage(tr("Could not prune old day notes: %1").arg(errorMessage));
        return;
    }

    const auto todayNote = m_repository.dailyNote(today, &errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(tr("Could not load today's note: %1").arg(errorMessage));
        return;
    }
    const auto previousNote = m_repository.dailyNote(today.addDays(-1), &errorMessage);
    if (!errorMessage.isEmpty()) {
        setStatusMessage(tr("Could not load yesterday's note: %1").arg(errorMessage));
        return;
    }

    m_currentDate = today;
    m_todayText = todayNote ? todayNote->text : QString();
    m_previousText = previousNote ? previousNote->text : QString();
    setDirty(false);
    emit todayTextChanged();
    emit previousNoteChanged();
}

void DailyNoteModel::clearStatus()
{
    setStatusMessage({});
}

bool DailyNoteModel::persist(bool announce)
{
    if (!m_currentDate.isValid()) {
        m_currentDate = QDate::currentDate();
    }
    if (!m_dirty) {
        if (announce) {
            setStatusMessage(tr("Day note is already saved."));
        }
        return true;
    }

    DailyNote note;
    note.date = m_currentDate;
    note.text = m_todayText;
    note.updatedAt = QDateTime::currentDateTime();
    QString errorMessage;
    if (!m_repository.saveDailyNote(note, &errorMessage)) {
        setStatusMessage(tr("Could not save the day note: %1").arg(errorMessage));
        return false;
    }
    setDirty(false);
    if (announce) {
        setStatusMessage(tr("Day note saved."));
    }
    return true;
}

void DailyNoteModel::setDirty(bool dirty)
{
    if (m_dirty == dirty) {
        return;
    }
    m_dirty = dirty;
    emit dirtyChanged();
}

void DailyNoteModel::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }
    m_statusMessage = message;
    emit statusMessageChanged();
}
