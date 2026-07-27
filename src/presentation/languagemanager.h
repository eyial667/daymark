// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "presentation/appsettings.h"

#include <QLocale>
#include <QTranslator>

class LanguageManager final
{
public:
    LanguageManager();
    ~LanguageManager();

    bool apply(AppSettings::Language language);

private:
    QLocale m_originalLocale;
    QTranslator m_translator;
    bool m_installed = false;
};
