/* $Id: InputOutput.c 192 2014-05-14 06:52:35Z marshan $ */
/** @file
 * Input/Output routines.
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

#include "InputOutput.h"
#include "Errors.h"
#include "Colors.h"
#include "StringOps.h"
#include "Magics.h"
#include "Assert.h"
#include "Evaluator.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

static bool g_fxTermColors = false;

void ErrorPrintf(int rc, char *pszError, ...)
{
    va_list FmtArgs;
    char szBuf[2048];

    va_start(FmtArgs, pszError);
    vsnprintf(szBuf, sizeof(szBuf) - 1, pszError, FmtArgs);
    va_end(FmtArgs);

    char *pszBuf = StrStripLF(szBuf, NULL /* pfStripped */);

    PCRCSTATUSMSG pStatusMsg = StatusMsgForRC(rc);
    if (pStatusMsg)
    {
        fprintf(stderr, "%sError!%s %s rc=%s%s%s (%d)\n\n", TCOLOR_BOLD_RED, TCOLOR_RESET, pszBuf,
                    TCOLOR_RED, pStatusMsg->pszName, TCOLOR_RESET, rc);
    }
    else
        fprintf(stderr, "Extreme error! Missing pStatusMsg!\n");
}


void ColorPrintf(char *pszColorCode, char *pszMsg, ...)
{
    va_list FmtArgs;
    char szBuf[2048];

    va_start(FmtArgs, pszMsg);
    vsnprintf(szBuf, sizeof(szBuf) - 1, pszMsg, FmtArgs);
    va_end(FmtArgs);

    bool fNewLine;
    char *pszBuf = StrStripLF(szBuf, &fNewLine);

    fprintf(stdout, "%s%s%s%s",
            g_fxTermColors ? pszColorCode : "",
            pszBuf,
            TCOLOR_RESET,
            fNewLine ? "\n" : "");
}


void DebugPrintf(char *pszMsg, ...)
{
    va_list FmtArgs;
    char szBuf[2048];

    va_start(FmtArgs, pszMsg);
    vsnprintf(szBuf, sizeof(szBuf) - 1, pszMsg, FmtArgs);
    va_end(FmtArgs);

    bool fNewLine;
    char *pszBuf = StrStripLF(szBuf, &fNewLine);

    fprintf(stderr, "%s%s%s", fNewLine ? "  dbg:" : "", pszBuf, fNewLine ? "\n" : "");
}


static char *TextLineGenerator(const char *pszText, int fState)
{
    static int s_iCommandIndex = 0;
    static int s_cchCommand = 0;

    /*
     * Only if this is a new word, initialize the data and length.
     * Otherwise we kept it as static data for incremental searches and
     * avoid length calculation every time. Not a real optimization for
     * the size of our dataset, but meh. whatever works.
     */
    if (!fState)
    {
        s_iCommandIndex = 0;
        s_cchCommand = StrLen(pszText);
    }

    /* Return the next name which partially matches from the command list. */
    int iEnd = 0;
    const char *pszCommand = EvaluatorFindFunctor(pszText, s_cchCommand, s_iCommandIndex, &iEnd);
    s_iCommandIndex = iEnd;
    if (pszCommand)
        return StrDup(pszCommand);

    /* Nothing found. */
    return NULL;
}


#if RL_READLINE_VERSION >= 0x0603
static char **TextLineCompletor(const char *pszText, int iStart, int iEnd)
#else
static char **TextLineCompletor(char *pszText, int iStart, int iEnd)
#endif
{
    /*
     * We'd like to complete always, regardless of the iStart/iEnd positions.
     */
    char **ppszMatches = NULL;

#ifndef RL_READLINE_VERSION
    /*
     * Doubt you'd get this far without a readline.
     */
# error You need to install readline library and headers.
#elif RL_READLINE_VERSION <= 0x0402
    /*
     * Darwin.... I really am going to need configure someday.
     * Too lazy to really write one from scratch.
     */
    ppszMatches = completion_matches(pszText, &TextLineGenerator);
#else
    /*
     * Rest of them *nix.
     */
    ppszMatches = rl_completion_matches(pszText, &TextLineGenerator);
#endif

    return ppszMatches;
}


int TextLineLibraryInit(const char *pszRCFileName)
{
    if (pszRCFileName)
    {
        /* Allow conditional parsing of the ~/.nopf file. */
        rl_readline_name = (char *)pszRCFileName;
    }

    /*
     * Color support only for xTerm/xTermc terminal displays.
     */
    char *pszTerm = getenv("TERM");
    if (   pszTerm
        && (    !strcmp(pszTerm, "xterm")
             || !strcmp(pszTerm, "xtermc")
             || !strcmp(pszTerm, "xterm-color")))
    {
        g_fxTermColors = true;
    }

    /* We'd like to hook into completion first. */
#if RL_READLINE_VERSION >= 0x0603
    rl_attempted_completion_function = TextLineCompletor;
#else
    rl_attempted_completion_function = (CPPFunction *)TextLineCompletor;
#endif
    return RINF_SUCCESS;
}


int TextLineRead(PTEXTLINE pLine, char *pszPrompt)
{
    Assert(pLine);
    AssertReturn(pLine->u32Magic == RMAG_TEXTLINE, RERR_BAD_MAGIC);

    TextLineDelete(pLine);

    /*
     * Read a line of input and add it to the history if required.
     */
    char *pszTmp = readline(pszPrompt);
    if (!pszTmp)
        return RERR_NO_DATA;

    char *pszLine = StrDup(pszTmp);
    free(pszTmp);

    char *pszStripped = StrStrip(pszLine);
    if (pszStripped && *pszStripped)
    {
        add_history(pszStripped);
        pLine->pszRaw = pszLine;
        pLine->pszData = pszStripped;
        return RINF_SUCCESS;
    }

    return RERR_NO_DATA;
}


void TextLineInit(PTEXTLINE pLine)
{
    Assert(pLine);

    pLine->u32Magic = RMAG_TEXTLINE;
    pLine->pszRaw = NULL;
    pLine->pszData = NULL;
}


void TextLineDelete(PTEXTLINE pLine)
{
    Assert(pLine);

    pLine->pszData = NULL;
    if (pLine->pszRaw)
    {
        StrFree(pLine->pszRaw);
        pLine->pszRaw = NULL;
    }
}


void TextLineLibraryTerm(void)
{
    /* nothing to do for readline */
}

