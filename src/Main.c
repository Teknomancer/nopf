/* $Id: Main.c 192 2014-05-14 06:52:35Z marshan $ */
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
#include "StringOps.h"
#include "Errors.h"
#include "Colors.h"
#include "Assert.h"
#include "Settings.h"
#include "InputOutput.h"
#include "Evaluator.h"
#include "GenericDefs.h"

#include <math.h>

/*******************************************************************************
*   Structures, Typedefs & Defines                                             *
*******************************************************************************/
#define APP_EXECNAME                "nopf"
#define STR_OVERFLOW                "overflow"  /* must be 8 */

#define PREFIX_COLOR                TCOLOR_CYAN
#define PARAM_COLOR                 TCOLOR_YELLOW
#define FUNC_COLOR                  TCOLOR_BLUE

#define CMD_HELP                    "help"
#define CMD_QUIT                    "quit"
#define CMD_QUIT_SHORT              "q"
#define CMD_EXIT                    "exit"
#define CMD_BYE                     "bye"
#define CMD_VARS                    "vars"


static char *ValueAsBinary(FLOAT dValue, bool fNegative, size_t *pcDigits)
{
    FLOAT dAbsResult = FABSFLOAT(dValue);
    if (!CanCastTo(dAbsResult, MAX_U64INTEGER))
        return NULL;

    U64INTEGER uValue = (U64INTEGER)dAbsResult;
    char *pszBuf = StrAlloc(65 + 20);    /* UINT64_MAX = 1111111111111111111111111111111111111111111111111111111111111111 = 64 chars */
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
        if (c % 4 == 0)
            pszBuf[i++] = ' ';
    } while (uValue > 0);

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

static void PrintResult(PCSETTINGS pSettings, FLOAT dResult)
{
    bool fNegative = DefinitelyLessThan(dResult, (FLOAT)0);
    FLOAT dAbsResult = FABSFLOAT(dResult);
    char szDstNat[512];
    char szDst32[sizeof(STR_OVERFLOW) * 3];
    char szDst64[sizeof(STR_OVERFLOW) * 3];
    StrNPrintf(szDst64, sizeof(szDst64), STR_OVERFLOW STR_OVERFLOW);
    StrNPrintf(szDst32, sizeof(szDst32), STR_OVERFLOW STR_OVERFLOW);
    StrNPrintf(szDstNat, sizeof(szDstNat), STR_OVERFLOW);

    bool fCastToU64 = CanCastTo(dResult, MAX_U64INTEGER);
    bool fCastToU32 = CanCastTo(dResult, MAX_U32INTEGER);

    /*
     * Output in various radices.
     */
    if (pSettings->fOutputBaseBool)
    {
        if (fCastToU32 && !fNegative)
        {
            if (EssentiallyEqual(dResult, 1.0))
                StrNPrintf(szDstNat, sizeof(szDstNat), "%11s", "true");
            else if (EssentiallyEqual(dResult, 0.0))
                StrNPrintf(szDstNat, sizeof(szDstNat), "%11s", "false");
            else
                StrNPrintf(szDstNat, sizeof(szDstNat), "%11s", "NAB");
        }
        else
            StrNPrintf(szDstNat, sizeof(szDstNat), "%11s", "NAB");

        ColorPrintf(PREFIX_COLOR, "Bool:");
        ColorPrintf(PARAM_COLOR, "  %s (N)\n", szDstNat);
    }

    if (pSettings->fOutputBaseDec)
    {
        if (fCastToU64) /* UINT64_MAX = 18446744073709551615 = 20 chars*/
            StrNPrintf(szDst64, sizeof(szDst64), "%20" FMTU64INTEGER_NAT, (U64INTEGER)dResult);

        if (fCastToU32) /* UINT32_MAX = 4294967295 = 10 chars */
            StrNPrintf(szDst32, sizeof(szDst32), "%10" FMTU32INTEGER_NAT, (U32INTEGER)dResult);

        if (fNegative)
            StrNPrintf(szDstNat, sizeof(szDstNat), "-%" FMTFLOAT, dAbsResult);
        else
            StrNPrintf(szDstNat, sizeof(szDstNat), " %" FMTFLOAT, dResult);

        ColorPrintf(PREFIX_COLOR, "Dec :");
        ColorPrintf(PARAM_COLOR,  "   %.10s (U32)  %.20s (U64)  %s (N)\n", szDst32, szDst64, szDstNat);
    }

    if (pSettings->fOutputBaseHex)
    {
        if (fCastToU64) /* UINT64_MAX = fffffffffffffffff = 16 chars */
        {
            StrNPrintf(szDst64, sizeof(szDst64), "%016" FMTU64INTEGER_HEX, (U64INTEGER)dResult);
            if (fNegative)
                StrNPrintf(szDstNat, sizeof(szDstNat), "-0x%0" FMTU64INTEGER_HEX, (U64INTEGER)dAbsResult);
            else
                StrNPrintf(szDstNat, sizeof(szDstNat), " 0x%0" FMTU64INTEGER_HEX, (U64INTEGER)dResult);
        }

        if (fCastToU32) /* UINT32_MAX = ffffffff = 8 chars */
            StrNPrintf(szDst32, sizeof(szDst32), "%08" FMTU32INTEGER_HEX, (U32INTEGER)dResult);

        ColorPrintf(PREFIX_COLOR, "Hex :");
        ColorPrintf(PARAM_COLOR,  "   0x%.8s (U32)    0x%.16s (U64)  %s (N)\n", szDst32, szDst64, szDstNat);
    }

    if (pSettings->fOutputBaseOct)
    {
        if (fCastToU64)
        {
            StrNPrintf(szDst64, sizeof(szDst64), "%018" FMTU64INTEGER_OCT, (U64INTEGER)dResult);
            if (fNegative)
                StrNPrintf(szDstNat, sizeof(szDstNat), "-0%0" FMTU64INTEGER_OCT, (U64INTEGER)dAbsResult);
            else
                StrNPrintf(szDstNat, sizeof(szDstNat), " 0%0" FMTU64INTEGER_OCT, (U64INTEGER)dResult);
        }

        if (fCastToU32) /* UINT32_MAX = 37777777777 = 11 chars */
            StrNPrintf(szDst32, sizeof(szDst32), "%011" FMTU32INTEGER_OCT, (U32INTEGER)dResult);

        ColorPrintf(PREFIX_COLOR, "Oct :");
        ColorPrintf(PARAM_COLOR, " 0%.11s (U32)   0%.18s (U64)  %s (N)\n", szDst32, szDst64, szDstNat);
    }

    if (pSettings->fOutputBaseBin)
    {
        size_t cDigits = 1;
        char *pszBin = ValueAsBinary(dResult, fNegative, &cDigits);
        ColorPrintf(PREFIX_COLOR, "Bin :");
        ColorPrintf(PARAM_COLOR, " %s (%d)\n", pszBin ? pszBin : " NA", cDigits);
        if (pszBin)
            StrFree(pszBin);
    }

    Printf("\n");
}


static void PrintHelp(PSETTINGS pSettings)
{
    NOREF(pSettings);

#define INDENT_SPACES   ""

    ColorPrintf(TCOLOR_BOLD_GREEN, "%s (c) 2012, Ramshankar (aka Teknomancer)\n", APP_EXECNAME);
    Printf("%s is a command-line expression evaluator that started as a bored weekend project.\n", APP_EXECNAME);
    Printf("Feel free to send me any feedback at <v.ramshankar(at)gmail.com>.\n\n");

    Printf(INDENT_SPACES "Operators:\n");

    unsigned cOperators = EvaluatorOperatorCount();
    for (unsigned i = 0; i < cOperators; i++)
    {
        char *pszOperator = NULL;
        char *pszSyntax = NULL;
        char *pszDesc = NULL;

        int rc = EvaluatorOperatorHelp(i, &pszOperator, &pszSyntax, &pszDesc);
        if (RC_SUCCESS(rc))
        {
            ColorPrintf(PREFIX_COLOR, "%11s ", pszOperator);
            ColorPrintf(PARAM_COLOR, "%-20s ", pszSyntax);
            Printf("%s\n", pszDesc);
        }
        StrFree(pszOperator);
        StrFree(pszSyntax);
        StrFree(pszDesc);
    }

    Printf(INDENT_SPACES "Functions:\n");

    /*
     * Display help for each functor with some attempt at pretty formatting.
     */
    unsigned cFunctors = EvaluatorFunctorCount();
    for (unsigned i = 0; i < cFunctors; i++)
    {
        char *pszFunctor = NULL;
        char *pszSyntax = NULL;
        char *pszDesc = NULL;

        int rc = EvaluatorFunctorHelp(i, &pszFunctor, &pszSyntax, &pszDesc);
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

            ColorPrintf(PREFIX_COLOR, "%11s ", pszFunctor);
            ColorPrintf(PARAM_COLOR, "%-20s ", pszSyntaxFmted);
            Printf("%s\n", pszDesc);
        }
        StrFree(pszFunctor);
        StrFree(pszSyntax);
        StrFree(pszDesc);
    }

    Printf("\tLegend:\n");
    ColorPrintf(PREFIX_COLOR, "\t <numX> ");
    Printf("A floating point number.\n");
    Printf("\t\tAny functor/operator taking <numX> will treat it as floating point.\n");

    ColorPrintf(PREFIX_COLOR, "\t <intX> ");
    Printf("An integer.\n");
    Printf("\t\tAny functor/operator taking <intX> indicates it will treat it as integer.\n");
    Printf("\t\tSince all <numX> are float, the functor/operator will cast it to integer.\n");
    Printf("\t\tIf <numX> cannot be represented as an integer, it will result in an overflow.\n");

    ColorPrintf(PREFIX_COLOR, "\t <cond> ");
    Printf("A condition or truth value.\n");
    Printf("\t\tA logical condition, with \"true\" being 0 and \"false\" being non-zero.\n");

    ColorPrintf(PREFIX_COLOR, "\t <expr> ");
    Printf("An expression such as \"5 + 4 * 3.6\" or \"42/sum(1024,128,512)\"\n");
    Printf("\t\tExpressions always result in a <numX>\n");

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

            ColorPrintf(PREFIX_COLOR, "%24s", pszVar);
            Printf(" = ");
            ColorPrintf(PARAM_COLOR, "%s\n", pszExprTrimmed);

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
}


static void PrintVarAssigned(PSETTINGS pSettings, PEVALUATOR pEval)
{
    ColorPrintf(PARAM_COLOR, "  Stored variable '%s'\n", pEval->Result.szVariable);
}

static void PrintCommandEvaluated(PSETTINGS pSettings, PEVALUATOR pEval)
{
    ColorPrintf(PREFIX_COLOR, "%s:\n", pEval->Result.szCommand);
    ColorPrintf(PARAM_COLOR, "%s\n",   pEval->Result.szCommandResult);
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
            PrintResult(pSettings, pEval->Result.dValue);
    }
    else
    {
        char szComponent[128];
        StrNPrintf(szComponent, sizeof(szComponent), "%s:", fParsed ? "Evaluator" : "Parser");
        switch (rc)
        {
            case RERR_EXPRESSION_INVALID:       ErrorPrintf(rc, "%s Invalid expression.\n", szComponent); break;
            case RERR_TOO_FEW_PARAMETERS:       ErrorPrintf(rc, "%s Not enough parameters to operator/functor.\n", szComponent); break;
            case RERR_TOO_MANY_PARAMETERS:      ErrorPrintf(rc, "%s Too many parameters to operator/functor.\n", szComponent); break;
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

