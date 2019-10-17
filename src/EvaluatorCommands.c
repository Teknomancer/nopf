/** @file
 * Evaluator Commands.
 */

/*
 * Copyright (C) 2014 Ramshankar (aka Teknomancer)
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
#include "Assert.h"
#include "Evaluator.h"
#include "EvaluatorCommands.h"
#include "Errors.h"
#include "GenericDefs.h"
#include "StringOps.h"
#include "InputOutput.h"

#ifdef _WIN32
# define R_VERTCHAR      0xb3
# define R_HORZCHAR      0xc4
# define R_BENDCHAR      0xc0
# define R_SPACECHAR     ' '
#else
# define R_VERTCHAR      '|'
# define R_HORZCHAR      '-'
# define R_BENDCHAR      '+'
# define R_SPACECHAR     ' '
#endif

/** Minimum width of 'long name' field while formatting registers. */
#define R_LONG_NAME_FIELD_MIN       23UL
/** Minimum width of the 'name' field while formatting registers. */
#define R_NAME_FIELD_MIN            2UL


/**
 * Internal helper for StrFormatReg32() to add a formatted string to a
 * pre-allocated string.
 *
 * The destination string points to the end of the added string!!!
 *
 * @return  Status code on the result of the string addition.
 * @param   pszDst      Pointer to the destination string.
 * @param   cbDst       Size of the destination string.
 * @param   pszLine     Pointer to the source string.
 * @param   cWritten    Where to store the number of characters written.
 */
static inline int StrAddLine(char **pszDst, size_t cbDst, char *pszLine, uint32_t *cWritten)
{
    int cFmt = StrNPrintf(*pszDst, cbDst - *cWritten, "  %s\n", pszLine);
    *cWritten += cFmt;
    if (*cWritten < cbDst - 1)
    {
        *pszDst += cFmt;
        return RINF_SUCCESS;
    }
    else
    {
        DEBUGPRINTF(("Insufficient space for formatting register.\n"));
        return RERR_BUFFER_OVERFLOW;
    }
}


/**
 * Formats a 32-bit registers given its value.
 *
 * @return  An allocated string with the formatted output. The caller must free it
 *          with StrFree().
 * @param   uReg    The 32-bit value of the register.
 * @param   pReg    Pointer to the register descriptor.
 */
static char *StrFormatReg32(uint32_t uReg, PCREGDESC32 pReg)
{
    /*
     * e.g.
     *   1000 0011 1000 0000 0000 0101 0011 0011
     *   ||                         |     |   ||
     *   ||                         |     |   |+----------  PE ( 0)
     *   ||                         |     |   +-----------  MP ( 1)
     *   ||                         |     +---------------  XX ( 4)
     *   ||                         +---------------------  XX ( 9)
     *   |+-----------------------------------------------  CD (30)
     *   +------------------------------------------------- PG (31)
     */
    size_t cbBuf = MAX_COMMAND_RESULT_LENGTH;
    char *pszBuf = StrAlloc(cbBuf);
    if (!pszBuf)
        return NULL;
    memset(pszBuf, 0, cbBuf);

    char *pszTmp   = pszBuf;
    uint32_t cDigits = 0;
    char *pszBinary = StrValue32AsBinary(uReg, false, false, true, &cDigits);
    if (!pszBinary)
    {
        StrFree(pszBuf);
        return NULL;
    }

    uint32_t cWritten = StrNPrintf(pszTmp, cbBuf, "%s\n", pszBinary);
    StrFree(pszBinary);
    if (cWritten >= cbBuf - 1)
    {
        DEBUGPRINTF(("Insufficient space for formatting register. szName=%s\n", pReg->szName));
        return pszBuf;
    }
    pszTmp += cWritten;

    int32_t const cBits = 32;
    char szLine[128];
    memset(szLine, 0, sizeof(szLine));
    int32_t iCol = 0;

    /*
     * First line extra consmetic (see format comment above for the line immediately
     * following the binary output).
     */
    for (int32_t k = cBits - 1; k >= 0; k--)
    {
        if (pReg->paRegBitDesc[k].enmType == enmNormal)
            szLine[iCol++] = R_VERTCHAR;
        else
            szLine[iCol++] = R_SPACECHAR;

        if (!(k % 4))
            szLine[iCol++] = R_SPACECHAR;
    }

    int rc = StrAddLine(&pszTmp,cbBuf, szLine, &cWritten);
    if (RC_FAILURE(rc))
        return pszBuf;

    /*
     * Format the register bits. Iterate over each recognized bit row.
     */
    for (int32_t i = cBits - 1; i >= 0; i--)
    {
        if (pReg->paRegBitDesc[cBits - 1 - i].enmType != enmNormal)
            continue;

        /* Reset iCol and szLine. */
        iCol  = 0;
        memset(szLine, 0, sizeof(szLine));

        /*
         * Iterate over all bits (columns).
         */
        bool    fFoundBit = false;
        int32_t iFoundBit = 0;
        for (int32_t k = cBits - 1; k >= 0; k--)
        {
            if (fFoundBit)
            {
                szLine[iCol++] = R_HORZCHAR;
                if (!(k % 4))
                    szLine[iCol++] = R_HORZCHAR;
            }
            else
            {
                if (pReg->paRegBitDesc[k].enmType == enmNormal)
                {
                    if (k == cBits - 1 - i)
                    {
                        fFoundBit = true;
                        iFoundBit = k;
                        szLine[iCol++] = R_BENDCHAR;
                    }
                    else
                        szLine[iCol++] = R_VERTCHAR;
                }
                else
                    szLine[iCol++] = R_SPACECHAR;

                if (!(k % 4))
                {
                    if (fFoundBit)
                        szLine[iCol++] = R_HORZCHAR;
                    else
                        szLine[iCol++] = R_SPACECHAR;
                }
            }
        }

        /*
         * Append label. e.g "--- PG (31)"
         */
        if (fFoundBit)
        {
            char szBitDesc[sizeof("-- ")+ sizeof(pReg->paRegBitDesc->szName)+ sizeof(" (  )")+ sizeof(" *")+ 2];
            StrNPrintf(szBitDesc, sizeof(szBitDesc), "%c%c %s (%2d) %c", R_HORZCHAR, R_HORZCHAR, pReg->paRegBitDesc[iFoundBit].szName,
                iFoundBit, (uReg & R_BIT(iFoundBit)) ? '*' : ' ');
            StrNCat(szLine, szBitDesc, StrLen(szBitDesc));
        }

        /*
         * Add the formatted line to the output.
         */
        rc = StrAddLine(&pszTmp, cbBuf, szLine, &cWritten);
        if (RC_FAILURE(rc))
            break;
    }
    return pszBuf;
}


/**
 * Displays the bit layout of a 32-bit register.
 *
 * @return  An allocated string with the layout, needs to be freed by caller
 *          with StrFree().
 * @param   pReg    Pointer to the 32-bit register descriptor.
 */
static char *StrFormatRegDesc32(PCREGDESC32 pReg)
{
    size_t cbBuf = MAX_COMMAND_RESULT_LENGTH;
    char *pszBuf = StrAlloc(cbBuf);
    if (!pszBuf)
        return NULL;
    memset(pszBuf, 0, cbBuf);

    int32_t const cBits = 32;

    /* Determine the maximum size of the name and long name field for formatting output (default minimum is 23). */
    int cMinLongNameWidth = R_LONG_NAME_FIELD_MIN;
    int cMinNameWidth     = R_NAME_FIELD_MIN;
    for (int32_t i = 0; i < cBits; i++)
    {
        if (pReg->paRegBitDesc[i].enmType == enmNormal)
        {
            int const cCurLongNameWidth = StrLen(pReg->paRegBitDesc[i].szLongName);
            if (cCurLongNameWidth > cMinLongNameWidth)
                cMinLongNameWidth = cCurLongNameWidth;

            int const cCurNameWidth = StrLen(pReg->paRegBitDesc[i].szName);
            if (cCurNameWidth > cMinNameWidth)
                cMinNameWidth = cCurNameWidth;
        }
    }

    char *pszTmp  = pszBuf;
    unsigned cWritten = 0;  /* StrNPrintf(pszTmp, cbBuf, "%s\n", pReg->szDesc); */
    if (cWritten < cbBuf)
    {
        pszTmp += cWritten;
        for (int32_t i = cBits - 1; i >= 0; i--)
        {
            if (pReg->paRegBitDesc[i].enmType == enmNormal)
            {
                int cFmt = StrNPrintf(pszTmp, cbBuf - cWritten, "%2d %*s  %*s  %s\n", i, cMinNameWidth,
                                      pReg->paRegBitDesc[i].szName, -cMinLongNameWidth, pReg->paRegBitDesc[i].szLongName,
                                      pReg->paRegBitDesc[i].szDesc);
                cWritten += cFmt;
                if (cWritten < cbBuf - 1)
                    pszTmp += cFmt;
                else
                {
                    DEBUGPRINTF(("Insufficient space for formatting register. szName=%s\n", pReg->szName));
                    break;
                }
            }
        }

        /*
         * Add the extra description if any.
         */
        int cAddDesc = StrLen(pReg->szAddDesc) + 2;
        if (   cAddDesc > 0
            && cWritten + cAddDesc < cbBuf)
            StrNCat(pszBuf, pReg->szAddDesc, sizeof(pReg->szAddDesc));
        else
            DEBUGPRINTF(("Insufficient space for formatting register. szName=%s\n", pReg->szName));
    }
    else
        DEBUGPRINTF(("Insufficient space for formatting register. szName=%s\n", pReg->szName));

    return pszBuf;
}


static int FnCr0(PEVALUATOR pEval, PTOKEN pToken, char **ppszResult)
{
    AssertReturn(ppszResult, RERR_INVALID_PARAMETER);
    *ppszResult = NULL;

    static REGBITDESC aBitDesc[32] =
    {
    /* Bit */   /* Type */     /* Name */  /* Long Name */            /* Description */
    /*  0  */   { enmNormal,    "PE",      "Protected mode Enable",   "Enables protected mode when set" },
    /*  1  */   { enmNormal,    "MP",      "Monitor co-processor",    "Controls interaction of WAIT/FWAIT instructions with CR0.TS" },
    /*  2  */   { enmNormal,    "EM",      "Emulation",               "If set, no x87 FPU present" },
    /*  3  */   { enmNormal,    "TS",      "Task Switched",           "Save FPU context on FPU instr. after a task-switch" },
    /*  4  */   { enmNormal,    "ET",      "Extention Type",          "On the 386, specified if the math coprocessor was an 80287 or 80387" },
    /*  5  */   { enmNormal,    "NE",      "Numeric Error",           "Enables x87 FPU error reporting when set, else old PC-style" },
    /*  6  */   { enmRsvd,      "",        "",                        "" },
    /*  7  */   { enmRsvd,      "",        "",                        "" },
    /*  8  */   { enmRsvd,      "",        "",                        "" },
    /*  9  */   { enmRsvd,      "",        "",                        "" },
    /* 10 */    { enmRsvd,      "",        "",                        "" },
    /* 11 */    { enmRsvd,      "",        "",                        "" },
    /* 12 */    { enmRsvd,      "",        "",                        "" },
    /* 13 */    { enmRsvd,      "",        "",                        "" },
    /* 14 */    { enmRsvd,      "",        "",                        "" },
    /* 15 */    { enmRsvd,      "",        "",                        "" },
    /* 16 */    { enmNormal,    "WP",      "Write Protect",           "Determines whether the CPU can write to pages marked read-only" },
    /* 17 */    { enmRsvd,      "",        "",                        "" },
    /* 18 */    { enmNormal,    "AM",      "Alignment Mask",          "Enables alignment check when set (for CPL=3, EFLAGS.AC set)" },
    /* 19 */    { enmRsvd,      "",        "",                        "" },
    /* 20 */    { enmRsvd,      "",        "",                        "" },
    /* 21 */    { enmRsvd,      "",        "",                        "" },
    /* 22 */    { enmRsvd,      "",        "",                        "" },
    /* 23 */    { enmRsvd,      "",        "",                        "" },
    /* 24 */    { enmRsvd,      "",        "",                        "" },
    /* 25 */    { enmRsvd,      "",        "",                        "" },
    /* 26 */    { enmRsvd,      "",        "",                        "" },
    /* 27 */    { enmRsvd,      "",        "",                        "" },
    /* 28 */    { enmRsvd,      "",        "",                        "" },
    /* 29 */    { enmNormal,    "NW",      "Not Write-through",       "Determines write-back or write-through for writes that hit cache" },
    /* 30 */    { enmNormal,    "CD",      "Cache Disable",           "Disables memory caching when set" },
    /* 31 */    { enmNormal,    "PG",      "Paging",                  "Enables paging when set" }
    };

    REGDESC32 Cr0Desc;
    memset(&Cr0Desc, 0, sizeof(Cr0Desc));
    StrCopy(Cr0Desc.szName, sizeof(Cr0Desc.szName), "CR0");
    StrCopy(Cr0Desc.szDesc, sizeof(Cr0Desc.szDesc), "Control Register 0 (Intel x86)");
    Cr0Desc.paRegBitDesc = aBitDesc;

    if (pToken)
    {
        uint64_t const uValue = pToken->u.Number.uValue;
        *ppszResult = StrFormatReg32(uValue, &Cr0Desc);
    }
    else
        *ppszResult = StrFormatRegDesc32(&Cr0Desc);
    return RINF_SUCCESS;
}


static int FnCr4(PEVALUATOR pEval, PTOKEN pToken, char **ppszResult)
{
    AssertReturn(ppszResult, RERR_INVALID_PARAMETER);
    *ppszResult = NULL;

    static REGBITDESC aBitDesc[32] =
    {
    /* Bit */   /* Type */     /* Name */     /* Long Name */                         /* Description */
    /*  0  */   { enmNormal,    "VME",        "Virtual 8086 Mode Extensions",         "Enables support for EFLAGS.VIF in Virtual 8086 mode" },
    /*  1  */   { enmNormal,    "PVI",        "Protected Mode Virtual Interrupts",    "Enables support for EFLAGS.VIF in Protected mode" },
    /*  2  */   { enmNormal,    "TSD",        "Time Stamp Disable",                   "If set, RDTSC can only be executed in ring-0" },
    /*  3  */   { enmNormal,    "DE",         "Debugging Extensions",                 "Enables debug register support for I/O access" },
    /*  4  */   { enmNormal,    "PSE",        "Page Size Extension",                  "If clear, page size is _4K, else _4M (or _2M with PAE)" },
    /*  5  */   { enmNormal,    "PAE",        "Physical Address Extension",           "Enables 36-bit physical addresses from 32-bit virtual addresses" },
    /*  6  */   { enmNormal,    "MCE",        "Machine Check Exceptions",             "Enables Machine Check Exceptions" },
    /*  7  */   { enmNormal,    "PGE",        "Page Global Enable",                   "If set, PDE/PTE can be shared between address spaces" },
    /*  8  */   { enmNormal,    "PCE",        "Page-Monitoring Counter Enable",       "If set, RDPMC can be executed at any ring level, else ring-0" },
    /*  9  */   { enmNormal,    "OSFXSR",     "OS FXSAVE/FXRSTOR Support",            "Enables SSE and fast FPU save/restore instructions" },
    /* 10 */    { enmNormal,    "OSXMMEXCPT", "OS Unmasked SIMD-FPU Exceptions",      "Enables unmasked SSE exceptions" },
    /* 11 */    { enmRsvd,      "",           "",                                     "" },
    /* 12 */    { enmRsvd,      "",           "",                                     "" },
    /* 13 */    { enmNormal,    "VMXE",       "Virtual Machine Extensions Enable",    "Enables VMX operation (Intel only)" },
    /* 14 */    { enmNormal,    "SMXE",       "Safer-Mode Extensions",                "Enables Safer-Mode Extensions (Intel only)" },
    /* 15 */    { enmRsvd,      "",           "",                                     "" },
    /* 16 */    { enmNormal,    "FSGSBASE",   "FSGSBASE Enable",                      "Enables (RD|WR)(FS|GS)BASE instructions (Intel only)" },
    /* 17 */    { enmNormal,    "PCIDE",      "Process-Context IDs Enable",           "Enables per-process IDs for TLBs" },
    /* 18 */    { enmNormal,    "OSXSAVE",    "OS XSAVE Enable",                      "Enables XSAVE instruction and processor extended states" },
    /* 19 */    { enmRsvd,      "",           "",                                     "" },
    /* 20 */    { enmNormal,    "SMEP",       "Supervisor Mode Execution Prevention", "Enables supervisor mode execution prevention" },
    /* 21 */    { enmNormal,    "SMAP",       "Supervisor Mode Access Prevention",    "Enables supervisor mode access prevention" },
    /* 22 */    { enmRsvd,      "",           "",                                     "" },
    /* 23 */    { enmRsvd,      "",           "",                                     "" },
    /* 24 */    { enmRsvd,      "",           "",                                     "" },
    /* 25 */    { enmRsvd,      "",           "",                                     "" },
    /* 26 */    { enmRsvd,      "",           "",                                     "" },
    /* 27 */    { enmRsvd,      "",           "",                                     "" },
    /* 28 */    { enmRsvd,      "",           "",                                     "" },
    /* 29 */    { enmRsvd,      "",           "",                                     "" },
    /* 30 */    { enmRsvd,      "",           "",                                     "" },
    /* 31 */    { enmRsvd,      "",           "",                                     "" },
    };

    REGDESC32 Cr4Desc;
    memset(&Cr4Desc, 0, sizeof(Cr4Desc));
    StrCopy(Cr4Desc.szName, sizeof(Cr4Desc.szName), "CR4");
    StrCopy(Cr4Desc.szDesc, sizeof(Cr4Desc.szDesc), "Control Register 4 (Intel x86)");
    Cr4Desc.paRegBitDesc = aBitDesc;

    if (pToken)
    {
        uint64_t const uValue = pToken->u.Number.uValue;
        *ppszResult = StrFormatReg32(uValue, &Cr4Desc);
    }
    else
        *ppszResult = StrFormatRegDesc32(&Cr4Desc);
    return RINF_SUCCESS;
}


static int FnEflags(PEVALUATOR pEval, PTOKEN pToken, char **ppszResult)
{
    AssertReturn(ppszResult, RERR_INVALID_PARAMETER);
    *ppszResult = NULL;

    static REGBITDESC aBitDesc[32] =
    {
    /* Bit */   /* Type */     /* Name */  /* Long Name */            /* Description */
    /*  0  */   { enmNormal,    "CF",      "Carry flag",              "Carry flag" },
    /*  1  */   { enmRsvd,      "",        "",                        "" },
    /*  2  */   { enmNormal,    "PF",      "Parity flag",             "Parity flag" },
    /*  3  */   { enmRsvd,      "",        "",                        "" },
    /*  4  */   { enmNormal,    "AF",      "Adjust flag",             "Adjust flag" },
    /*  5  */   { enmRsvd,      "",        "",                        "" },
    /*  6  */   { enmNormal,    "ZF",      "Zero flag",               "Zero flag" },
    /*  7  */   { enmNormal,    "SF",      "Sign flag",               "Sign flag" },
    /*  8  */   { enmNormal,    "TF",      "Trap flag",               "Trap flag (single-step)" },
    /*  9  */   { enmNormal,    "IF",      "Interrupt flag",          "If set, interrupts are enabled, else interrupts are disabled" },
    /* 10 */    { enmNormal,    "DF",      "Direction flag",          "Direction flag" },
    /* 11 */    { enmNormal,    "OF",      "Overflow flag",           "Overflow flag" },
    /* 12 */    { enmNormal,    "IOPL",    "IOPL",                    "Input/Output privilege level" },
    /* 13 */    { enmNormal,    "IOPL",    "IOPL",                    "Input/Output privilege level" },
    /* 14 */    { enmNormal,    "NT",      "Nested Task flag",        "Nested Task flag" },
    /* 15 */    { enmRsvdMB1,    "",        "",                       "" },
    /* 16 */    { enmNormal,    "RF",      "Resume flag",             "Resume flag" },
    /* 17 */    { enmNormal,    "VF",      "V86 mode flag",           "Virtual 8086 mode flag" },
    /* 18 */    { enmNormal,    "AC",      "Alignment check",         "Alignment check" },
    /* 19 */    { enmNormal,    "VIF",     "Virtual Interrupt flag",  "Virtual Interrupt flag" },
    /* 20 */    { enmRsvd,      "",        "",                        "" },
    /* 21 */    { enmNormal,    "ID",      "CPUID flag",              "If set, CPUID can be used" },
    /* 22 */    { enmRsvd,      "",        "",                        "" },
    /* 23 */    { enmRsvd,      "",        "",                        "" },
    /* 24 */    { enmRsvd,      "",        "",                        "" },
    /* 25 */    { enmRsvd,      "",        "",                        "" },
    /* 26 */    { enmRsvd,      "",        "",                        "" },
    /* 27 */    { enmRsvd,      "",        "",                        "" },
    /* 28 */    { enmRsvd,      "",        "",                        "" },
    /* 29 */    { enmRsvd,      "",        "",                        "" },
    /* 30 */    { enmRsvd,      "",        "",                        "" },
    /* 31 */    { enmRsvd,      "",        "",                        "" },
    };

    REGDESC32 EflagsDesc;
    memset(&EflagsDesc, 0, sizeof(EflagsDesc));
    StrCopy(EflagsDesc.szName, sizeof(EflagsDesc.szName), "EFLAGS");
    StrCopy(EflagsDesc.szDesc, sizeof(EflagsDesc.szDesc), "EFLAGS register (Intel x86)");
    EflagsDesc.paRegBitDesc = aBitDesc;

    if (pToken)
    {
        uint64_t const uValue = pToken->u.Number.uValue;
        *ppszResult = StrFormatReg32(uValue, &EflagsDesc);
    }
    else
        *ppszResult = StrFormatRegDesc32(&EflagsDesc);
    return RINF_SUCCESS;
}


static int FnEfer(PEVALUATOR pEval, PTOKEN pToken, char **ppszResult)
{
    AssertReturn(ppszResult, RERR_INVALID_PARAMETER);
    *ppszResult = NULL;

    static REGBITDESC aBitDesc[32] =
    {
    /* Bit */   /* Type */     /* Name */  /* Long Name */            /* Description */
    /*  0  */   { enmNormal,    "SCE",     "System Call Ext",         "System Call Extensions" },
    /*  1  */   { enmRsvd,      "",        "",                        "" },
    /*  2  */   { enmRsvd,      "",        "",                        "" },
    /*  3  */   { enmRsvd,      "",        "",                        "" },
    /*  4  */   { enmRsvd,      "",        "",                        "" },
    /*  5  */   { enmRsvd,      "",        "",                        "" },
    /*  6  */   { enmRsvd,      "",        "",                        "" },
    /*  7  */   { enmRsvd,      "",        "",                        "" },
    /*  8  */   { enmNormal,    "LME",     "Long mode enable",        "Long mode enable" },
    /*  7  */   { enmRsvd,      "",        "",                        "" },
    /* 10 */    { enmNormal,    "LMA",     "Long mode active",        "Long mode active" },
    /* 11 */    { enmNormal,    "NXE",     "No-Execute enable",       "No-Execute enable" },
    /* 12 */    { enmNormal,    "SVME",    "SVM enable",              "Secure Virtual Machine enable (AMD only)" },
    /* 13 */    { enmNormal,    "LMSLE",   "LMSL enable",             "Long mode segment limit enable (AMD only)" },
    /* 14 */    { enmNormal,    "FFXSR",   "Fast FXSAVE/FXRSTOR",     "Fast FXSAVE/FXRSTOR" },
    /* 15 */    { enmRsvdMB1,    "TCE",    "Translation Cache Ext",   "Translation Cache Extensions" },
    /* 16 */    { enmRsvd,      "",        "",                        "" },
    /* 17 */    { enmRsvd,      "",        "",                        "" },
    /* 18 */    { enmRsvd,      "",        "",                        "" },
    /* 19 */    { enmRsvd,      "",        "",                        "" },
    /* 20 */    { enmRsvd,      "",        "",                        "" },
    /* 21 */    { enmRsvd,      "",        "",                        "" },
    /* 22 */    { enmRsvd,      "",        "",                        "" },
    /* 23 */    { enmRsvd,      "",        "",                        "" },
    /* 24 */    { enmRsvd,      "",        "",                        "" },
    /* 25 */    { enmRsvd,      "",        "",                        "" },
    /* 26 */    { enmRsvd,      "",        "",                        "" },
    /* 27 */    { enmRsvd,      "",        "",                        "" },
    /* 28 */    { enmRsvd,      "",        "",                        "" },
    /* 29 */    { enmRsvd,      "",        "",                        "" },
    /* 30 */    { enmRsvd,      "",        "",                        "" },
    /* 31 */    { enmRsvd,      "",        "",                        "" },
    };

    REGDESC32 EferDesc;
    memset(&EferDesc, 0, sizeof(EferDesc));
    StrCopy(EferDesc.szName, sizeof(EferDesc.szName), "EFER");
    StrCopy(EferDesc.szDesc, sizeof(EferDesc.szDesc), "Extended Feature Enable register (Intel x86)");
    EferDesc.paRegBitDesc = aBitDesc;

    if (pToken)
    {
        uint64_t const uValue = pToken->u.Number.uValue;
        *ppszResult = StrFormatReg32(uValue, &EferDesc);
    }
    else
        *ppszResult = StrFormatRegDesc32(&EferDesc);
    return RINF_SUCCESS;
}


static int FnCSAttr(PEVALUATOR pEval, PTOKEN pToken, char **ppszResult)
{
    AssertReturn(ppszResult, RERR_INVALID_PARAMETER);
    *ppszResult = NULL;

    static REGBITDESC aBitDesc[32] =
    {
    /* Bit */   /* Type */     /* Name */       /* Long Name */            /* Description */
    /*  0  */   { enmNormal,   "Seg. type",    "Segment type",             "Segment type" },
    /*  1  */   { enmInRange,  "",             "",                         "" },
    /*  2  */   { enmInRange,  "",             "",                         "" },
    /*  3  */   { enmInRange,  "",             "",                         "" },
    /*  4  */   { enmNormal,   "Desc. type",   "Descriptor type",          "Descriptor type (0=system,1=code/data)" },
    /*  5  */   { enmNormal,   "DPL",          "DPL",                      "Descriptor privilege level" },
    /*  6  */   { enmInRange,  "",             "",                         "" },
    /*  7  */   { enmNormal,   "Present",      "Present",                  "If set, segment is present" },
    /*  8  */   { enmNormal,   "Limit",        "Segment limit (hi)",       "Segment limit (high bits 16:19)" },
    /*  7  */   { enmInRange,  "",             "",                         "" },
    /* 10 */    { enmInRange,  "",             "",                         "" },
    /* 11 */    { enmInRange,  "",             "",                         "" },
    /* 12 */    { enmNormal,   "Available",    "Available",                "If set, available for use by system software" },
    /* 13 */    { enmNormal,   "Long",         "Long",                     "64-bit code segment (long mode only)" },
    /* 14 */    { enmNormal,   "DefBig",       "Default/Big",              "Default operation size (0=16-bit, 1=32-bit)" },
    /* 15 */    { enmNormal,   "Gran",         "Granularity",              "Granularity (0=byte scaling,1=4KB scaling)" },
    /* 16 */    { enmNormal,   "Unusable",      "Unusable",                "Unusable segment (Intel VT-x only)" },
    /* 17 */    { enmRsvd,      "",             "",                        "" },
    /* 18 */    { enmRsvd,      "",             "",                        "" },
    /* 19 */    { enmRsvd,      "",             "",                        "" },
    /* 20 */    { enmRsvd,      "",             "",                        "" },
    /* 21 */    { enmRsvd,      "",             "",                        "" },
    /* 22 */    { enmRsvd,      "",             "",                        "" },
    /* 23 */    { enmRsvd,      "",             "",                        "" },
    /* 24 */    { enmRsvd,      "",             "",                        "" },
    /* 25 */    { enmRsvd,      "",             "",                        "" },
    /* 26 */    { enmRsvd,      "",             "",                        "" },
    /* 27 */    { enmRsvd,      "",             "",                        "" },
    /* 28 */    { enmRsvd,      "",             "",                        "" },
    /* 29 */    { enmRsvd,      "",             "",                        "" },
    /* 30 */    { enmRsvd,      "",             "",                        "" },
    /* 31 */    { enmRsvd,      "",             "",                        "" },
    };

    REGDESC32 CSAttrDesc;
    memset(&CSAttrDesc, 0, sizeof(CSAttrDesc));
    StrCopy(CSAttrDesc.szName, sizeof(CSAttrDesc.szName), "CSAttr");
    StrCopy(CSAttrDesc.szDesc, sizeof(CSAttrDesc.szDesc), "Code Segment Attributes (Intel x86)");
    StrCopy(CSAttrDesc.szAddDesc, sizeof(CSAttrDesc.szAddDesc),
            "---------------------------\n"
            "Code and Data Segment Types\n"
            "---------------------------\n"
            "Decimal  Binary   Type    Description\n"
            "0        0 0 0 0  Data    Read-Only\n"
            "1        0 0 0 1  Data    Read-Only,  accessed\n"
            "2        0 0 1 0  Data    Read/Write\n"
            "3        0 0 1 1  Data    Read/Write, accessed\n"
            "4        0 1 0 0  Data    Read-Only,  expand-down\n"
            "5        0 1 0 1  Data    Read-Only,  expand-down, accessed\n"
            "6        0 1 1 0  Data    Read/Write, expand-down\n"
            "7        0 1 1 1  Data    Read/Write, expand-down, accessed\n"
            "8        1 0 0 0  Code    Execute-Only\n"
            "9        1 0 0 1  Code    Execute-Only, accessed\n"
            "10       1 0 1 0  Code    Execute/Read\n"
            "11       1 0 1 1  Code    Execute/Read, accessed\n"
            "12       1 1 0 0  Code    Execute-Only, conforming\n"
            "13       1 1 0 1  Code    Execute-Only, conforming, accessed\n"
            "14       1 1 1 0  Code    Execute/Read, conforming\n"
            "15       1 1 1 1  Code    Execute/Read, conforming, accessed\n");
    CSAttrDesc.paRegBitDesc = aBitDesc;

    if (pToken)
    {
        uint64_t const uValue = pToken->u.Number.uValue;
        *ppszResult = StrFormatReg32(uValue, &CSAttrDesc);
    }
    else
        *ppszResult = StrFormatRegDesc32(&CSAttrDesc);


    return RINF_SUCCESS;
}


/**
 * g_aCommands: Table of commands.
 */
COMMAND g_aCommands[] =
{
    /*  Name                             pfn                   Syntax           Desc.          */
    { "cr0",                            FnCr0,               "<x86reg>",      "Intel x86: CR0 register format." },
    { "cr4",                            FnCr4,               "<x86reg>",      "Intel x86: CR4 register format." },
    { "eflags",                         FnEflags,            "<[e|r]flags>",  "Intel x86: EFLAGS register format." },
    { "efer",                           FnEfer,              "<x86reg>",      "Intel x86: EFER format." },
    { "csattr",                         FnCSAttr,            "<csattr>",      "Intel x86: CS segment attributes." }
};

/** Total number of commands in the table. */
const unsigned g_cCommands = R_ARRAY_ELEMENTS(g_aCommands);

