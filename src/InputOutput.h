/** @file
 * Input/Output routines header.
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

#ifndef INPUT_OUTPUT_H___
#define INPUT_OUTPUT_H___

#include <stdint.h>
#ifdef _WIN32
/* Breaks because minwindef.h defines an ATOM type. WTF!? Ugh, no namespaces in C either and no real way to redefine a typedef. */
/* # include <Windows.h> */
#endif

#define Printf printf
void ErrorPrintf(int rc, char *pszError, ...);
void ColorPrintf(char *pszColorCode, char *pszMsg, ...);
void DebugPrintf(char *pszMsg, ...);

#ifdef _DEBUG
#define DEBUGPRINTF(s)         DebugPrintf s
#else
#define DEBUGPRINTF(s)         do { } while (0)
#endif

/**
 * Text Line record.
 */
typedef struct TEXTLINE
{
    uint32_t                     u32Magic;   /**< Magic. */
    char                        *pszRaw;     /**< The complete/original buffer line */
    char                        *pszData;    /**< The line stripped/altered, a subset of 'pszRaw' */
#if 0
#ifdef _WIN32
    HANDLE                      hConsole;    /**< Console handle. */
    CONSOLE_SCREEN_BUFFER_INFO  ConsoleInfo; /**< Console screen buffer info. */
#endif
#endif
} TEXTLINE;
/** Pointer to a line record. */
typedef TEXTLINE *PTEXTLINE;
/** Pointer to a const line record. */
typedef const TEXTLINE *PCTEXTLINE;

/** TextLine data structure manipulators. */
void    TextLineInit(PTEXTLINE pLine);
void    TextLineDelete(PTEXTLINE pLine);

/** TextLine library routines */
int     TextLineLibraryInit(const char *pszRCFileName);
void    TextLineLibraryTerm(void);
int     TextLineRead(PTEXTLINE pLine, char *pszPrompt);

#endif /* INPUT_OUTPUT_H___ */

