/** @file
 * Error routines.
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

/*******************************************************************************
 *   Header Files                                                              *
 *******************************************************************************/
#include "Errors.h"
#include "GenericDefs.h"

#include <stdlib.h>

/*******************************************************************************
 *   Globals, Typedefs & Defines                                               *
 *******************************************************************************/
const RCSTATUSMSG g_UnknownMsg =
{
    "Unknown status.", 1
};

const RCSTATUSMSG g_aStatusMsgs[] =
{
#ifndef _WIN32
#include "GenErrorData.h"
#endif
    { NULL, 0 }
};


/**
 * Finds the status message object for an error code.
 *
 * @return  Pointer to the matching Status Message object.
 * @param   rc  The error code to find.
 */
PCRCSTATUSMSG StatusMsgForRC(int rc)
{
    for (unsigned i = 0; i < R_ARRAY_ELEMENTS(g_aStatusMsgs); i++)
    {
        if (g_aStatusMsgs[i].rc == rc)
            return &g_aStatusMsgs[i];
    }

    return &g_UnknownMsg;
}

