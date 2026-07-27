// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/languagemanager.h"

#include <QCoreApplication>
#include <QtTest>

class LanguageManagerTest final : public QObject
{
    Q_OBJECT

private slots:
    void switchesBetweenBundledLanguages();
};

void LanguageManagerTest::switchesBetweenBundledLanguages()
{
    LanguageManager manager;

    QVERIFY(manager.apply(AppSettings::French));
    QCOMPARE(
        QCoreApplication::translate("Sidebar", "Today"),
        QStringLiteral("Aujourd’hui"));
    QCOMPARE(
        QCoreApplication::translate("PriorityEngine", "Critical importance"),
        QStringLiteral("Importance critique"));
    QCOMPARE(
        QCoreApplication::translate("UpdateService", "Open installation wizard"),
        QStringLiteral("Ouvrir l’assistant d’installation"));

    QVERIFY(manager.apply(AppSettings::English));
    QCOMPARE(
        QCoreApplication::translate("Sidebar", "Today"),
        QStringLiteral("Today"));
}

QTEST_GUILESS_MAIN(LanguageManagerTest)

#include "languagemanager_test.moc"
