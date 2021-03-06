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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef _WIN32
# include <readline/readline.h>
# include <readline/history.h>
#else
# include <Windows.h>
#endif

#include "InputOutput.h"
#include "Errors.h"
#include "Colors.h"
#include "StringOps.h"
#include "Magics.h"
#include "Assert.h"
#include "Evaluator.h"
#include "GenericDefs.h"

#ifdef _WIN32
static bool g_fxTermColors = true;
#else
static bool g_fxTermColors = false;
#endif

#ifdef _WIN32
static inline WORD GetOutConsoleAttrs(HANDLE *phConsole)
{
    Assert(phConsole);
    CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

    *phConsole = GetStdHandle(STD_OUTPUT_HANDLE);           /* Get and copy console output handle to caller. */
    GetConsoleScreenBufferInfo(*phConsole, &ConsoleInfo);   /* Get the screen buffer info. */
    return ConsoleInfo.wAttributes;                         /* Return attributes to caller. */
}

static inline WORD GetErrConsoleAttrs(HANDLE *phConsole)
{
    Assert(phConsole);
    CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

    *phConsole = GetStdHandle(STD_ERROR_HANDLE);            /* Get and copy console error handle to caller. */
    GetConsoleScreenBufferInfo(*phConsole, &ConsoleInfo);   /* Get the screen buffer info. */
    return ConsoleInfo.wAttributes;                         /* Return attributes to caller. */
}


static inline void SetConsoleAttrs(HANDLE hConsole, WORD wAttrs)
{
    SetConsoleTextAttribute(hConsole, wAttrs);              /* Restore the console attributes. */
}
#endif

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
        if (g_fxTermColors)
        {
#ifdef _WIN32
            HANDLE     hStdError;
            WORD const wStdErrAttrs = GetErrConsoleAttrs(&hStdError);
            SetConsoleTextAttribute(hStdError, FOREGROUND_RED | FOREGROUND_INTENSITY);
            fprintf(stderr, "Error! %s rc=%s (%d)\n\n", pszBuf, pStatusMsg->pszName, rc);
            SetConsoleAttrs(hStdError,  wStdErrAttrs);
#else
            fprintf(stderr, "%sError!%s %s rc=%s%s%s (%d)\n\n", TCOLOR_BOLD_RED, TCOLOR_RESET, pszBuf,
                        TCOLOR_RED, pStatusMsg->pszName, TCOLOR_RESET, rc);
#endif
        }
        else
            fprintf(stderr, "Error! %s rc=%s (%d)\n\n", pszBuf, pStatusMsg->pszName, rc);
    }
    else
        fprintf(stderr, "Extreme error! Missing pStatusMsg!\n");
}


void ColorPrintf(TEXTCOLOR enmTextColor, char *pszMsg, ...)
{
    va_list FmtArgs;
    char szBuf[2048];

    va_start(FmtArgs, pszMsg);
    vsnprintf(szBuf, sizeof(szBuf) - 1, pszMsg, FmtArgs);
    va_end(FmtArgs);

    bool fNewLine;
    char *pszBuf = StrStripLF(szBuf, &fNewLine);

    if (g_fxTermColors)
    {
#ifdef _WIN32
        static WORD s_wColorCodes[] = { 0,                                                                           /* None */
                                        FOREGROUND_RED                                       | FOREGROUND_INTENSITY, /* Red */
                                        FOREGROUND_GREEN                                     | FOREGROUND_INTENSITY, /* Green */
                                        FOREGROUND_BLUE                                      | FOREGROUND_INTENSITY, /* Blue */
                                        FOREGROUND_RED  | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, /* White */
                                        FOREGROUND_BLUE | FOREGROUND_GREEN                   | FOREGROUND_INTENSITY, /* Cyan */
                                        FOREGROUND_RED  | FOREGROUND_GREEN                   | FOREGROUND_INTENSITY  /* Yellow */
                                        };
        AssertReturnVoid(enmTextColor < R_ARRAY_ELEMENTS(s_wColorCodes));
        DWORD const wColorCode = s_wColorCodes[enmTextColor];

        HANDLE     hStdError;
        WORD const wStdErrAttrs = GetErrConsoleAttrs(&hStdError);
        if (enmTextColor != enmTextColorNone)
            SetConsoleTextAttribute(hStdError, wColorCode);
        fprintf(stdout, "%s%s", pszBuf, fNewLine ? "\n" : "");
        if (enmTextColor != enmTextColorNone)
            SetConsoleAttrs(hStdError,  wStdErrAttrs);
#else
        static const char * const s_apszColorCodes[] = { TCOLOR_RESET,
                                                         TCOLOR_RED,
                                                         TCOLOR_GREEN,
                                                         TCOLOR_BLUE,
                                                         TCOLOR_WHITE,
                                                         TCOLOR_CYAN,
                                                         TCOLOR_YELLOW
                                                         };
        AssertReturnVoid(enmTextColor < R_ARRAY_ELEMENTS(s_apszColorCodes));
        const char *pszColorCode = s_apszColorCodes[enmTextColor];

        fprintf(stdout, "%s%s%s%s",
                g_fxTermColors ? pszColorCode : "",
                pszBuf,
                TCOLOR_RESET,
                fNewLine ? "\n" : "");
#endif
    }
    else
        fprintf(stdout, "%s%s", pszBuf, fNewLine ? "\n" : "");
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
    static uint32_t s_iCommandIndex = 0;
    static uint32_t s_cchCommand    = 0;

    /*
     * Only if this is a new word, initialize the data and length.
     * Otherwise we kept it as static data for incremental searches and
     * avoid length calculation every time. Not a real optimization for
     * the size of our dataset, but meh. whatever works.
     */
    if (!fState)
    {
        s_iCommandIndex = 0;
        s_cchCommand    = StrLen(pszText);
    }

    /* Return the next name which partially matches from the command list. */
    uint32_t iEnd = 0;
    const char *pszCommand = EvaluatorFindFunction(pszText, s_cchCommand, s_iCommandIndex, &iEnd);
    s_iCommandIndex = iEnd;
    if (pszCommand)
        return StrDup(pszCommand);

    /* Nothing found. */
    return NULL;
}


#ifndef _WIN32
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
#endif


int TextLineLibraryInit(const char *pszRCFileName)
{
#ifndef _WIN32
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
# if RL_READLINE_VERSION >= 0x0603
    rl_attempted_completion_function = TextLineCompletor;
# else
    rl_attempted_completion_function = (CPPFunction *)TextLineCompletor;
# endif
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
#ifndef _WIN32
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
#else
    char szTmp[1024];
    fprintf(stdout, "> ");
    fgets(&szTmp[0], sizeof(szTmp), stdin);

    char *pszLine = StrDup(szTmp);
    char *pszStripped = StrStrip(pszLine);
    if (pszStripped && *pszStripped)
    {
        pLine->pszRaw = pszLine;
        pLine->pszData = pszStripped;
        return RINF_SUCCESS;
    }
#endif

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

