// SPDX-License-Identifier: GPL-3.0-or-later

#include "presentation/languagemanager.h"

#include <QCoreApplication>

LanguageManager::LanguageManager()
    : m_originalLocale(QLocale())
{
}

LanguageManager::~LanguageManager()
{
    if (m_installed) {
        QCoreApplication::removeTranslator(&m_translator);
    }
}

bool LanguageManager::apply(AppSettings::Language language)
{
    if (m_installed) {
        QCoreApplication::removeTranslator(&m_translator);
        m_installed = false;
    }

    if (language == AppSettings::English) {
        QLocale::setDefault(QLocale(QLocale::English, m_originalLocale.territory()));
        return true;
    }

    if (!m_translator.load(QStringLiteral(":/i18n/daymark_fr.qm"))) {
        QLocale::setDefault(m_originalLocale);
        return false;
    }
    m_installed = QCoreApplication::installTranslator(&m_translator);
    if (m_installed) {
        QLocale::setDefault(QLocale(QLocale::French, QLocale::France));
    }
    return m_installed;
}
