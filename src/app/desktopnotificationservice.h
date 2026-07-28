// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSystemTrayIcon>

class QIcon;

class DesktopNotificationService final : public QObject
{
    Q_OBJECT

public:
    explicit DesktopNotificationService(
        const QIcon &icon,
        QObject *parent = nullptr);

public slots:
    void showNotification(const QString &title, const QString &body);

private:
    QSystemTrayIcon m_trayIcon;
};
