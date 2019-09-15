/** @file
 * String routines.
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

#include "StringOps.h"
#include "Assert.h"
#include "Errors.h"
#include "InputOutput.h"
#include "Types.h"

#include <ctype.h>
#include <math.h>
#include <stdint.h>

typedef struct UINTEGER64
{
    uint32_t    u32Hi;
    uint32_t    u32Lo;
} UINTEGER64;
typedef UINTEGER64 *PUINTEGER64;
typedef const UINTEGER64 *PCUINTEGER64;


/**
 * Allocates memory and zeros before returning.
 *
 * @return  Pointer to the allocated, zerod memory, or NULL if no memory available.
 * @param   cb  Number of bytes to allocate.
 */
void *MemAllocZ(uint32_t cb)
{
    void *pv = MemAlloc(cb);
    if (pv)
        MemSet(pv, 0, cb);
    return pv;
}


/**
 * Safe copy a string.
 *
 * @return  RINF_SUCCESS on success or appropriate status code.
 * @param   pszDst  Pointer to the destination string.
 * @param   cbDst   Size of the destination string including NULL terminator.
 * @param   pszSrc  Pointer to the source string.
 */
int StrCopy(char *pszDst, uint32_t cbDst, const char *pszSrc)
{
    uint32_t const cchSrc = StrLen(pszSrc);
    if (cchSrc < cbDst)
    {
        memcpy(pszDst, pszSrc, cchSrc + 1);
        return RINF_SUCCESS;
    }

    if (cbDst)
    {
        memcpy(pszDst, pszSrc, cbDst - 1);
        pszDst[cbDst - 1] = '\0';
    }
    return RERR_BUFFER_OVERFLOW;
}


/**
 * Convert a 32-bit number to its binary form returned in a string.
 *
 * @return  The number converted to binary.
 * @param   uValue          The number to convert to binary.
 * @param   fNegative       Whether it's a negative number or not.
 * @param   fDoubleSpace    Whether an extra space per binary digit is required.
 * @param   fFullLength     Whether padding to the full 32-bit length is required.
 * @param   pcDigits        Where to store the number of binary digits (including
 *                          padding if specified in @a fFullLength).
 */
char *StrValue32AsBinary(U64INTEGER uValue, bool fNegative, bool fDoubleSpace, bool fFullLength, uint32_t *pcDigits)
{
    char *pszBuf = StrAlloc(65 + 20);    /* UINT64_MAX = 64 chars */
    if (!pszBuf)
        return NULL;

    /* Pretty basic, worry about efficiency later, just get smth done */
    size_t i = 0;
    size_t c = 0;
    do
    {
        pszBuf[i++] = '0' + (uValue % 2);
        uValue /= 2;
        ++c;
        if (fDoubleSpace)
            pszBuf[i++] = ' ';
        if ((c % 4) == 0)
            pszBuf[i++] = ' ';
    } while (uValue > 0);

    if (   fFullLength
        && c < 32)
    {
        while (c < 32)
        {
            if (  !(c % 4)
                && pszBuf[i - 1] != ' ')
            {
                pszBuf[i++] = ' ';
            }
            pszBuf[i++] = '0';
            ++c;
        }
        pszBuf[i++] = ' ';
    }

    if (*pcDigits)
        *pcDigits = c - 1;

    pszBuf[i++] = fNegative ? '-' : ' ';
    pszBuf[i++] = '\0';

    size_t cBuf = i - 1; /* StrLen(pszBuf); */
    for (size_t k = 0; k < cBuf / 2; k++)
    {
        char cTmp = pszBuf[cBuf - k - 1];
        pszBuf[cBuf - k - 1] = pszBuf[k];
        pszBuf[k] = cTmp;
    }
    return pszBuf;
}


/**
 * Duplicate a string.
 *
 * @return  A newly allocated, NULL terminated string copy, or NULL if no memory.
 * @param   pszSrc  Pointer to the source string to duplicate.
 */
char *StrDup(const char *pszSrc)
{
    size_t cbSrc = StrLen(pszSrc);
    char *pszDst = (char *)StrAlloc(cbSrc + 1);
    if (!pszDst)
        return NULL;
    StrCopy(pszDst, cbSrc + 1, pszSrc);
    return pszDst;
}


/**
 * Strip a string of leading and trailing white spaces.
 *
 * @return  Pointer to stripped string.
 * @param   pszBuf  The string to be stripped.
 */
char *StrStrip(char *pszBuf)
{
    /* leading white space */
    while (isspace(*pszBuf))
        pszBuf++;

    /* trailing white space */
    char *pszEnd = strchr(pszBuf, '\0');
    while (--pszEnd > pszBuf && isspace(*pszEnd))
        *pszEnd = '\0';

    return pszBuf;
}


/**
 * Strip a string of trailing new line (\n)
 *
 * @return  Pointer to stripped string.
 * @param   pszBuf      The string to be stripped.
 * @param   pfStripped  Where to store if the line was stripped or not, can be NULL.
 */
char *StrStripLF(char *pszBuf, bool *pfStripped)
{
    if (pfStripped)
        *pfStripped = false;
    if (pszBuf)
    {
        size_t cbBuf = strlen(pszBuf);
        if (   cbBuf > 1
            && pszBuf[cbBuf - 1] == '\n')
        {
            pszBuf[cbBuf - 1] = '\0';
            if (pfStripped)
                *pfStripped = true;
        }
    }
    return pszBuf;
}


/**
 * Convert a FLOAT into string equivalent in the specified radix.
 *
 * @return  Appropriate status code, RERR_NOT_SUPPORTED for unsupported radices.
 * @param   pszDst      Pointer to the destination string.
 * @param   cbDst       Size of the destination string including NULL terminator.
 * @param   dValue      The value to convert.
 * @param   uiRadix     Which base should be used during conversion.
 * @param   iWidth      What width to format to.
 * @param   fFlags      Formatting flags (see header).
 */
int StrFormat(char *pszDst, size_t cbDst, FLOAT dValue, unsigned int uiRadix, unsigned int iWidth, unsigned int fFlags)
{
    AssertReturn(pszDst, RERR_INVALID_PARAMETER);
    AssertReturn(cbDst, RERR_INVALID_PARAMETER);

    /* Move this to header */
    if (fFlags & FSTR_ZERO_PAD_SPLIT_EIGHTS)
        fFlags |= FSTR_ZERO_PAD;

    /*
     * Buffer overflow & sanity checks.
     */
    if (uiRadix < 2 || uiRadix > 16)
        return RERR_NOT_SUPPORTED;

    if (iWidth < 0)
        return RERR_NOT_SUPPORTED;

    int cPrefix = 0;
    bool fPrefix = false;
    if (fFlags & FSTR_APPEND_PREFIX)
    {
        fPrefix = true;
        cPrefix = 2; //uiRadix == 16 ? 2 /* Hex '0x' */: 1 /* Oct '0' */;
    }
    size_t cbMinWidth = iWidth + cPrefix + 2;    /* 2 characters, '-' and '\0' */
    if (cbMinWidth > cbDst)
        return RERR_BUFFER_OVERFLOW;

    FLOAT dAbsValue = FABSFLOAT(dValue);
    bool fNegative = DefinitelyLessThan(dValue, (FLOAT)0);
    bool fOverflow = DefinitelyGreaterThan(dAbsValue, (FLOAT)UINT64_MAX);
    if (fFlags & FSTR_VALUE_32_BIT)
        fOverflow = DefinitelyGreaterThan(dAbsValue, (FLOAT)UINT32_MAX);

    /*
     * Negative number.
     */
    uint64_t u64Value = fOverflow ? UINT64_MAX + 1 : (uint64_t)dAbsValue;
    if (   fNegative
        && iWidth > 0)
    {
        if (!(fFlags & FSTR_VALUE_SIGNED))
        {
            u64Value = ~u64Value + 1;
            fNegative = false;

            /*
             * It's late, and I'm sure I've gone mad.
             */
            if (!(fFlags & FSTR_VALUE_32_BIT))
                u64Value = UINT64_MAX + 1 + u64Value;
        }
    }

    if (fFlags & FSTR_VALUE_32_BIT)
        u64Value = fOverflow ? UINT32_MAX : (uint32_t)u64Value;

    /*
     * Calculate the size of the number.
     */
    unsigned cBuffer = 0;
    uint64_t u64Tmp = u64Value;
    do
    {
        u64Tmp /= uiRadix;
        ++cBuffer;
    } while (u64Tmp);

    cbMinWidth += cBuffer;
    if (cbMinWidth > cbDst)
        return RERR_BUFFER_OVERFLOW;

    int rc = RINF_SUCCESS;
    if (   iWidth > 0
        && cBuffer > iWidth)
    {
        /* Hmpf. */
        rc = RWRN_TRUNCATED;
        cBuffer = iWidth;
        u64Value = ~0UL;
    }

    if (!(fFlags & FSTR_VALUE_32_BIT))
        DEBUGPRINTF(("cBuffer=%u %x %x\n", cBuffer, ~2, u64Value));
    
     /* Binary, Octal, Decimal, Hex (2 digits each at uiRadix index) */
    static const char s_szUpperPrefixes[] = "!!  !!!! 0  !!!!0x";
    static const char s_szLowerPrefixes[] = "!!  !!!! 0  !!!!0x";

    /*
     * Append '-' sign for negative number, this is tricky.
     * If padding with zero we need the '-' sign up front,
     * but padding with spaces then we need it at the back.
     */
    int i = 0;
    if (iWidth > cBuffer)
    {
        /*
         * Fixed width.
         */
        if (fFlags & FSTR_ZERO_PAD)
        {
            if (fNegative)
            {
                if (uiRadix == 16)
                    pszDst[i++] = '-';
                else if (uiRadix == 8)
                {
                    pszDst[i++] = ' ';
                    pszDst[i++] = '-';
                }
            }
            else
                pszDst[i++] = ' ';

            if (fPrefix)
            {
                if (fFlags & FSTR_UPPERCASE)
                {
                    if (!(fNegative && uiRadix == 8))
                        pszDst[i++] = s_szUpperPrefixes[uiRadix];
                    pszDst[i++] = s_szUpperPrefixes[uiRadix + 1];
                }
                else
                {
                    if (!(fNegative && uiRadix == 8))
                        pszDst[i++] = s_szLowerPrefixes[uiRadix];
                    pszDst[i++] = s_szLowerPrefixes[uiRadix + 1];
                }
            }
        }
        else
             pszDst[i++] = ' ';
    }
    else
    {
        /*
         * Variable width.
         */
        if (fNegative)
            pszDst[i++] = '-';
        else
            pszDst[i++] = ' ';

        if (fPrefix)
        {
            pszDst[i++] = '0';
            if (uiRadix == 16)
                pszDst[i++] = (fFlags & FSTR_UPPERCASE) ? 'X' : 'x';
        }        
    }

    /*
     * Pad to width if requested.
     */
    if (iWidth > cBuffer)
    {
        char chPad = (fFlags & FSTR_ZERO_PAD) ? '0' : ' ';
        for (unsigned k = 0; k < iWidth; k++)
        {
            pszDst[i++] = chPad;
        }
    }

    if (   iWidth > cBuffer
        && !(fFlags & FSTR_ZERO_PAD))
    {
        /*
         * Fixed width, without zero padding.
         */
        if (fPrefix)
        {
            i -= cPrefix;
            if (fNegative)
            {
                pszDst[--i] = '-';
                ++i;
            }
            pszDst[i++] = '0';
            if (uiRadix == 16)
                pszDst[i++] = (fFlags & FSTR_UPPERCASE) ? 'X' : 'x';
            i += cPrefix;
        }
        else
        {
            /*
             * Fixed width, without prefix.
             */
            if (fNegative)
            {
                pszDst[--i] = '-';
                ++i;
            }
        }
    }

    /*
     * Finally convert.
     */
    if (iWidth > cBuffer)
        pszDst += iWidth + 1 /* '-' */ + cPrefix;
    else
    {
        pszDst += cBuffer;
        pszDst += i;
    }

    const char *pachBaseDigits = (fFlags & FSTR_UPPERCASE) ? "0123456789ABCDEF" : "0123456789abcdef";
    i = -1;
    do
    {
        pszDst[i--] = pachBaseDigits[u64Value % uiRadix];
        u64Value /= uiRadix;
        if (i == -cBuffer - 1)
            break;
    } while (u64Value);

    *pszDst = '\0';
    return rc;
}


/*
 * StrFormats to support:
 *
 *  Input = -5.0  (double)
 *
 *                        Binary          Octal           Decimal             Hexadecimal
 *  Natural:                 101             05                5                      0x5
 * Signed (32)  :                                                              0xFFFFFFFA
 * Unsigned (32):   
 *
 * Unsigned:
 */
 
