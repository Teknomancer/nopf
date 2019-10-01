/** @file
 * Program settings, command line options etc., header.
 */

/*
 * Copyright (C) 2011 Ramshankar (aka Teknomancer)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SETTINGS_H___
#define SETTINGS_H___

#include <stdbool.h>

/**
 * The settings object.
 */
typedef struct SETTINGS
{
    bool            fUseColors;         /**< Whether to use xTerm colors in the output. */
    char           *pszPrompt;          /**< The input prompt. */
    bool            fOutputBaseBool;    /**< Whether to output Boolean. */
    bool            fOutputBaseDec;     /**< Whether to output Decimal. */
    bool            fOutputBaseOct;     /**< Whether to output Octal. */
    bool            fOutputBaseHex;     /**< Whether to output Hexadecimal. */
    bool            fOutputBaseBin;     /**< Whether to output Binary. */
} SETTINGS;
typedef SETTINGS *PSETTINGS;
typedef SETTINGS const *PCSETTINGS;

int     SettingsCreate(PSETTINGS *ppSettings, PCSETTINGS pFrom);
void    SettingsDestroy(PSETTINGS pSettings);

extern const SETTINGS g_FactorySettings;

#endif /* SETTINGS_H___ */

