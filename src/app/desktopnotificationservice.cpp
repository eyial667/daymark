// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/desktopnotificationservice.h"

#include <QIcon>

#if defined(Q_OS_LINUX)
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QVariantMap>
#endif

DesktopNotificationService::DesktopNotificationService(
    const QIcon &icon,
    QObject *parent)
    : QObject(parent)
    , m_trayIcon(icon, this)
{
    m_trayIcon.setToolTip(tr("Daymark meeting reminders"));
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_trayIcon.show();
    }
}

void DesktopNotificationService::showNotification(
    const QString &title,
    const QString &body)
{
#if defined(Q_OS_LINUX)
    if (QDBusConnection::sessionBus().isConnected()) {
        QDBusMessage message = QDBusMessage::createMethodCall(
            QStringLiteral("org.freedesktop.Notifications"),
            QStringLiteral("/org/freedesktop/Notifications"),
            QStringLiteral("org.freedesktop.Notifications"),
            QStringLiteral("Notify"));
        message << QStringLiteral("Daymark")
                << uint(0)
                << QStringLiteral("org.daymark.dashboard")
                << title
                << body
                << QStringList()
                << QVariantMap()
                << 10000;
        QDBusConnection::sessionBus().asyncCall(message);
        return;
    }
#endif

    if (m_trayIcon.isVisible() && QSystemTrayIcon::supportsMessages()) {
        m_trayIcon.showMessage(
            title,
            body,
            QSystemTrayIcon::Information,
            10000);
    }
}
