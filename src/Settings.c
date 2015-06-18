/* $Id: Settings.c 181 2012-04-28 14:54:11Z marshan $ */
/** @file
 * Program settings, command line options etc.
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

#include "Settings.h"
#include "StringOps.h"
#include "Errors.h"
#include "Assert.h"

/** The global factory (default) settings */
SETTINGS const g_FactorySettings =
{
    /* .fUseColors = */                         true,
    /* .pszPrompt = */                          ">",
    /* .fOutputBaseBool = */                    true,
    /* .fOutputBaseDec = */                     true,
    /* .fOutputBaseOct = */                     true,
    /* .fOutputBaseHex = */                     true,
    /* .fOutputBaseBin = */                     true
};


/**
 * Creates a new settings blob, cloning @a pSource.
 *
 * @returns RC status code.
 * @param   ppSettings          Where to store the new settings blob.
 * @param   pFrom               The settings to clone.
 */
int SettingsCreate(PSETTINGS *ppSettings, PCSETTINGS pSource)
{
    AssertReturn(pSource, RERR_INVALID_PARAMETER);
    AssertReturn(ppSettings, RERR_INVALID_PARAMETER);

    *ppSettings = NULL;
    PSETTINGS pSettings = MemAllocZ(sizeof(*pSettings));
    if (!pSettings)
        return RERR_NO_MEMORY;

    pSettings->pszPrompt = StrDup(pSource->pszPrompt);
    if (!pSettings->pszPrompt)
    {
        StrFree(pSettings);
        return RERR_NO_MEMORY;
    }

    pSettings->fUseColors      = pSource->fUseColors;
    pSettings->fOutputBaseBool = pSource->fOutputBaseBin;
    pSettings->fOutputBaseDec  = pSource->fOutputBaseDec;
    pSettings->fOutputBaseOct  = pSource->fOutputBaseOct;
    pSettings->fOutputBaseHex  = pSource->fOutputBaseHex;
    pSettings->fOutputBaseBin  = pSource->fOutputBaseBin;

    *ppSettings = pSettings;
    return RINF_SUCCESS;
}


/**
 * Destroys a settings structure.
 *
 * @param   pSettings           The settgins structure to destroy.
 */
void SettingsDestroy(PSETTINGS pSettings)
{
    AssertReturnVoid(pSettings);

    if (pSettings->pszPrompt)
    {
        StrFree(pSettings->pszPrompt);
        pSettings->pszPrompt = NULL;
    }

    MemFree(pSettings);
}

