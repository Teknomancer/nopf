/** @file
 * String operations header.
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

#ifndef STRING_OPS_H___
#define STRING_OPS_H___

#include <string.h>
#ifndef _WIN32
#include <strings.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Types.h"

/*
 * Just for having capitalized names, replacing such allocs isn't
 * going to be really need for such a small app. Even if it's we can
 * use electric fence libraries.
 */
#define MemAlloc            malloc
#define MemFree             free
#define StrAlloc            malloc
#define StrFree             free

#define MemCpy              memcpy
#define MemCmp              memcmp
#define MemSet              memset
#define StrCmp              strcmp
#define StrNCmp             strncmp
#ifdef _WIN32
#define StrNCaseCmp         _strnicmp
#else
#define StrNCaseCmp         strncasecmp
#endif
#define StrNPrintf          snprintf
#define StrCat              strcat
#define StrNCat             strncat
#define StrLen              strlen
#define MemZero(s)          (memset((s), 0, sizeof((s))))

void   *MemAllocZ(uint32_t cb);
char   *StrDup(const char *pszSrc);
int     StrCopy(char *pszDst, uint32_t cbDst, const char *pszSrc);
char   *StrStrip(char *pszBuf);
char   *StrStripLF(char *pszBuf, bool *pfStripped);
char   *StrValue32AsBinary(uint64_t uValue, bool fNegative, bool fDoubleSpace, bool fFullLength, uint32_t *pcDigits);

/*
 * String flags for StrFormat.
 */
#define FSTR_ZERO_PAD                   0x01
#define FSTR_APPEND_PREFIX              0x02
#define FSTR_UPPERCASE                  0x04
#define FSTR_ZERO_PAD_SPLIT_EIGHTS      0x08
#define FSTR_VALUE_SIGNED               0x10
#define FSTR_VALUE_32_BIT               0x20

int StrFormat(char *pszDst, size_t cbDst, long double dValue, unsigned int uiRadix, unsigned int iWidth, unsigned int fFlags);

#endif /* STRING_OPS_H___ */

