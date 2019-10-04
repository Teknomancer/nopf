/** @file
 * Main, implementation.
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
*   Header Files                                                               *
*******************************************************************************/
#include "Evaluator.h"
#include "StringOps.h"
#include "Errors.h"
#include "Colors.h"
#include "Assert.h"
#include "Settings.h"
#include "GenericDefs.h"
#include "InputOutput.h"

#include <math.h>

/*******************************************************************************
*   Structures, Typedefs & Defines                                             *
*******************************************************************************/
#define APP_EXECNAME                "nopf"
#define STR_OVERFLOW                "overflow"  /* must be 8 */

#define PREFIX_COLOR                enmTextColorCyan
#define OUTPUT_COLOR                enmTextColorNone
#define PARAM_COLOR                 enmTextColorNone
#define VARS_COLOR                  enmTextColorYellow
#define APP_COPYRIGHT_COLOR         enmTextColorWhite

#define CMD_HELP                    "help"
#define CMD_QUIT                    "quit"
#define CMD_QUIT_SHORT              "q"
#define CMD_EXIT                    "exit"
#define CMD_BYE                     "bye"
#define CMD_VARS                    "vars"


static char *GetValueAsBinaryString(uint64_t uValue, size_t *pcDigits)
{
    char *pszBuf = StrAlloc(  sizeof("1111111111111111111111111111111111111111111111111111111111111111")
                            + 15 /* 1 space between each 4 bits, so for 64-bits => 16 spaces. */
                            + 1  /* 1 for negative space or dash */);
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
        if (   c % 4 == 0
            && uValue > 0)
            pszBuf[i++] = ' ';
    } while (uValue > 0);

    if (*pcDigits)
        *pcDigits = c;

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


static void PrintResult(PCSETTINGS pSettings, PCEVALUATOR pEval)
{
    /*
     * Length required for formatting output.
     *
     * ---------------------------------------------------------
     *  Radix                  Number  Digits    Prefix   Total
     * ---------------------------------------------------------
     * UINT32_MAX:
     *    Dec              4294967295      10                10
     *    Oct             37777777777      11        0       12
     *    Hex                ffffffff       8       0x       10
     * --------------------------------------------------------
     * Maximum                                               12
     *
     * UINT64_MAX:
     *    Dec    18446744073709551615      20                20
     *    Oct  1777777777777777777777      22        0       23
     *    Hex        ffffffffffffffff      16       0x       18
     * --------------------------------------------------------
     * Maximum                                               23
     */
    int const      cIndent0 = 1;       /* Indent for 1st column. */
    int const      cIndent1 = 2;       /* Indent for 2nd column. */
    int const      cIndent2 = 2;       /* Indent for 3rd column. */

    long double const    dResult = pEval->Result.dValue;
    uint64_t const uResult = pEval->Result.uValue;
    if (pSettings->fOutputBaseBool)
    {
        bool const fResult = !!uResult;

        char szDstBool[sizeof("false")];
        StrNPrintf(szDstBool, sizeof(szDstBool), "%s", fResult ? "true" : "false");

        ColorPrintf(PREFIX_COLOR, "Bool:");
        ColorPrintf(OUTPUT_COLOR, "%*s%12s (N)\n", cIndent0, "", szDstBool);
    }

    if (pSettings->fOutputBaseDec)
    {
        char szDst32[sizeof("4294967295")];
        StrNPrintf(szDst32, sizeof(szDst32), "%" FMT_U32_NAT, (uint32_t)uResult);

        char szDst64[sizeof("18446744073709551615")];
        StrNPrintf(szDst64, sizeof(szDst64), "%" FMT_U64_NAT, uResult);

        char szDstFloat[128];
        StrNPrintf(szDstFloat, sizeof(szDstFloat), "%" FMT_FLT_NAT, dResult);

        ColorPrintf(PREFIX_COLOR, "Dec :");
        ColorPrintf(OUTPUT_COLOR, "%*s%12s (U32)%*s%23s (U64)%*s%s (N)\n",
                    cIndent0, "", szDst32,
                    cIndent1, "", szDst64,
                    cIndent2, "", szDstFloat);
    }

    if (pSettings->fOutputBaseHex)
    {
        char szDst32[sizeof("ffffffff") + sizeof("0x")];
        StrNPrintf(szDst32, sizeof(szDst32), "0x%08" FMT_U32_HEX, (uint32_t)uResult);

        char szDst64[sizeof("ffffffffffffffff") + sizeof("0x")];
        StrNPrintf(szDst64, sizeof(szDst64), "0x%016" FMT_U64_HEX, uResult);

        char szDstNat[sizeof("ffffffffffffffff") + sizeof("0x")];
        StrNPrintf(szDstNat, sizeof(szDstNat), "0x%" FMT_U64_HEX, uResult);

        ColorPrintf(PREFIX_COLOR, "Hex :");
        ColorPrintf(OUTPUT_COLOR, "%*s%12s (U32)%*s%23s (U64)%*s%s (N)\n",
                    cIndent0, "", szDst32,
                    cIndent1, "", szDst64,
                    cIndent2, "", szDstNat);
    }

    if (pSettings->fOutputBaseOct)
    {
        char szDst32[sizeof("37777777777") + sizeof("0")];
        StrNPrintf(szDst32, sizeof(szDst32), "0%08" FMT_U32_OCT, (uint32_t)uResult);

        char szDst64[sizeof("1777777777777777777777") + sizeof("0")];
        StrNPrintf(szDst64, sizeof(szDst64), "0%016" FMT_U64_OCT, uResult);

        char szDstNat[sizeof("1777777777777777777777") + sizeof("0")];
        StrNPrintf(szDstNat, sizeof(szDstNat), "0%" FMT_U64_OCT, uResult);

        ColorPrintf(PREFIX_COLOR, "Oct :");
        ColorPrintf(OUTPUT_COLOR, "%*s%12s (U32)%*s%23s (U64)%*s%s (N)\n",
                    cIndent0, "", szDst32,
                    cIndent1, "", szDst64,
                    cIndent2, "", szDstNat);
    }

    if (pSettings->fOutputBaseBin)
    {
        size_t cDigits = 1;
        char *pszBin = GetValueAsBinaryString(uResult, &cDigits);
        ColorPrintf(PREFIX_COLOR, "Bin :");
        ColorPrintf(OUTPUT_COLOR, "%*s%s (%u)\n", cIndent0, "", pszBin ? pszBin : " N/A", (unsigned)cDigits);
        if (pszBin)
            StrFree(pszBin);
    }

    Printf("\n");
}


static void PrintHelp(PSETTINGS pSettings)
{
    NOREF(pSettings);

#define INDENT_SPACES   ""

    ColorPrintf(APP_COPYRIGHT_COLOR, "%s (c) 2012 Ramshankar (aka Teknomancer)\n", APP_EXECNAME);
    Printf("Compiled: %s\n", __TIMESTAMP__);
    Printf("  %s is a command-line expression evaluator that started as sunny day project.\n", APP_EXECNAME);
    Printf("  %s is free software, published under the GNU Public License (GPLv3).\n", APP_EXECNAME);
    Printf("  For feedback and patches, e-mail:  <v.ramshankar(at)gmail.com>.\n\n");

    Printf(INDENT_SPACES "Operators:\n");

    unsigned cOperators = EvaluatorOperatorCount();
    for (unsigned i = 0; i < cOperators; i++)
    {
        char *pszOperator = NULL;
        char *pszSyntax = NULL;
        char *pszDesc = NULL;
        int const cIndent = 5;
        int rc = EvaluatorOperatorHelp(i, &pszOperator, &pszSyntax, &pszDesc);
        if (RC_SUCCESS(rc))
        {
            ColorPrintf(PREFIX_COLOR, "%10s%*s", pszOperator, cIndent, "");
            ColorPrintf(PARAM_COLOR,  "%-20s ", pszSyntax);
            Printf("%s\n", pszDesc);
        }
        StrFree(pszOperator);
        StrFree(pszSyntax);
        StrFree(pszDesc);
    }

    Printf("\n");
    Printf(INDENT_SPACES "Functions:\n");

    /*
     * Display help for each Function with some attempt at pretty formatting.
     */
    unsigned cFunctions = EvaluatorFunctionCount();
    for (unsigned i = 0; i < cFunctions; i++)
    {
        char *pszFunction = NULL;
        char *pszSyntax = NULL;
        char *pszDesc = NULL;

        int rc = EvaluatorFunctionHelp(i, &pszFunction, &pszSyntax, &pszDesc);
        if (RC_SUCCESS(rc))
        {
            size_t cbSyntaxFmted = StrLen(pszSyntax) + sizeof("()");
            char *pszSyntaxFmted = MemAlloc(cbSyntaxFmted);
            if (cbSyntaxFmted > sizeof("()"))
            {
                if (!pszSyntaxFmted)
                {
                    Printf("Out of memory\n");
                    return;
                }

                StrNPrintf(pszSyntaxFmted, cbSyntaxFmted, "(%s)", pszSyntax);
            }
            else
                StrCopy(pszSyntaxFmted, sizeof(pszSyntaxFmted), "");

            ColorPrintf(PREFIX_COLOR, "%11s ", pszFunction);
            ColorPrintf(PARAM_COLOR, "%-20s ", pszSyntaxFmted);
            Printf("%s\n", pszDesc);
        }
        StrFree(pszFunction);
        StrFree(pszSyntax);
        StrFree(pszDesc);
    }

    const char * const pszIndent = " ";
    const int          cIndent   = -10;
    /* Legend. */
    Printf("\n");
    {
        Printf("Legend:\n");
        ColorPrintf(PREFIX_COLOR, "%s%*s", pszIndent, cIndent, "<numx>");
        Printf("A number (internally represented as both integer and floating point).\n");
        Printf("%s%*sAny Function/Operator taking <numX> will compute both integer and float.\n", pszIndent, cIndent, "");
        Printf("%s%*sSome Functions/Operators make compute float and cast to integer.\n", pszIndent, cIndent, "");

        ColorPrintf(PREFIX_COLOR, "%s%*s", pszIndent, cIndent, "<intX>");
        Printf("An integer.\n");
        Printf("%s%*sAny Function/Operator taking <intX> will compute integer and cast to float.\n", pszIndent, cIndent, "");

        ColorPrintf(PREFIX_COLOR, "%s%*s", pszIndent, cIndent, "<cond>");
        Printf("A condition or truth value.\n", pszIndent, cIndent, "");
        Printf("%s%*sA logical condition, with \"true\" being 0 and \"false\" being non-zero.\n", pszIndent, cIndent, "");

        ColorPrintf(PREFIX_COLOR, "%s%*s", pszIndent, cIndent, "<expr>");
        Printf("An expression such as \"5 + 4 * 3.6\" or \"42 / sum(1024,128,512)\"\n");
        Printf("%s%*sExpressions always result in a <numX>\n", pszIndent, cIndent, "");
    }

    /* Number prefixes. */
    Printf("\n");
    {
        Printf("Number prefixes:\n");
        ColorPrintf(PREFIX_COLOR, "%s%*s", pszIndent, cIndent, "[n|N]");
        Printf("Binary (e.g. \"n111\" is 7).\n");

        ColorPrintf(PREFIX_COLOR, "%s%*s", pszIndent, cIndent, "[0]");
        Printf("Octal (e.g. \"033\" is 27).\n");

        ColorPrintf(PREFIX_COLOR, "%s%*s", pszIndent, cIndent, "[0x|0X]");
        Printf("Hexadecimal (e.g. \"b111\" is 45329).\n");
    }

    Printf("\n");

#undef INDENT_SPACES
}


static void PrintVars(PSETTINGS pSettings)
{
    NOREF(pSettings);

    for (uint32_t i = 0; /* forever */; i++)
    {
        char *pszVar = NULL;
        char *pszExpr = NULL;
        int rc = EvaluatorVariableValue(i, &pszVar, &pszExpr);
        if (RC_SUCCESS(rc))
        {
            char *pszExprTrimmed = StrStrip(pszExpr);

            ColorPrintf(VARS_COLOR, "%24s", pszVar);
            Printf(" = ");
            ColorPrintf(OUTPUT_COLOR, "%s\n", pszExprTrimmed);

            StrFree(pszVar);
            StrFree(pszExpr);
        }
        else
        {
            Assert(!pszVar);
            Assert(!pszExpr);
            break;
        }
    }

    Printf("\n");
}


static void PrintVarAssigned(PSETTINGS pSettings, PCEVALUATOR pEval)
{
    ColorPrintf(PREFIX_COLOR, "Stored variable:");
    ColorPrintf(OUTPUT_COLOR, " '%s'\n", pEval->Result.szVariable);
    Printf("\n");
}

static void PrintCommandEvaluated(PSETTINGS pSettings, PCEVALUATOR pEval)
{
    ColorPrintf(PREFIX_COLOR, "%s:\n", pEval->Result.szCommand);
    ColorPrintf(OUTPUT_COLOR, "%s\n",  pEval->Result.szCommandResult);
}


static int ProcessExpression(PSETTINGS pSettings, PEVALUATOR pEval, const char *pszExpr)
{
    bool fParsed = false;
    int rc = EvaluatorParse(pEval, pszExpr);
    if (RC_SUCCESS(rc))
    {
        fParsed = true;

        /*
         * For expression assignment we must not evaluate the expression. It's just
         * an assignment, syntactically correct.
         */
        if (pEval->Result.fVariableAssignment == false)
            rc = EvaluatorEvaluate(pEval);
    }

    if (RC_SUCCESS(rc))
    {
        if (pEval->Result.fVariableAssignment)
            PrintVarAssigned(pSettings, pEval);
        else if (pEval->Result.fCommandEvaluated)
            PrintCommandEvaluated(pSettings, pEval);
        else
            PrintResult(pSettings, pEval);
    }
    else
    {
        char szComponent[128];
        StrNPrintf(szComponent, sizeof(szComponent), "%s:", fParsed ? "Evaluator" : "Parser");
        switch (rc)
        {
            case RERR_EXPRESSION_INVALID:       ErrorPrintf(rc, "%s Invalid expression.\n", szComponent); break;
            case RERR_TOO_FEW_PARAMETERS:       ErrorPrintf(rc, "%s Not enough parameters to operator/function.\n", szComponent); break;
            case RERR_TOO_MANY_PARAMETERS:      ErrorPrintf(rc, "%s Too many parameters to operator/function.\n", szComponent); break;
            case RERR_NO_MEMORY:                ErrorPrintf(rc, "%s Out of memory.\n", szComponent); break;
            case RERR_UNDEFINED_BEHAVIOUR:      ErrorPrintf(rc, "%s Pesky overflow, calculation hindered.\n", szComponent); break;
            case RERR_VARIABLE_UNDEFINED:       ErrorPrintf(rc, "%s Variable '%s' undefined.\n", szComponent, pEval->Result.szVariable); break;
            case RERR_CIRCULAR_DEPENDENCY:      ErrorPrintf(rc, "%s Circular dependency for variable '%s'.\n", szComponent, pEval->Result.szVariable); break;
            case RERR_INVALID_ASSIGNMENT:       ErrorPrintf(rc, "%s Cannot assign expression to non-lvalue.\n", szComponent); break;
            case RERR_VARIABLE_CANNOT_REASSIGN: ErrorPrintf(rc, "%s Cannot re-assign variable '%s'.\n", szComponent, pEval->Result.szVariable); break;
            default:                            ErrorPrintf(rc, "%s Undefined error.\n", szComponent); break;
        }
    }

    return rc;
}


/**
 * And so it begins...
 */
int main(int cArgs, char *aszArgs[])
{
    /** @todo getopt .. sigh that's GNU again */

    PSETTINGS pSettings = NULL;
    int rc = SettingsCreate(&pSettings, &g_FactorySettings);
    if (RC_FAILURE(rc))
    {
        ErrorPrintf(rc, "Failed to create settings\n");
        return rc;
    }

    char szErrorBuf[1024];
    MemSet(szErrorBuf, 0, sizeof(szErrorBuf));

    rc = EvaluatorInitGlobals();
    if (RC_FAILURE(rc))
    {
        ErrorPrintf(rc, "Failed to initialize evaluator globals\n");
        return rc;
    }

    EVALUATOR Eval;
    rc = EvaluatorInit(&Eval, szErrorBuf, sizeof(szErrorBuf) - 1);
    if (RC_FAILURE(rc))
    {
        ErrorPrintf(rc, "Failed to initialize evaluator:\n\t%s\n", szErrorBuf);
        return rc;
    }

    TextLineLibraryInit("~/." APP_EXECNAME);
    if (cArgs > 1)
    {
        ProcessExpression(pSettings, &Eval, aszArgs[1]);
        goto the_end;
    }

    TEXTLINE Line;
    TextLineInit(&Line);

    for (;;)
    {
        rc = TextLineRead(&Line, pSettings->pszPrompt);
        if (RC_SUCCESS(rc))
        {
            if (   !StrCmp(Line.pszData, CMD_QUIT)
                || !StrCmp(Line.pszData, CMD_QUIT_SHORT)
                || !StrCmp(Line.pszData, CMD_EXIT))
            {
                break;
            }

            if (!StrCmp(Line.pszData, CMD_BYE))
            {
                Printf("Buh bye :)\n");
                break;
            }

            if (!StrCmp(Line.pszData, CMD_VARS))
            {
                PrintVars(pSettings);
                continue;
            }

            if (!StrCmp(Line.pszData, CMD_HELP))
            {
                PrintHelp(pSettings);
                continue;
            }

            ProcessExpression(pSettings, &Eval, Line.pszData);
        }
    }

    TextLineDelete(&Line);
    TextLineLibraryTerm();

the_end:
    EvaluatorDestroy(&Eval);
    EvaluatorDestroyGlobals();
    SettingsDestroy(pSettings);

    return rc;
}

