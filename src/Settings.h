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
 * The settings blob.
 */
typedef struct SETTINGS
{
    /** Use xTerm colors in the output. */
    bool            fUseColors;
    /** The input prompt. */
    char           *pszPrompt;
    /** Output Boolean */
    bool            fOutputBaseBool;
    /** Output Decimal */
    bool            fOutputBaseDec;
    /** Output Octal */
    bool            fOutputBaseOct;
    /** Output Hexadecimal */
    bool            fOutputBaseHex;
    /** Output Binary */
    bool            fOutputBaseBin;
} SETTINGS;
/** Pointer to rawb settings. */
typedef SETTINGS *PSETTINGS;
typedef SETTINGS const *PCSETTINGS;

int SettingsCreate(PSETTINGS *ppSettings, PCSETTINGS pFrom);
void SettingsDestroy(PSETTINGS pSettings);
extern const SETTINGS g_FactorySettings;

#endif /* SETTINGS_H___ */

