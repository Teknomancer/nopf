/** @file
 * Evaluator, hopefully should be fairly generic.
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
#include "InputOutput.h"
#include "EvaluatorInternal.h"
#include "EvaluatorFunctions.h"
#include "EvaluatorCommands.h"
#include "Stack.h"
#include "Queue.h"
#include "List.h"
#include "Assert.h"
#include "GenericDefs.h"
#include "Errors.h"
#include "Magics.h"
#include "StringOps.h"

#include <errno.h>

/*******************************************************************************
 *   Static functions                                                          *
 *******************************************************************************/
static int OpAdd(PEVALUATOR, PTOKEN[]);
static int OpSubtract(PEVALUATOR, PTOKEN[]);
static int OpNegate(PEVALUATOR, PTOKEN[]);
static int OpMultiply(PEVALUATOR, PTOKEN[]);
static int OpDivide(PEVALUATOR, PTOKEN[]);
static int OpIncrement(PEVALUATOR, PTOKEN[]);
static int OpDecrement(PEVALUATOR, PTOKEN[]);
static int OpShiftLeft(PEVALUATOR, PTOKEN[]);
static int OpShiftRight(PEVALUATOR, PTOKEN[]);
static int OpBitNegate(PEVALUATOR, PTOKEN[]);
static int OpModulo(PEVALUATOR, PTOKEN[]);
static int OpLessThan(PEVALUATOR, PTOKEN[]);
static int OpGreaterThan(PEVALUATOR, PTOKEN[]);
static int OpEqualTo(PEVALUATOR, PTOKEN[]);
static int OpLessThanOrEqualTo(PEVALUATOR, PTOKEN[]);
static int OpGreaterThanOrEqualTo(PEVALUATOR, PTOKEN[]);
static int OpNotEqualTo(PEVALUATOR, PTOKEN[]);
static int OpLogicalNot(PEVALUATOR, PTOKEN[]);
static int OpBitwiseAnd(PEVALUATOR, PTOKEN[]);
static int OpBitwiseXor(PEVALUATOR, PTOKEN[]);
static int OpBitwiseOr(PEVALUATOR, PTOKEN[]);
static int OpLogicalAnd(PEVALUATOR, PTOKEN[]);
static int OpLogicalOr(PEVALUATOR, PTOKEN[]);


/*******************************************************************************
 *   Globals, Typedefs & Defines                                               *
 *******************************************************************************/
PCOPERATOR g_pOperatorOpenParenthesis = NULL;
PCOPERATOR g_pOperatorCloseParenthesis = NULL;

/** List of Operators. */
OPERATOR g_aOperators[] =
{
    /*  Id             Pri Associativity cParams fUIntParams Name  pfn     ShortHelp          LongHelp */
    { OPEN_PAREN_ID,  99,  enmDirNone,   0,       false,    "(",  NULL, "(<expr>", "Begin subexpression or function." },
    { CLOSE_PAREN_ID, 99,  enmDirNone,   0,       false,    ")",  NULL, "<expr>)", "End subexpression or function." },
    { PARAM_SEP_ID,    0,  enmDirLeft,   2,       false,    ",",  NULL, "<param1>, <param2>...", "Function Parameter separator." },
    { VAR_ASSIGN_ID,   0,  enmDirLeft,   2,       false,    "=",  NULL, "<var>=<expr>", "Variable assignment." },

    {   3,            90,  enmDirLeft,   1,       false,   "++",  OpIncrement, "<num1>++", "Numeric increment." },
    {   4,            90, enmDirRight,   1,       false,   "++",  OpIncrement, "++<num1>", "Numeric increment." },
    {   5,            90,  enmDirLeft,   1,       false,   "--",  OpDecrement, "<num1>--", "Numeric decrement." },
    {   6,            90, enmDirRight,   1,       false,   "--",  OpDecrement, "--<num1>", "Numeric decrement." },
    {   7,            90, enmDirRight,   1,       false,    "-",  OpNegate, "-<num1>", "Numeric negation." },
    {   8,            90, enmDirRight,   1,       false,    "+",  NULL, "+<num1>", "Numeric unary plus." },
    {   9,            90, enmDirRight,   1,        true,    "~",  OpBitNegate, "~<num1>", "Bitwise negation." },
    {  10,            90, enmDirRight,   1,        true,    "!",  OpLogicalNot, "!<cond>", "Logical negation." },

    {  11,            80,  enmDirLeft,   2,       false,    "*",  OpMultiply, "<num1> * <num2>", "Numeric multiplication." },
    {  12,            80,  enmDirLeft,   2,       false,    "/",  OpDivide, "<num1> / <num2>", "Numeric division." },
    {  13,            80,  enmDirLeft,   2,       true,     "%",  OpModulo, "<int1> % <int2>", "Integer modulus (remainder)." },

    {  14,            70,  enmDirLeft,   2,       false,    "+",  OpAdd, "<num1> + <num2>", "Numeric addition."  },
    {  15,            70,  enmDirLeft,   2,       false,    "-",  OpSubtract, "<num1> - <num2>", "Numeric subtraction." },

    {  16,            60,  enmDirLeft,   2,       true,    "<<",  OpShiftLeft, "<int1> << <int2>", "Arithmetic left shift <int1> by <int2> places." },
    {  17,            60,  enmDirLeft,   2,       true,    ">>",  OpShiftRight, "<int1> >> <int2>", "Arithmetic right shift <int1> by <int2> places." },

    {  18,            50,  enmDirLeft,   2,       false,    "<",  OpLessThan, "<num1> < <num2>", "Less than." },
    {  19,            50,  enmDirLeft,   2,       false,    ">",  OpGreaterThan, "<num1> > <num2>", "Greater than." },
    {  20,            50,  enmDirLeft,   2,       false,   "<=",  OpLessThanOrEqualTo, "<num1> <= <num2>", "Less than or equals." },
    {  21,            50,  enmDirLeft,   2,       false,   ">=",  OpGreaterThanOrEqualTo, "<num1> >= <num2>", "Greater than or equals." },

    {  22,            40,  enmDirLeft,   2,       false,   "==",  OpEqualTo, "<num1> == <num2>", "Equals to comparison." },
    {  23,            40,  enmDirLeft,   2,       false,   "!=",  OpNotEqualTo, "<num1> != <num2>", "Not equal to comparison." },

    {  24,            30,  enmDirLeft,   2,        true,    "&",  OpBitwiseAnd, "<int1> & <int2>", "Bitwise AND." },

    {  25,            25,  enmDirLeft,   2,        true,    "^",  OpBitwiseXor, "<int1> ^ <int2>", "Bitwise XOR." },

    {  26,            20,  enmDirLeft,   2,        true,    "|",  OpBitwiseOr, "<int1> | <int2>", "Bitwise OR." },

    {  27,            18,  enmDirLeft,   2,        true,   "&&",  OpLogicalAnd, "<int1> && <int2>", "Logical AND." },

    {  28,            16,  enmDirLeft,   2,        true,   "||",  OpLogicalOr, "<int1> || <int2>", "Logical OR." },

};

const unsigned g_cOperators = R_ARRAY_ELEMENTS(g_aOperators);

/** Global list of Variables. */
static LIST g_VarList;

/** Alphabetically sorted array of Functions */
PFUNCTION g_paSortedFunctions = NULL;


/*******************************************************************************
*   Helper Functions                                                           *
*******************************************************************************/
static inline bool CanCastToken(PCTOKEN pToken, long double MinValue, long double MaxValue)
{
    return (DefinitelyLessThan(pToken->u.Number.dValue, MaxValue) && DefinitelyGreaterThan(pToken->u.Number.dValue, MinValue));
}

static inline void TokenInit(PTOKEN pToken)
{
    pToken->Type = enmTokenEmpty;
    pToken->Position = 0;
}

static PTOKEN TokenDup(PCTOKEN pSrcToken)
{
    PTOKEN pDstToken = MemAlloc(sizeof(TOKEN));
    if (pDstToken)
    {
        pDstToken->Type            = pSrcToken->Type;
        pDstToken->Position        = pSrcToken->Position;
        pDstToken->cFunctionParams = pSrcToken->cFunctionParams;
        StrCopy(pDstToken->szVariable, sizeof(pDstToken->szVariable), pSrcToken->szVariable);
        pDstToken->u               = pSrcToken->u; /* yeah implicit memcpy */
    }
    return pDstToken;
}

static inline bool TokenIsOperator(PCTOKEN pToken)
{
    return (pToken && pToken->Type == enmTokenOperator);
}

static inline bool TokenIsFunction(PCTOKEN pToken)
{
    return (pToken && pToken->Type == enmTokenFunction);
}

static inline bool TokenIsVariable(PCTOKEN pToken)
{
    return (pToken && pToken->Type == enmTokenVariable);
}

static inline bool OperatorIsOpenParenthesis(PCOPERATOR pOperator)
{
    return (pOperator->OperatorId == OPEN_PAREN_ID);
}

static inline bool OperatorIsCloseParenthesis(PCOPERATOR pOperator)
{
    return (pOperator->OperatorId == CLOSE_PAREN_ID);
}

static inline bool OperatorIsParamSeparator(PCOPERATOR pOperator)
{
    return (pOperator->OperatorId == PARAM_SEP_ID);
}

static inline bool OperatorIsAssignment(PCOPERATOR pOperator)
{
    return (pOperator->OperatorId == VAR_ASSIGN_ID);
}

static inline bool NumberIsNegative(PTOKEN pToken)
{
    return DefinitelyLessThan(pToken->u.Number.dValue, (long double)0);
}

static inline bool TokenIsCloseParenthesis(PCTOKEN pToken)
{
    if (   !pToken
        || !TokenIsOperator(pToken))
        return false;
    return OperatorIsCloseParenthesis(pToken->u.pOperator);
}

static inline bool TokenIsOpenParenthesis(PCTOKEN pToken)
{
    if (   !pToken
        || !TokenIsOperator(pToken))
        return false;
    return OperatorIsOpenParenthesis(pToken->u.pOperator);
}

static inline bool TokenIsAssignment(PCTOKEN pToken)
{
    if (   !pToken
        || !TokenIsOperator(pToken))
        return false;
    return OperatorIsAssignment(pToken->u.pOperator);
}

static inline bool TokenIsParamSeparator(PCTOKEN pToken)
{
    if (   !pToken
        || !TokenIsOperator(pToken))
        return false;
    return OperatorIsParamSeparator(pToken->u.pOperator);
}

static inline bool TokenIsParenthesis(PCTOKEN pToken)
{
    if (   !pToken
        || !TokenIsOperator(pToken))
        return false;
    return (TokenIsOpenParenthesis(pToken) || TokenIsCloseParenthesis(pToken));
}

static void EvaluatorInvertTokenArray(PTOKEN apTokens[], uint32_t cTokens)
{
    if (cTokens < 2)
        return;

    uint32_t i = 0;
    PTOKEN pUnstableToken = NULL;
    for (i = 0; i < cTokens / 2; i++)
    {
        --cTokens;
        pUnstableToken = apTokens[i];
        apTokens[i] = apTokens[cTokens];
        apTokens[cTokens] = pUnstableToken;
    }
}


/**
 * Searches for a variable and returns a Variable Token if found.
 *
 * @return  Pointer to an allocated Variable Token or NULL if @a pszVariable could
 *          not be found.
 * @param   pszVariable     Name of the variable to find.
 */
static PVARIABLE EvaluatorFindVariable(const char *pszVariable, PLIST pVarList)
{
    for (uint32_t i = 0; i < ListSize(pVarList); i++)
    {
        PVARIABLE pVariable = ListItemAt(pVarList, i);
        AssertReturn(pVariable, NULL);
        DEBUGPRINTF(("EvaluatorFindVariable szVar=%s pszVar=%s\n", pVariable->szVariable, pszVariable));
        if (!StrCmp(pVariable->szVariable, pszVariable))
            return pVariable;
    }
    return NULL;
}


/**
 * Destroys a variable.
 *
 * @param   pVariable   The Variable to destroy.
 */
static void EvaluatorDestroyVariable(PVARIABLE pVariable)
{
    if (pVariable)
    {
        if (pVariable->pvRPNQueue)
        {
            PTOKEN pToken = NULL;
            while ((pToken = (PTOKEN)QueueRemove(pVariable->pvRPNQueue)) != NULL)
                MemFree(pToken);
        }
        MemFree(pVariable->pvRPNQueue);

        if (pVariable->pszExpr)
            StrFree(pVariable->pszExpr);

        MemFree(pVariable);
    }
}


/**
 * Cleans up the variable queue of unassigned Variables.
 */
static void EvaluatorCleanVariables(void)
{
    PLIST pVarList = &g_VarList;
    uint32_t i = 0;
    for (;;)
    {
        PVARIABLE pVariable = ListItemAt(pVarList, i);
        if (!pVariable)
            break;

        ++i;
        if (!pVariable->pvRPNQueue)
        {
            DEBUGPRINTF(("Destroying invalid variable '%s'\n", pVariable->szVariable));
            ListRemove(pVarList, pVariable);
            EvaluatorDestroyVariable(pVariable);
            if (i > 0)
                --i;
        }
    }
}


#ifdef _DEBUG
static void EvaluatorPrintVarList(PLIST pList)
{
    for (uint32_t i = 0; i < ListSize(pList); i++)
    {
        PVARIABLE pVariable = ListItemAt(pList, i);
        if (!pVariable)
            DEBUGPRINTF(("Huh i=%u!\n", i));
        else
            DEBUGPRINTF(("Var[%u]=%s\n", i, pVariable->szVariable));
    }
}
#endif


/**
 * Parses a number and returns a Number Token.
 *
 * @return  Pointer to an allocated Number Token or NULL if @a pszExpr is not a
 *          number.
 * @param   pszExpr     The whitespace skipped expression to parse.
 * @param   ppszEnd     Where to store till what point in pszExpr was scanned.
 * @param   prc         Where to store the status code while identifying the
 *                      number.
 */
static PTOKEN EvaluatorParseNumber(const char *pszExpr, const char **ppszEnd)
{
    DEBUGPRINTF(("Parse Number:\n"));

    /*
     * UINT64_MAX is the maximum supported type which is:
     *   In binary     :  1111111111111111111111111111111111111111111111111111111111111111 (64 digits)
     *   In octal      :  1777777777777777777777                                           (22 digits)
     *   In decimal    :  18446744073709551615                                             (20 digits)
     *   In hexadecimal:  ffffffffffffffff                                                 (16 digits)
     *
     * We allow inputing numbers in binary, so the maximum digits we need to allow is 64.
     */
    char szNum[64 + 1];
    int  iRadix = 0;
    int  iNum   = 0;
    bool fDecPt = false;

    const char *pszStart = pszExpr;
    while (*pszExpr)
    {
        bool fStopNumParse = false;

        /*
         * Skip whitespace.
         */
        if (isspace(*pszExpr))
        {
            pszExpr++;
            continue;
        }

        /*
         * Check for overflow of known radices.
         */
        if (   (iRadix == 16 && iNum == 16)
            || (iRadix == 8  && iNum == 22)
            || (iRadix == 2  && iNum == 64)
            || ((iRadix == 0 || iRadix == 10) && iNum == 20))
        {
            pszExpr = pszStart;
            return NULL;
        }

        Assert(iNum < sizeof(szNum));

        /*
         * Parse explicit prefixes.
         */
        if (iNum == 0)
        {
            switch (*pszExpr)
            {
                /* Binary. */
                case 'n':
                case 'N':
                {
                    if (iRadix == 0)
                    {
                        iRadix = 2;
                        ++pszExpr;
                        continue;
                    }
                    break;
                }

                /* Octal. */
                case '0':
                {
                    if (iRadix == 0)
                    {
                        iRadix = 8;
                        ++pszExpr;
                        continue;
                    }
                    break;
                }

                /* Hexadecimal. */
                case 'x':
                case 'X':
                {
                    if (iRadix == 8)
                    {
                        iRadix = 16;
                        ++pszExpr;
                        continue;
                    }
                    break;
                }
            }
        }

        /*
         * Parse number (and decipher a prefix).
         */
        switch (*pszExpr)
        {
            case '.':
            {
                if (fDecPt)             /* If decimal point has already been used once in this number (e.g.: "10.5.5") */
                {
                    pszExpr = pszStart;
                    return NULL;
                }

                if (iRadix == 0)        /* If no prefix has been specified thus far, use implicit decimal prefix (for float). */
                {
                    iRadix = 10;
                    szNum[iNum++] = *pszExpr++;
                    fDecPt = true;
                    continue;
                }

                if (iRadix != 10)       /* Using decimal point after for a non-decimal number (e.g.: "0xffec.5"). */
                {
                    pszExpr = pszStart;
                    return NULL;
                }
            }

            case 'f': case 'F':
            case 'e': case 'E':
            case 'd': case 'D':
            case 'c': case 'C':
            case 'b': case 'B':
            case 'a': case 'A':
            {
                if (iRadix == 0)        /* If no prefix has been specified, try implicit hexadecimal prefix. */
                    iRadix = 16;
                else if (iRadix != 16)  /* Using non-hexadecimal digits after specifying a hexadecimal prefix. */
                {
                    pszExpr = pszStart;
                    return NULL;
                }
                R_FALLTHRU();
            }
            case '9':
            case '8':
            {
                if (iRadix == 8)        /* Using non-octal digits after specifying an octal prefix. */
                {
                    pszExpr = pszStart;
                    return NULL;
                }
                R_FALLTHRU();
            }
            case '7': case '6':
            case '5': case '4':
            case '3': case '2':
            {
                if (iRadix == 2)        /* Using non-binary digits after specifying a binary prefix. */
                {
                    pszExpr = pszStart;
                    return NULL;
                }
                R_FALLTHRU();
            }
            case '0':
            case '1':
            {
                szNum[iNum++] = *pszExpr++;
                continue;
            }

            /*
             * All other characters that are not part of a number.
             */
            default:
            {
                /*
                 * If we have an unrecognized character -WITH- numeric digits parsed so far
                 * (with or without a prefix), it -might- be a number (e.g.: "32UL"). Stop
                 * parsing the number in this loop, but proceed with overall parsing.
                 *
                 * This also applies when a sequence like "RT_BIT(0)" ends up parsing "0)"
                 * so we -should- parse the 0 in this case but not proceed with parsing.
                 */
                fStopNumParse = true;
                break;
            }
        }

        /* Check if we need to stop parsing the number and bail from this loop. */
        if (fStopNumParse)
            break;
    }

    if (iNum == 0)
    {
        /* When only '0' is given, we interpret it as Octal prefix and don't fill szNum, add it as a plain 0 here. */
        if (iRadix == 8)
        {
            szNum[iNum++] = '0';
            iRadix = 10;
        }
        else
        {
            pszExpr = pszStart;
            return NULL;
        }
    }
    else if (szNum[iNum - 1] == '.')
    {
        /* A decimal number ending with a '.' is invalid (e.g.: "5."). */
        pszExpr = pszStart;
        return NULL;
    }

    /*
     * Handle suffixes, order of array is important (longest to shortest).
     */
    static const char * const s_apszSuffixes[] =
    {
        "ULL",
        "LLU",
        "UL",
        "LU",
        "LL",
        "U",
        "L"
    };
    for (size_t k = 0; k < R_ARRAY_ELEMENTS(s_apszSuffixes); k++)
        if (!StrNCaseCmp(pszExpr, s_apszSuffixes[k], StrLen(s_apszSuffixes[k])))
            pszExpr += StrLen(s_apszSuffixes[k]);

    /*
     * eg: "b2mb", we've read "b2" as hexadecimal and then check if an alphabet is following
     * the number, if so we don't interpret it as a hexadecimal number because numbers are
     * never suffixed.
     */
    if (isalpha(*pszExpr))
        return NULL;

    /*
     * We've got a number. Terminate our string buffer, and convert it.
     */
    szNum[iNum] = '\0';
    char *pszEndTmp = NULL;

    errno = 0;
    DEBUGPRINTF(("Parse Number: szNum=\"%s\"\n", szNum));
    uint64_t const uValue = (uint64_t)strtoull(szNum, &pszEndTmp, iRadix);
    if (errno)
    {
        DEBUGPRINTF(("Error while string to unsigned conversion of %s\n", szNum));
        return NULL;
    }

    /*
     * Covert the number as a float value as well.
     */
    long double dValue = strtold(szNum, &pszEndTmp);
    if (iRadix == 10 || iRadix == 0)
    {
        dValue = strtold(szNum, &pszEndTmp);
        if (errno)
        {
            DEBUGPRINTF(("Error while string to unsigned conversion of %s\n", szNum));
            return NULL;
        }
    }
    else
        dValue = uValue;

    /*
     * Create a new Number Token and store the numeric values.
     */
    PTOKEN pToken = MemAlloc(sizeof(TOKEN));
    if (!pToken)
        return NULL;
    pToken->Type = enmTokenNumber;
    pToken->u.Number.uValue = uValue;
    pToken->u.Number.dValue = dValue;
    *ppszEnd = pszExpr;

    DEBUGPRINTF(("Parse Number: U=%" FMT_U64_NAT " (%" FMT_U64_HEX ") F=%" FMT_FLT_NAT "\n", uValue, uValue, dValue));

    /*
     * Return the allocated Token.
     */
    return pToken;
}


/**
 * Parses an operator and returns an Operator Token.
 *
 * @return  Pointer to an allocated Operator Token or NULL if @a pszExpr was not an
 *          operator.
 * @param   pszExpr         The whitespace skipped expression to parse.
 * @param   ppszEnd         Where to store till what point in pszExpr was scanned.
 * @param   pPreviousToken   The previously passed Token in @a pszExpr if any, can be
 *                          NULL.
 */
static PTOKEN EvaluatorParseOperator(const char *pszExpr, const char **ppszEnd, PCTOKEN pPreviousToken)
{
    DEBUGPRINTF(("Parse Operator:\n"));

    for (unsigned i = 0; i < g_cOperators; i++)
    {
        size_t cbOperator = StrLen(g_aOperators[i].pszOperator);
        if (!StrNCmp(g_aOperators[i].pszOperator, pszExpr, cbOperator))
        {
            /*
             * Verify if there are enough parameters on the queue for left associative operators.
             * e.g for binary '-', the previous token must exist and must not be an open parenthesis or any
             * other operator.
             */
            if (g_aOperators[i].Direction == enmDirLeft)
            {
                /* e.g: "-4" */
                if (!pPreviousToken)
                    continue;

                /*
                 * pPreviousToken should never  be close parantheis as it's deleted in EvaluatorParse(),
                 * but included in here for the logical condition.
                 */

                /* e.g: "(-4"  and "=-4" and ",-4" */
                if (   TokenIsOperator(pPreviousToken)
                    && !TokenIsCloseParenthesis(pPreviousToken))
                    continue;
            }

            PTOKEN pToken = MemAlloc(sizeof(TOKEN));
            if (!pToken)
                return NULL;
            pToken->Type = enmTokenOperator;
            pToken->u.pOperator = &g_aOperators[i];
            pszExpr += cbOperator;
            *ppszEnd = pszExpr;
            return pToken;
        }
    }
    return NULL;
}


/**
 * Parses a Function and returns a Function Token.
 *
 * @return  Pointer to an allocated Function Token or NULL if @a pszExpr was not a
 *          function.
 * @param   pszExpr         The whitespace skipped expression to parse.
 * @param   ppszEnd         Where to store till what point in pszExpr was scanned.
 * @param   pPreviousToken   The previously passed Token in @a pszExpr if any, can be
 *                          NULL.
 */
static PTOKEN EvaluatorParseFunction(const char *pszExpr, const char **ppszEnd, PCTOKEN pPreviousToken)
{
    for (unsigned i = 0; i < g_cFunctions; i++)
    {
        size_t cbFunction = StrLen(g_aFunctions[i].pszFunction);
        if (!StrNCmp(g_aFunctions[i].pszFunction, pszExpr, cbFunction))
        {
            /*
             * Skip over whitespaces till we encounter an open parenthesis.
             */
            pszExpr += cbFunction;
            while (isspace(*pszExpr))
                pszExpr++;

            if (!StrNCmp(pszExpr, g_pOperatorOpenParenthesis->pszOperator,
                            StrLen(g_pOperatorOpenParenthesis->pszOperator)))
            {
                PTOKEN pToken = MemAlloc(sizeof(TOKEN));
                if (!pToken)
                    return NULL;
                pToken->Type = enmTokenFunction;
                pToken->u.pFunction = &g_aFunctions[i];
                pToken->cFunctionParams = 0;
                *ppszEnd = pszExpr;
                return pToken;
            }
        }
    }
    return NULL;
}


/**
 * Parses a variable and returns a Variable Token.
 *
 * @return  Pointer to an allocated Variable Token or NULL if @a pszExpr was not a
 *          variable.
 * @param   pEval           The Evaluator object.
 * @param   pszExpr         The whitespace skipped expression to parse.
 * @param   ppszEnd         Where to store till what point in pszExpr was scanned.
 * @param   pPreviousToken   The previously passed Token in if any, can be NULL.
 * @param   prc             Where to store the status code while identifying the
 *                          variable.
 */
static PTOKEN EvaluatorParseVariable(PEVALUATOR pEval, const char *pszExpr, const char **ppszEnd, PCTOKEN pPreviousToken, int *prc)
{
    DEBUGPRINTF(("Parse Variable:\n"));

    /*
     * A variable is a stream of one or more contiguous alpha numerics i.e. only "_[a-z][0-9]".
     * Variables cannot begin with a digit [0-9] though.
     */
    char       szBuf[MAX_VARIABLE_NAME_LENGTH];
    unsigned   iVar = 0;
    bool       fValid = false;

    if (!isdigit(*pszExpr))
    {
        while (*pszExpr)
        {
            if (   *pszExpr == '_'
                || isalnum(*pszExpr))
            {
                fValid = true;
                szBuf[iVar++] = *pszExpr++;
                if (iVar == MAX_VARIABLE_NAME_LENGTH - 1)
                    break;
            }
            else
                break;
        }
    }
    szBuf[iVar] = '\0';
    *ppszEnd = pszExpr;

    if (!fValid)
    {
        *prc = RERR_VARIABLE_NAME_INVALID;
        return NULL;
    }

    PTOKEN pToken = MemAlloc(sizeof(TOKEN));
    if (!pToken)
    {
        *prc = RERR_NO_MEMORY;
        return NULL;
    }
    pToken->Type = enmTokenVariable;

    /*
     * Associate the Token with a predefined Variable, if not just record the name.
     * When we assign the Variable, we will create the actual Variable entry.
     */
    pToken->u.pVariable = EvaluatorFindVariable(szBuf, &g_VarList);
    StrCopy(pToken->szVariable, sizeof(pToken->szVariable), szBuf);

    /*
     * Determine error code. For variables too long we return a successful (name truncated) Variable Token
     * and the caller deals with exiting, cleaning-up etc. If we just return a NULL Token, the caller cannot
     * identify the exact variable name.
     */
    if (iVar >= MAX_VARIABLE_NAME_LENGTH - 1)
        *prc = RERR_VARIABLE_NAME_TOO_LONG;
    else
        *prc = RINF_SUCCESS;
    return pToken;
}


/**
 * Parses a command and returns a Command Token.
 *
 * @return  Pointer to an allocated Command Token or NULL if @a pszExpr was not a
 *          command.
 * @param   pEval           The Evaluator object.
 * @param   pszExpr         The whitespace skipped expression to parse.
 * @param   ppszEnd         Where to store till what point in pszExpr was scanned.
 * @param   pPreviousToken   The previously passed Token in @pszExpr if any, can be
 *                          NULL.
 * @param   prc             Where to store the status code while identifying the
 *                          variable.
 */
static PTOKEN EvaluatorParseCommand(PEVALUATOR pEval, const char *pszExpr, const char **ppszEnd, PCTOKEN pPreviousToken, int *prc)
{
    DEBUGPRINTF(("Parse Command\n"));

    /*
     * A command must be the first token in the expression.
     */
    if (pPreviousToken)
        return NULL;

    /*
     * A command is a stream of contiguous alpha numerics, i.e. only [_][a-z][0-9], nothing else.
     */
    for (unsigned i = 0; i < g_cCommands; i++)
    {
        DEBUGPRINTF(("Parse Command: %s\n", g_aCommands[i].pszCommand));
        size_t cCommand = StrLen(g_aCommands[i].pszCommand);
        if (!StrNCmp(g_aCommands[i].pszCommand, pszExpr, cCommand))
        {
            /*
             * Make sure the next character is a space or an open parenthesis.
             */
            pszExpr += cCommand;

            if (   *pszExpr == '\0'
                ||  isspace(*pszExpr)
                || !StrNCmp(pszExpr, g_pOperatorOpenParenthesis->pszOperator,
                            StrLen(g_pOperatorOpenParenthesis->pszOperator)))
            {
                /*
                 * Skip over all whitespaces, this is important, see EvaluatorParse().
                 */
                while (isspace(*pszExpr))
                    pszExpr++;

                PTOKEN pToken = MemAllocZ(sizeof(TOKEN));
                if (!pToken)
                    return NULL;
                pToken->Type = enmTokenCommand;
                pToken->u.pCommand = &g_aCommands[i];
                *ppszEnd = pszExpr;
                return pToken;
            }
        }
    }
    return NULL;
}


/**
 * Parses an Token.
 *
 * @return  Pointer to an allocated Token or NULL if @a pszExpr has run out of
 *          identifying tokens.
 * @param   pEval           The Evaluator object.
 * @param   pszExpr         The expression to parse.
 * @param   ppszEnd         Where to store till what point in pszExpr was scanned.
 * @param   pPreviousToken   The previously passed Token in @a pszExpr if any, can be
 *                          NULL.
 * @param   prc             Where to store the status code during parsing.
 */
static PTOKEN EvaluatorParseToken(PEVALUATOR pEval, const char *pszExpr, const char **ppszEnd, PCTOKEN pPreviousToken, int *prc)
{
    DEBUGPRINTF(("GetToken %s\n", pszExpr));
    PTOKEN pToken = NULL;
    while (*pszExpr)
    {
        /*
         * Skip whitespaces.
         */
        if (isspace(*pszExpr))
        {
            pszExpr++;
            continue;
        }

        /*
         * Parse function.
         */
        pToken = EvaluatorParseFunction(pszExpr, ppszEnd, pPreviousToken);
        if (pToken)
            break;

        /*
         * Parse command.
         */
        pToken = EvaluatorParseCommand(pEval, pszExpr, ppszEnd, pPreviousToken, prc);
        if (pToken)
            break;

        /*
         * Parse number.
         */
        pToken = EvaluatorParseNumber(pszExpr, ppszEnd);
        if (pToken)
            break;

        /*
         * Parse operator.
         */
        pToken = EvaluatorParseOperator(pszExpr, ppszEnd, pPreviousToken);
        if (pToken)
            break;

        /*
         * Parse variable.
         */
        pToken = EvaluatorParseVariable(pEval, pszExpr, ppszEnd, pPreviousToken, prc);
        if (pToken)
            break;

        /** @todo hmm, think about this!? */
        *ppszEnd = pszExpr;
        break;
    }
    return pToken;
}


static int OperatorSortCompare(const void *pvOperator1, const void *pvOperator2)
{
    PCOPERATOR pOperator1 = (PCOPERATOR)pvOperator1;
    PCOPERATOR pOperator2 = (PCOPERATOR)pvOperator2;
    const char *pszOperator1 = pOperator1->pszOperator;
    const char *pszOperator2 = pOperator2->pszOperator;

    int rc = StrNCmp(pszOperator1, pszOperator2, R_MAX(StrLen(pszOperator1), StrLen(pszOperator2)));
    if (!rc)
        return pOperator2->cParams - pOperator1->cParams;
    return -rc;
}


static int FunctionSortCompare(const void *pvFunction1, const void *pvFunction2)
{
    PCFUNCTION pFunction1 = (PCFUNCTION)pvFunction1;
    PCFUNCTION pFunction2 = (PCFUNCTION)pvFunction2;
    const char *pszFunction1 = pFunction1->pszFunction;
    const char *pszFunction2 = pFunction2->pszFunction;
    return -StrNCmp(pszFunction1, pszFunction2, R_MAX(StrLen(pszFunction1), StrLen(pszFunction2)));
}


static int AscendingFunctionSortCompare(const void *pvFunction1, const void *pvFunction2)
{
    PCFUNCTION pFunction1 = (PCFUNCTION)pvFunction1;
    PCFUNCTION pFunction2 = (PCFUNCTION)pvFunction2;
    const char *pszFunction1 = pFunction1->pszFunction;
    const char *pszFunction2 = pFunction2->pszFunction;

    /*
     * For commands (i.e. no syntax description) we always sort them last.
     * For functions (i.e. with syntax description) we sort alphabetically.
     */
    bool fIsCommand1 = !StrCmp(pFunction1->pszSyntax, "");
    bool fIsCommand2 = !StrCmp(pFunction2->pszSyntax, "");
    if (fIsCommand1 == fIsCommand2)
        return StrCmp(pszFunction1, pszFunction2);

    if (fIsCommand1)
        return 1;
    else
        return -1;
}


/**
 * Internal, essential Evaluator initialization. The other one does a lot of
 * extra work like global initializations. This is the real deal.
 *
 * @param   pEval   The Evaluator object, cannot be NULL.
 */
static void EvaluatorInitInternal(PEVALUATOR pEval)
{
    pEval->pvRPNQueue = NULL;
    pEval->u32Magic = RMAG_EVALUATOR;
    ListInit(&pEval->VarList);
    pEval->Result.fCommandEvaluated   = false;
    pEval->Result.fVariableAssignment = false;
}


/**
 * Cleans up an Evaluator object half-way through parsing or evaluation.
 * Used whenever a fatal error occurs and processing of expression must not continue.
 *
 * @param   pEval   The Evaluator object, cannot be NULL.
 * @param   pStack  Pointer to any Stack to empty, can be NULL.
 */
static void EvaluatorCleanUp(PEVALUATOR pEval, PSTACK pStack)
{
    Assert(pEval);

    PTOKEN pToken = NULL;
    PQUEUE pQueue = (PQUEUE)pEval->pvRPNQueue;
    if (pQueue)
    {
        while ((pToken = QueueRemove(pQueue)) != NULL)
            MemFree(pToken);
        MemFree(pQueue);
    }
    pEval->pvRPNQueue = NULL;

    /*
     * Only holds copies of PVARIABLE pointers, so nothing to free
     * but empty the List's items.
     */
    while (ListRemoveItemAt(&pEval->VarList, 0))
        ;

    if (pStack)
    {
        while ((pToken = StackPop(pStack)) != NULL)
            MemFree(pToken);
    }
}


/**
 * Parses the expression into a modified reverse polish notation form. The logic
 * is mostly based on the shunting yard algorithm with modifications for extra
 * elements. This constitutes the first pass in evaluating an expression.
 * The @a pEval object is internally updated with the intermediate representation
 * which will be used in the next pass, which is evaluation.
 *
 * @return  Status code on result of the parsing pass.
 * @param   pEval   The Evaluator object.
 */
int EvaluatorParse(PEVALUATOR pEval, const char *pszExpr)
{
    Assert(pEval);
    AssertReturn(pEval->u32Magic == RMAG_EVALUATOR, RERR_BAD_MAGIC);

    const char *pszEnd   = NULL;
    PCTOKEN pPreviousToken = NULL;
    PTOKEN pToken          = NULL;

    STACK Stack;
    StackInit(&Stack);

    PQUEUE pQueue = MemAlloc(sizeof(QUEUE));
    if (!pQueue)
        return RERR_NO_MEMORY;
    QueueInit(pQueue);

    /*
     * Assume this is not a variable assignment expression. If it is, the relevant
     * parts will fill this information so the caller can handle it as an assignemnt.
     */
    pEval->Result.fVariableAssignment = false;
    pEval->Result.fCommandEvaluated = false;
    MemZero(pEval->Result.szVariable);
    MemZero(pEval->Result.szCommandResult);

    /*
     * Parse tokens onto the stack or queue.
     */
    int rc = RERR_UNDEFINED;
    while ((pToken = EvaluatorParseToken(pEval, pszExpr, &pszEnd, pPreviousToken, &rc)) != NULL)
    {
        if (TokenIsCloseParenthesis(pPreviousToken))
        {
            MemFree((void *)pPreviousToken);
            pPreviousToken = NULL;
        }

        if (pToken->Type == enmTokenNumber)
        {
            DEBUGPRINTF(("Adding number (U=%" FMT_U64_NAT " F=%" FMT_FLT_NAT ") to queue\n", pToken->u.Number.uValue,
                         pToken->u.Number.dValue));
            QueueAdd(pQueue, pToken);
        }
        else if (pToken->Type == enmTokenOperator)
        {
            PCOPERATOR pOperator = pToken->u.pOperator;
            if (OperatorIsOpenParenthesis(pOperator))
            {
                /*
                 * Open parenthesis.
                 */
                DEBUGPRINTF(("Parenthesis begin '%s' pushing to stack\n", pToken->u.pOperator->pszOperator));
                StackPush(&Stack, pToken);
            }
            else if (OperatorIsCloseParenthesis(pOperator))
            {
                /*
                 * Close paranthesis.
                 */
                DEBUGPRINTF(("Parenthesis end '%s'\n", pToken->u.pOperator->pszOperator));
                PTOKEN pStackToken = NULL;
                while ((   pStackToken = StackPeek(&Stack)) != NULL
                        && !TokenIsOpenParenthesis(pStackToken))
                {
                    DEBUGPRINTF(("Popping '%s' to queue\n", pStackToken->u.pOperator->pszOperator));
                    StackPop(&Stack);
                    QueueAdd(pQueue, pStackToken);
                }

                /*
                 * If no matching open parenthesis for close parenthesis found, bail.
                 */
                if (pStackToken == NULL)
                {
                     DEBUGPRINTF(("Missing open paranthesis\n"));
                     MemFree(pToken);
                     EvaluatorCleanUp(pEval, &Stack);
                     return RERR_PARENTHESIS_UNBALANCED;
                }

                /*
                 * This means "pStackToken" is an open parenthesis, verify and discard.
                 */
                Assert(TokenIsOpenParenthesis(pStackToken));
                StackPop(&Stack);
                MemFree(pStackToken);
                pStackToken = NULL;

                /*
                 * If the left parenthesis is preceeded by a function, pop it to the Queue incrementing number
                 * of parameters the function already has.
                 */
                pStackToken = StackPeek(&Stack);
                if (   pStackToken
                    && TokenIsFunction(pStackToken))
                {
                    DEBUGPRINTF(("Popping function '%s' to queue\n", pStackToken->u.pFunction->pszFunction));
                    StackPop(&Stack);
                    QueueAdd(pQueue, pStackToken);

                    ++pStackToken->cFunctionParams;
                    if (pStackToken->cFunctionParams > MAX_FUNCTION_PARAMETERS)
                    {
                        DEBUGPRINTF(("Error! too many parameters to function '%s'\n", pStackToken->u.pFunction->pszFunction));

                        /*
                         * Too many parameters to function. Get 0wt.
                         */
                        MemFree(pToken);
                        EvaluatorCleanUp(pEval, &Stack);
                        return RERR_TOO_MANY_PARAMETERS;
                    }

                    if (pStackToken->cFunctionParams < pStackToken->u.pFunction->cMinParams)
                    {
                        DEBUGPRINTF(("Error! too few parameters to function '%s'\n", pStackToken->u.pFunction->pszFunction));

                        /*
                         * Too few parameters to function. 0wttie.
                         */
                        MemFree(pToken);
                        EvaluatorCleanUp(pEval, &Stack);
                        return RERR_TOO_FEW_PARAMETERS;
                    }

                    DEBUGPRINTF(("Function '%s' cParams=%u\n", pStackToken->u.pFunction->pszFunction, (unsigned)pStackToken->cFunctionParams));
                }
            }
            else if (OperatorIsParamSeparator(pOperator))
            {
                /*
                 * Function parameter separator.
                 */
                DEBUGPRINTF(("Parameter separator '%s'\n", pToken->u.pOperator->pszOperator));
                PTOKEN pStackToken = NULL;
                while ((pStackToken = StackPeek(&Stack)) != NULL)
                {
                    if (TokenIsOpenParenthesis(pStackToken))
                        break;
                    StackPop(&Stack);
                    QueueAdd(pQueue, pStackToken);
                }
                if (!TokenIsOpenParenthesis(pStackToken))
                {
                    DEBUGPRINTF(("Operator '%s' parameter mismatch\n", pOperator->pszOperator));
                    MemFree(pToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_PARANTHESIS_SEPARATOR_UNEXPECTED;
                }

                /*
                 * Check if the paranthesis is part of a function, if so increment the parameter count
                 * in the function structure.
                 */
                PTOKEN pOpenParenthesisToken = StackPop(&Stack);
                PTOKEN pFunctionToken = StackPop(&Stack);
                if (!TokenIsFunction(pFunctionToken))
                {
                    DEBUGPRINTF(("No function specified\n"));
                    MemFree(pToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_PARANTHESIS_SEPARATOR_UNEXPECTED;
                }

                ++pFunctionToken->cFunctionParams;
                if (pFunctionToken->cFunctionParams >= pFunctionToken->u.pFunction->cMaxParams)
                {
                    DEBUGPRINTF(("Error too many parameters to function '%s' maximum=%d\n", pFunctionToken->u.pFunction->pszFunction,
                                pFunctionToken->u.pFunction->cMaxParams));

                    /*
                     * Too many parameters to function. Exit, stage 0wt.
                     */
                    MemFree(pToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_TOO_MANY_PARAMETERS;
                }

                /*
                 * Now that we've recorded the info into the function Token, restore the stack items as though
                 * nothing happened :)
                 */
                StackPush(&Stack, pFunctionToken);
                StackPush(&Stack, pOpenParenthesisToken);
                DEBUGPRINTF(("Function '%s' cParams=%u\n", pFunctionToken->u.pFunction->pszFunction, (unsigned)pFunctionToken->cFunctionParams));
            }
            else if (OperatorIsAssignment(pOperator))
            {
                /*
                 * Variable assignment operator. This is going to be fun.
                 */
                PTOKEN pVarToken = QueuePeekTail(pQueue);
                if (   pVarToken
                    && TokenIsVariable(pVarToken))
                {
                    EVALUATOR SubExprEval;
                    EvaluatorInitInternal(&SubExprEval);
                    const char *pszRightExpr = pszEnd;
                    DEBUGPRINTF(("Parsing subexpression '%s'\n", pszRightExpr));
                    int rc2 = EvaluatorParse(&SubExprEval, pszRightExpr);
                    if (RC_SUCCESS(rc2))
                    {
                        DEBUGPRINTF(("-- Done subexpression assignment '%s'\n", pszRightExpr));

                        if (!pVarToken->u.pVariable)
                        {
                            DEBUGPRINTF(("Creating global variable entry for '%s'\n", pVarToken->szVariable));

                            /*
                             * Create a variable entry for the Variable Token.
                             */
                            PVARIABLE pVariable = MemAlloc(sizeof(VARIABLE));
                            if (!pVariable)
                            {
                                MemFree(pToken);
                                EvaluatorCleanUp(pEval, &Stack);
                                EvaluatorDestroy(&SubExprEval);
                                return RERR_NO_MEMORY;
                            }
                            StrCopy(pVariable->szVariable, sizeof(pVariable->szVariable), pVarToken->szVariable);
                            pVariable->pszExpr = StrDup(pszRightExpr);  /* @todo check for failure */
                            pVariable->pvRPNQueue = SubExprEval.pvRPNQueue;
                            pVariable->fCanReinit = true;
                            SubExprEval.pvRPNQueue = NULL;  /* tricky shit, i know... */

                            /*
                             * Add variable entry to the 'global' list & connect Token to the variable.
                             * Heh, good thing we are not multi-threaded.
                             */
                            ListAdd(&g_VarList, pVariable);
                            MemFree(pToken);
                        }
                        else if (pVarToken->u.pVariable->fCanReinit)
                        {
                            /*
                             * Reassigning existing variable.
                             */
                            if (pVarToken->u.pVariable->pszExpr)
                                StrFree(pVarToken->u.pVariable->pszExpr);
                            pVarToken->u.pVariable->pszExpr = StrDup(pszRightExpr);
                            if (!pVarToken->u.pVariable->pszExpr)
                            {
                                MemFree(pToken);
                                EvaluatorCleanUp(pEval, &Stack);
                                EvaluatorDestroy(&SubExprEval);
                                return RERR_NO_MEMORY;
                            }

                            pVarToken->u.pVariable->pvRPNQueue = SubExprEval.pvRPNQueue;
                            SubExprEval.pvRPNQueue = NULL;  /* tricky shit, i know... */
                        }
                        else
                        {
                            StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), pVarToken->szVariable);
                            MemFree(pToken);
                            EvaluatorCleanUp(pEval, &Stack);
                            EvaluatorDestroy(&SubExprEval);
                            return RERR_VARIABLE_CANNOT_REASSIGN;
                        }

                        StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), pVarToken->szVariable);
                        pEval->Result.fVariableAssignment = true;

                        /*
                         * Clean up undefined variables & destroy temporary Evaluator object used for parsing.
                         */
                        EvaluatorCleanVariables();
                        EvaluatorDestroy(&SubExprEval);
                        break;
                    }
                    else
                    {
                        MemFree(pToken);
                        EvaluatorCleanUp(pEval, &Stack);
                        EvaluatorDestroy(&SubExprEval);
                        return RERR_EXPRESSION_INVALID;
                    }
                }
                else
                {
                    MemFree(pToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_INVALID_ASSIGNMENT;
                }
            }
            else
            {
                /*
                 * Regular operator, handle precedence.
                 */
                PTOKEN pStackToken = NULL;
                while ((pStackToken = StackPeek(&Stack)) != NULL)
                {
                    if (  !TokenIsOperator(pStackToken)
                        || TokenIsParenthesis(pStackToken))
                        break;

                    PCOPERATOR pStackOperator = pStackToken->u.pOperator;
                    if (   (pOperator->Direction == enmDirLeft && pOperator->Priority <= pStackOperator->Priority)
                        || (pOperator->Direction == enmDirRight && pOperator->Priority < pStackOperator->Priority))
                    {
                        DEBUGPRINTF(("Moving operator '%s' cParams=%d from stack to queue\n", pStackToken->u.pOperator->pszOperator,
                                    pStackToken->u.pOperator->cParams));
                        StackPop(&Stack);
                        QueueAdd(pQueue, pStackToken);
                    }
                    else
                        break;
                }

                DEBUGPRINTF(("Pushing operator '%s' (id=%d) cParams=%d to stack\n", pToken->u.pOperator->pszOperator,
                        pToken->u.pOperator->OperatorId, pToken->u.pOperator->cParams));
                StackPush(&Stack, pToken);
            }
        }
        else if (pToken->Type == enmTokenFunction)
        {
            DEBUGPRINTF(("Pushing function '%s' to stack\n", pToken->u.pFunction->pszFunction));
            StackPush(&Stack, pToken);
        }
        else if (pToken->Type == enmTokenVariable)
        {
            if (rc == RERR_VARIABLE_NAME_TOO_LONG)
            {
                DEBUGPRINTF(("Variable name '%s' too long\n", pToken->u.pVariable->szVariable));
                MemFree(pToken);
                EvaluatorCleanUp(pEval, &Stack);
                return rc;
            }
            else if (rc == RERR_VARIABLE_NAME_INVALID)
            {
                DEBUGPRINTF(("Variable name '%s' invalid\n", pToken->u.pVariable->szVariable));
                MemFree(pToken);
                EvaluatorCleanUp(pEval, &Stack);
                return rc;
            }

            DEBUGPRINTF(("Adding variable '%s' to queue\n", pToken->szVariable));
            QueueAdd(pQueue, pToken);
        }
        else if (pToken->Type == enmTokenCommand)
        {
            DEBUGPRINTF(("Adding command '%s' to queue\n", pToken->u.pCommand->pszCommand));
            QueueAdd(pQueue, pToken);

            /*
             * Evaluate the rest of the expression as an argument to the command token if any.
             */
            const char *pszRightExpr = pszEnd;
            if (*pszRightExpr)
            {
                DEBUGPRINTF(("Command: -- Parsing subexpression '%s'\n", pszRightExpr));
                EVALUATOR SubExprEval;
                EvaluatorInitInternal(&SubExprEval);
                int rc2 = EvaluatorParse(&SubExprEval, pszRightExpr);
                if (RC_SUCCESS(rc2))
                {
                    DEBUGPRINTF(("Command: -- Done subexpression evaluation '%s'\n", pszRightExpr));

                    if (SubExprEval.Result.fVariableAssignment)
                    {
                        MemFree(pToken);
                        EvaluatorCleanUp(pEval, &Stack);
                        EvaluatorDestroy(&SubExprEval);
                        return RERR_CANT_ASSIGN_VARIABLE_FOR_COMMAND;
                    }

                    Assert(!pToken->pszCommandExpr);
                    pToken->pszCommandExpr = pszRightExpr;
                    DEBUGPRINTF(("pszCommandExpr=%s\n", pToken->pszCommandExpr));
                    if (!pToken->pszCommandExpr)
                    {
                        MemFree(pToken);
                        EvaluatorCleanUp(pEval, &Stack);
                        EvaluatorDestroy(&SubExprEval);
                        return RERR_NO_MEMORY;
                    }

                    /*
                     * Evaluate it and add resulting token to the RPN queue.
                     */
                    rc2 = EvaluatorEvaluate(&SubExprEval);
                    if (RC_SUCCESS(rc2))
                    {
                        if (SubExprEval.Result.fCommandEvaluated)
                        {
                            MemFree(pToken);
                            EvaluatorCleanUp(pEval, &Stack);
                            EvaluatorDestroy(&SubExprEval);
                            return RERR_INVALID_COMMAND_PARAMETER;
                        }

                        PTOKEN pParamToken = MemAlloc(sizeof(TOKEN));
                        if (!pParamToken)
                        {
                            MemFree(pToken);
                            EvaluatorCleanUp(pEval, &Stack);
                            EvaluatorDestroy(&SubExprEval);
                            return RERR_NO_MEMORY;
                        }
                        pParamToken->Type = enmTokenNumber;
                        pParamToken->u.Number.uValue = SubExprEval.Result.uValue;
                        pParamToken->u.Number.dValue = SubExprEval.Result.dValue;
                        pToken->pvCommandParamToken = pParamToken;
                    }
                    else
                    {
                        MemFree(pToken);
                        EvaluatorCleanUp(pEval, &Stack);
                        EvaluatorDestroy(&SubExprEval);
                        return RERR_INVALID_PARAMETER;
                    }
                }
                else
                {
                    MemFree(pToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    EvaluatorDestroy(&SubExprEval);
                    return RERR_EXPRESSION_INVALID;
                }
                EvaluatorDestroy(&SubExprEval);
            }
            else
            {
                Assert(!pToken->pszCommandExpr);
                Assert(!pToken->pvCommandParamToken);
                DEBUGPRINTF(("Command: -- Parsing completed sucessfully no params.\n"));
            }

            /*
             * Stop parsing now that we've parsed a command and prepared any parameters tokens.
             */
            break;
        }
        else
        {
            DEBUGPRINTF(("unknown!\n"));
            MemFree(pToken);
            pToken = NULL;
            break;
        }
        pszExpr = pszEnd;
        pPreviousToken = pToken;
    }

    /*
     * Pop remainder operators/functions to the queue.
     */
    while ((pToken = StackPop(&Stack)) != NULL)
    {
        /*
         * An open parenthesis on the stop of stack means we've got
         * unbalanced parenthesis, just get 0wt.
         */
        if (   StackSize(&Stack) == 0
            && TokenIsOpenParenthesis(pToken))
        {
            DEBUGPRINTF(("Unbalanced paranthesis\n"));
            MemFree(pToken);
            return RERR_PARENTHESIS_UNBALANCED;
        }

        DEBUGPRINTF(("Popping stack and adding to queue\n"));
        QueueAdd(pQueue, pToken);
    }

    /*
     * Clear old queue if any, and store the new one.
     */
    PQUEUE pPrevQueue = (PQUEUE)pEval->pvRPNQueue;
    if (pPrevQueue)
    {
        while ((pToken = QueueRemove(pPrevQueue)) != NULL)
            MemFree(pToken);
        MemFree(pPrevQueue);
    }
    pEval->pvRPNQueue = pQueue;

    if (QueueSize(pQueue) == 0)
    {
        EvaluatorCleanUp(pEval, &Stack);
        DEBUGPRINTF(("Error, no tokens detected!\n"));
        return RERR_EXPRESSION_INVALID;
    }
    return RINF_SUCCESS;
}


/**
 * Evaluates an internal representation of a parsed expression. The logic is
 * reverse polish notation evaluation but modified to support variables, variable
 * parameters to functions and more.
 *
 * @return  Status code on the result of the evaluation.
 * @param   pEval   The Evaluator object.
 */
int EvaluatorEvaluate(PEVALUATOR pEval)
{
    Assert(pEval);
    AssertReturn(pEval->u32Magic == RMAG_EVALUATOR, RERR_BAD_MAGIC);

    PQUEUE pQueue = pEval->pvRPNQueue;
    if (!pQueue)
        return RERR_INVALID_RPN;

    STACK Stack;
    StackInit(&Stack);
    PTOKEN pToken = NULL;

    /*
     * Evaluate RPN from Queue to the Stack.
     */
    DEBUGPRINTF(("EvaluatorEvaluate: RPN: \n"));
    while ((pToken = QueueRemove(pQueue)) != NULL)
    {
        if (pToken->Type == enmTokenNumber)
        {
            DEBUGPRINTF(("Number: (U=%" FMT_U64_NAT " F=%" FMT_FLT_NAT ") ", pToken->u.Number.uValue, pToken->u.Number.dValue));
            StackPush(&Stack, pToken);
        }
        else if (pToken->Type == enmTokenOperator)
        {
            PCOPERATOR pOperator = pToken->u.pOperator;
            DEBUGPRINTF(("%s ", pOperator->pszOperator));
            if (StackSize(&Stack) < pOperator->cParams)
            {
                DEBUGPRINTF(("Error StackSize=%u Operator '%s' cParams=%d\n",
                                    (unsigned)StackSize(&Stack), pOperator->pszOperator, pOperator->cParams));

                /*
                 * Insufficient parameters to operator.
                 * Destroy remained of the queue, the stack and bail.
                 */
                MemFree(pToken);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_TOO_FEW_PARAMETERS;
            }

            /*
             * Construct the array of PTOKENS parameters and pass it to the Operator evaluator if any, otherwise
             * just push the first parameter as the result.
             */
            PTOKEN apTokens[MAX_OPERATOR_PARAMETERS];
            memset(apTokens, 0, sizeof(apTokens));
            Assert(pOperator->cParams <= MAX_OPERATOR_PARAMETERS);
            int rc = RINF_SUCCESS;
            PTOKEN pResultantToken = NULL;
            for (int i = 0; i < pOperator->cParams; i++)
            {
                apTokens[i] = StackPop(&Stack);

                /*
                 * Check if operator can cast to required type to perform it's operation.
                 * If not, we cannot proceed because it would invoke undefined behaviour.
                 */
                if (   pOperator->fUIntParams
                    && !CanCastToken(apTokens[i], (long double)INT64_MIN, (long double)UINT64_MAX))
                {
                    /*
                     * Bleh, operator cannot handle this big a number. Exit, stage whatever.
                     */
                    rc = RERR_UNDEFINED_BEHAVIOUR;
                    DEBUGPRINTF(("Operand to '%s' cannot be cast to integer without UB. rc=%d\n", pOperator->pszOperator, rc));
                    MemFree(pToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    return rc;
                }
            }

            if (RC_SUCCESS(rc))
            {
                if (pOperator->pfnOperator)
                {
                    EvaluatorInvertTokenArray(apTokens, pOperator->cParams);
                    rc = pOperator->pfnOperator(pEval, apTokens);
                    if (RC_SUCCESS(rc))
                        pResultantToken = apTokens[0];
                }
                else
                    pResultantToken = apTokens[0];
            }

            if (   RC_SUCCESS(rc)
                && pResultantToken)
            {
                for (int i = 1; i < pOperator->cParams; i++)
                    MemFree(apTokens[i]);
                StackPush(&Stack, pResultantToken);
            }
            else
            {
                DEBUGPRINTF(("Operator '%s' on given operands failed. rc=%d\n", pOperator->pszOperator, rc));
                MemFree(pToken);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_BASIC_OPERATOR_MISSING;
            }
        }
        else if (pToken->Type == enmTokenFunction)
        {
            PCFUNCTION pFunction = pToken->u.pFunction;
            DEBUGPRINTF(("%s ", pFunction->pszFunction));
            if (StackSize(&Stack) < pToken->cFunctionParams)
            {
                DEBUGPRINTF(("Error StackSize=%u Function '%s' cParams=%d cMinParams=%d cMaxParams=%d\n",
                             (unsigned)StackSize(&Stack), pFunction->pszFunction, pToken->cFunctionParams,
                             pFunction->cMinParams, pFunction->cMaxParams));

                /*
                 * Insufficient parameters to function. Buh bye.
                 */
                MemFree(pToken);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_TOO_FEW_PARAMETERS;
            }

            /*
             * Construct an array of maximum possible PTOKENS parameters and
             * pass it to the Function evaluator if any, otherwise
             * just push the first parameter as the result.
             */
            Assert(pFunction->cMaxParams <= MAX_FUNCTION_PARAMETERS);
            uint32_t cParams = R_MIN(pToken->cFunctionParams, MAX_FUNCTION_PARAMETERS);
            PTOKEN *papTokens = MemAlloc(cParams * sizeof(TOKEN));
            if (!papTokens)
            {
                MemFree(pToken);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_NO_MEMORY;
            }

            int rc = RERR_UNDEFINED;
            for (uint32_t i = 0; i < cParams; i++)
            {
                papTokens[i] = StackPop(&Stack);

                /*
                 * Check if function can cast to required type to perform it's operation.
                 * If not, we cannot proceed because it would invoke undefined behaviour.
                 */
                if (   pFunction->fUIntParams
                    && !CanCastToken(papTokens[i], (long double)INT64_MIN, (long double)UINT64_MAX))
                {
                    /*
                     * Bleh, function cannot handle this big a number. 0wT.
                     */
                    rc = RERR_UNDEFINED_BEHAVIOUR;
                    DEBUGPRINTF(("Parameter to '%s' cannot be cast to integer without UB. rc=%d\n", pFunction->pszFunction, rc));
                    MemFree(pToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    return rc;
                }
            }

            PTOKEN pResultantToken = NULL;
            if (pFunction->pfnFunction)
            {
                EvaluatorInvertTokenArray(papTokens, cParams);
                rc = pFunction->pfnFunction(pEval, papTokens, cParams);
                if (RC_SUCCESS(rc))
                    pResultantToken = papTokens[0];
            }
            else
                pResultantToken = papTokens[0];

            if (pResultantToken)
            {
                for (uint32_t k = 1; k < cParams; k++)
                    MemFree(papTokens[k]);

                MemFree(papTokens);
                StackPush(&Stack, pResultantToken);
            }
            else
            {
                Assert(RC_FAILURE(rc));
                DEBUGPRINTF(("Function '%s' on given operands failed! rc=%d\n", pFunction->pszFunction, rc));
                MemFree(pToken);
                MemFree(papTokens);
                EvaluatorCleanUp(pEval, &Stack);
                return rc;
            }
        }
        else if (pToken->Type == enmTokenCommand)
        {
            PCOMMAND pCommand = pToken->u.pCommand;
            Assert(pCommand);

            DEBUGPRINTF(("EvaluatorEvaluate: Command %s\n", pCommand->pszCommand));

            char *pszResult  = NULL;
            PTOKEN pParamToken = NULL;
            int rc;
            if (pToken->pvCommandParamToken)
            {
                pParamToken = pToken->pvCommandParamToken;
                if (!TokenIsNumber(pParamToken))
                {
                    DEBUGPRINTF(("Not a number token for command argument.\n"));
                    MemFree(pToken);
                    MemFree(pParamToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_INVALID_COMMAND_PARAMETER;
                }
            }

            rc = pCommand->pfnCommand(pEval, pParamToken, &pszResult);
            if (RC_SUCCESS(rc))
            {
                pEval->Result.fCommandEvaluated = true;
                StrCopy(pEval->Result.szCommand, sizeof(pEval->Result.szCommand), pCommand->pszCommand);
                if (pszResult)
                {
                    StrCopy(pEval->Result.szCommandResult, sizeof(pEval->Result.szCommandResult), pszResult);
                    StrFree(pszResult);
                }
            }
            else
            {
                pEval->Result.fCommandEvaluated = false;
                rc = RERR_COMMAND_FAILED;
            }

            MemFree(pToken);
            if (pParamToken)
                MemFree(pParamToken);
            EvaluatorCleanUp(pEval, &Stack);
            break;
        }
        else if (pToken->Type == enmTokenVariable)
        {
            PVARIABLE pVariable = pToken->u.pVariable;
            if (!pVariable)
            {
                /*
                 * Try find the variable if the Variable token was created BEFORE the creation of
                 * the variable entry. e.g. _a=_b+1, _b=5, _a and we are evaluating "_a" now whose
                 * RPN Token "_b" has no association with the global variable entry "_b" yet.
                 */
                pToken->u.pVariable = EvaluatorFindVariable(pToken->szVariable, &g_VarList);
                if (!pToken->u.pVariable)
                {
                    StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), pToken->szVariable);
                    EvaluatorCleanVariables();
                    MemFree(pToken);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_VARIABLE_UNDEFINED;
                }

                pVariable = pToken->u.pVariable;
            }

            PQUEUE pVarQueue = pToken->u.pVariable->pvRPNQueue;
            if (!pVarQueue)
            {
                /*
                 * Huh? User typed probably typed some crap and we formed variables out of it.
                 * Delete them and bail.
                 */
                EvaluatorCleanVariables();
                MemFree(pToken);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_EXPRESSION_INVALID;
            }

            /*
             * Avoid circular variable dependencies. We know which variable we are evaluating, add it to
             * a list of variables. As we recurse into sub-variables we check the list to make sure we are
             * not evaluating a variable being evaluated.
             */
            PVARIABLE pAncestorVar = EvaluatorFindVariable(pToken->u.pVariable->szVariable, &pEval->VarList);
            if (!pAncestorVar)
            {
                /*
                 * Evaluating an RPN Queue 'consumes' the queue. So let's copy it to leave the original
                 * Variable entry's RPN queue unmodified for further evaluations.
                 */
                PQUEUE pEvalQueue = MemAlloc(sizeof(QUEUE));    /* fix this shit. make pvRPNQueue as a stack variable */
                QueueInit(pEvalQueue);
                for (uint32_t i = 0; i < QueueSize(pVarQueue); i++)
                {
                    PTOKEN pSrcToken = QueueItemAt(pVarQueue, i);
                    Assert(pSrcToken);
                    PTOKEN pTmpToken = TokenDup(pSrcToken);
                    if (!pTmpToken)
                    {
                        MemFree(pToken);
                        EvaluatorCleanUp(pEval, &Stack);
                        return RERR_NO_MEMORY;
                    }
                    QueueAdd(pEvalQueue, pTmpToken);
                }

                /*
                 * Construct a temporary Evaluator object and try evaluate the Variable.
                 */
                EVALUATOR VarEval;
                EvaluatorInitInternal(&VarEval);
                VarEval.pvRPNQueue = pEvalQueue;
                ListAppend(&VarEval.VarList, &pEval->VarList);
                ListAdd(&VarEval.VarList, pVariable);

                DEBUGPRINTF(("Evaluating variable '%s'\n", pToken->u.pVariable->szVariable));
#ifdef _DEBUG
                EvaluatorPrintVarList(&VarEval.VarList);
#endif
                int rc = EvaluatorEvaluate(&VarEval);
                if (RC_SUCCESS(rc))
                {
                    /*
                     * Reuse Variable Token as a Number Token & push it to the Stack.
                     */
                    DEBUGPRINTF(("Variable '%s' is %" FMT_FLT_NAT "\n", pToken->u.pVariable->szVariable, VarEval.Result.dValue));
                    pToken->Type = enmTokenNumber;
                    pToken->u.Number.uValue = VarEval.Result.uValue;
                    pToken->u.Number.dValue = VarEval.Result.dValue;

                    /*
                     * Remove the variable from the dependency list.
                     */
                    StackPush(&Stack, pToken);
                    EvaluatorDestroy(&VarEval);
                }
                else
                {
                    /** Do -NOT- alter rc, it could be circular dependency error. */
                    DEBUGPRINTF(("Failed to evaluate right-hand expression for variable '%s'\n", pToken->u.pVariable->szVariable));
                    StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), VarEval.Result.szVariable);
                    MemFree(pToken);
                    EvaluatorDestroy(&VarEval);
                    EvaluatorCleanUp(pEval, &Stack);
                    return rc;
                }
            }
            else
            {
                DEBUGPRINTF(("Circular variable depedency on variable '%s'\n", pAncestorVar->szVariable));
                StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), pAncestorVar->szVariable);
                MemFree(pToken);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_CIRCULAR_DEPENDENCY;
            }
        }
        else
        {
            DEBUGPRINTF(("UnknownToken!\n"));
            MemFree(pToken);
        }
    }

    DEBUGPRINTF(("\n"));

    /*
     * If a command is evaluated successfully, stop evaluating and break.
     */
    if (pEval->Result.fCommandEvaluated)
    {
        DEBUGPRINTF(("fCommandEvaluated\n"));
        /** @todo Anything to cleanup? */
        pEval->Result.ErrorIndex = -1;
        return RINF_SUCCESS;
    }

    /*
     * Result is on the stack, otherwise errors.
     */
    if (StackSize(&Stack) == 1)
    {
        pToken = StackPop(&Stack);
        DEBUGPRINTF(("Result: (U=%" FMT_U64_NAT " F=%" FMT_FLT_NAT ")\n", pToken->u.Number.uValue, pToken->u.Number.dValue));
        pEval->Result.ErrorIndex = -1;
        pEval->Result.uValue = pToken->u.Number.uValue;
        pEval->Result.dValue = pToken->u.Number.dValue;
        MemFree(pToken);
        return RINF_SUCCESS;
    }
    else
        DEBUGPRINTF(("Here\n"));

    while ((pToken = StackPop(&Stack)) != NULL)
    {
        DEBUGPRINTF(("PTOKEN free (U=%" FMT_U64_NAT " F=%" FMT_FLT_NAT ")\n", pToken->u.Number.uValue, pToken->u.Number.dValue));
        MemFree(pToken);
    }

    DEBUGPRINTF(("Too many tokens, invalid expression\n"));
    pEval->Result.ErrorIndex = -1;
    return RERR_EXPRESSION_INVALID;
}


/**
 * Destroys the Evaluator object.
 *
 * @param   pEval   The Evaluator object, cannot be NULL.
 */
void EvaluatorDestroy(PEVALUATOR pEval)
{
    /*
     * Free the RPN queue, don't destroy the global data.
     */
    EvaluatorCleanUp(pEval, NULL /* pStack */);
    pEval->u32Magic = ~(RMAG_EVALUATOR);
}


/**
 * Initializes the Evaluator object.
 *
 * @return  Status code of initialization.
 * @param   pEval       The Evaluator object, cannot be NULL.
 * @param   pszError    Where to write a descriptive error if one should occur while initializing.
 * @param   cbError     Size of the @pszError buffer including NULL terminator.
 */
int EvaluatorInit(PEVALUATOR pEval, char *pszError, size_t cbError)
{
    Assert(pEval);

    /*
     * Dry run of operators to detect invalid, multiple definitions and conflicts.
     */
    pEval->u32Magic = ~(RMAG_EVALUATOR);
    for (unsigned i = 0; i < g_cOperators; i++)
    {
        PCOPERATOR pOperator = &g_aOperators[i];
        if (   !pOperator->pszOperator
            || !pOperator->pszSyntax
            || !pOperator->pszDesc)
        {
            StrNPrintf(pszError, cbError, "Operator with missing name/syntax or description. index=%d id=%d.", i, pOperator->OperatorId);
            return RERR_INVALID_OPERATOR;
        }

        if (isdigit(*pOperator->pszOperator) || *pOperator->pszOperator == '.')
        {
            StrNPrintf(pszError, cbError, "Invalid operator name '%s' at [%d] id=%d.", pOperator->pszOperator, i, pOperator->OperatorId);
            return RERR_INVALID_OPERATOR;
        }

        /*
         * Let us for now only allow Operators taking 2 or lower parameters.
         * The evaluation logic can of course handle any number of parameters
         * but we don't have a parameter separator for Operators unlike Functions.
         */
        if (pOperator->cParams > 2)
        {
            StrNPrintf(pszError, cbError, "Operator '%s' at [%d] exceeds maximum parameter limit of 2.", pOperator->pszOperator, i);
            return RERR_INVALID_OPERATOR;
        }

        for (unsigned k = 0; k < g_cOperators; k++)
        {
            if (i == k)
                continue;

            PCOPERATOR pCur = &g_aOperators[k];

            /*
             * Make sure each operator Id is unique.
             */
            if (pOperator->OperatorId == pCur->OperatorId)
            {
                StrNPrintf(pszError, cbError, "Duplicate operator Id=%d '%s' at [%d] and '%s' at [%d].", pOperator->OperatorId,
                           pOperator->pszOperator, i, pCur->pszOperator, k);
                return RERR_CONFLICTING_OPERATORS;
            }

            if (   !StrCmp(pOperator->pszOperator, pCur->pszOperator)
                && pOperator->Direction == pCur->Direction)
            {
                if (pOperator->cParams == pCur->cParams)
                {
                    StrNPrintf(pszError, cbError, "Duplicate operator '%s' at [%d] and [%d].", pOperator->pszOperator, i, k);
                    return RERR_DUPLICATE_OPERATOR;
                }

                StrNPrintf(pszError, cbError, "Conflicting operator '%s' at [%d] and [%d].", pOperator->pszOperator, i, k);
                return RERR_CONFLICTING_OPERATORS;
            }
        }
    }

    /*
     * Sort operator list to workaround overlapping operators names, eg: "++" always appears before "+".
     * Also sort such that operators with the same name, the operator with more parameters is sorted first.
     * For e.g. binary '-' before unary '-', the reason for this is to parse the correct operator
     * (see EvaluatorParseOperator).
     * Also locate open and close parenthesis.
     */
    qsort(g_aOperators, g_cOperators, sizeof(OPERATOR), OperatorSortCompare);
    DEBUGPRINTF(("Sorted Operators\n"));
    PCOPERATOR pOperatorParamSeparator = NULL;
    for (unsigned i = 0; i < g_cOperators; i++)
    {
        DEBUGPRINTF(("%s cParams=%d Id=%d\n", g_aOperators[i].pszOperator, g_aOperators[i].cParams, g_aOperators[i].OperatorId));
        if (OperatorIsOpenParenthesis(&g_aOperators[i]))
            g_pOperatorOpenParenthesis = &g_aOperators[i];
        else if (OperatorIsCloseParenthesis(&g_aOperators[i]))
            g_pOperatorCloseParenthesis = &g_aOperators[i];
        else if (OperatorIsParamSeparator(&g_aOperators[i]))
            pOperatorParamSeparator = &g_aOperators[i];
    }

    if (!g_pOperatorOpenParenthesis)
    {
        StrNPrintf(pszError, cbError, "Extreme error! Open parenthesis operator not found!");
        return RERR_BASIC_OPERATOR_MISSING;
    }

    if (!g_pOperatorCloseParenthesis)
    {
        StrNPrintf(pszError, cbError, "Extreme error! Close parenthesis operator not found!");
        return RERR_BASIC_OPERATOR_MISSING;
    }

    if (!pOperatorParamSeparator)
    {
        StrNPrintf(pszError, cbError, "Extreme error! Parameter separator operator not found!");
        return RERR_BASIC_OPERATOR_MISSING;
    }

    /*
     * Check for function duplicates.
     */
    for (unsigned i = 0; i < g_cFunctions; i++)
    {
        PCFUNCTION pFunction = &g_aFunctions[i];
        for (unsigned k = 0; k < g_cFunctions; k++)
        {
            if (i == k)
                continue;

            PCFUNCTION pCur = &g_aFunctions[k];
            if (!StrCmp(pCur->pszFunction, pFunction->pszFunction))
            {
                StrNPrintf(pszError, cbError, "Function '%s' is duplicated. at [%d] and [%d].", pFunction->pszFunction, i, k);
                return RERR_DUPLICATE_FUNCTION;
            }
        }
    }

    /*
     * Sort function list to workaround overlapping function names, eg: "sqr" and "sqrt".
     */
    qsort(g_aFunctions, g_cFunctions, sizeof(FUNCTION), FunctionSortCompare);
    DEBUGPRINTF(("Sorted Functions\n"));
    for (unsigned i = 0; i < g_cFunctions; i++)
    {
        if (   !g_aFunctions[i].pszFunction
            || !g_aFunctions[i].pszSyntax
            || !g_aFunctions[i].pszDesc)
        {
            StrNPrintf(pszError, cbError, "Function with missing name/syntax or description. index=%d.", i);
            return RERR_INVALID_FUNCTION;
        }

        DEBUGPRINTF(("%s cMinParams=%d cMaxParams=%d\n", g_aFunctions[i].pszFunction, g_aFunctions[i].cMinParams, g_aFunctions[i].cMaxParams));
    }

    /*
     * Create alphabetically sorted Function list (for help listing)
     */
    if (g_paSortedFunctions)
        MemFree(g_paSortedFunctions);
    g_paSortedFunctions = MemAlloc(sizeof(FUNCTION) * g_cFunctions);
    if (!g_paSortedFunctions)
        return RERR_NO_MEMORY;

    MemCpy(g_paSortedFunctions, &g_aFunctions, sizeof(FUNCTION) * g_cFunctions);
    qsort(g_paSortedFunctions, g_cFunctions, sizeof(FUNCTION), AscendingFunctionSortCompare);

    /*
     * Create alphabetically sorted Command list (for help listing)
     */
    /** @todo  */

    EvaluatorInitInternal(pEval);
    return RINF_SUCCESS;
}


/*******************************************************************************
 *   Hello, Operator?!                                                         *
 *******************************************************************************/

static int OpAdd(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue + apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.dValue + apTokens[1]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int OpSubtract(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue - apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.dValue - apTokens[1]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int OpNegate(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = -apTokens[0]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = -apTokens[0]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int OpMultiply(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue * apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.dValue * apTokens[1]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int OpDivide(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue / apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.dValue / apTokens[1]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int OpIncrement(PEVALUATOR pEval, PTOKEN apTokens[])
{
    ++apTokens[0]->u.Number.uValue;
    ++apTokens[0]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int OpDecrement(PEVALUATOR pEval, PTOKEN apTokens[])
{
    --apTokens[0]->u.Number.uValue;
    --apTokens[0]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int OpShiftLeft(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue << apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = (uint64_t)apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpShiftRight(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue >> apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = (uint64_t)apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpBitNegate(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = ~apTokens[0]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = (uint64_t)apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpModulo(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue % apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpLessThan(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = !!(apTokens[0]->u.Number.uValue < apTokens[1]->u.Number.uValue);
    apTokens[0]->u.Number.dValue = (long double)DefinitelyLessThan(apTokens[0]->u.Number.dValue, apTokens[1]->u.Number.dValue);
    return RINF_SUCCESS;
}

static int OpGreaterThan(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = !!(apTokens[0]->u.Number.uValue > apTokens[1]->u.Number.uValue);
    apTokens[0]->u.Number.dValue = (long double)DefinitelyGreaterThan(apTokens[0]->u.Number.dValue, apTokens[1]->u.Number.dValue);
    return RINF_SUCCESS;
}

static int OpEqualTo(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = !!(apTokens[0]->u.Number.uValue == apTokens[1]->u.Number.uValue);
    apTokens[0]->u.Number.dValue = (long double)EssentiallyEqual(apTokens[0]->u.Number.dValue, apTokens[1]->u.Number.dValue);
    return RINF_SUCCESS;
}

static int OpLessThanOrEqualTo(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = !!(apTokens[0]->u.Number.uValue <= apTokens[1]->u.Number.uValue);
    bool const fLessThan = DefinitelyLessThan(apTokens[0]->u.Number.dValue, apTokens[1]->u.Number.dValue);
    bool const fEqualTo  = EssentiallyEqual(apTokens[0]->u.Number.dValue, apTokens[1]->u.Number.dValue);
    apTokens[0]->u.Number.dValue = (long double)(fLessThan || fEqualTo);
    return RINF_SUCCESS;
}

static int OpGreaterThanOrEqualTo(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = !!(apTokens[0]->u.Number.uValue >= apTokens[1]->u.Number.uValue);
    bool const fGreaterThan = DefinitelyGreaterThan(apTokens[0]->u.Number.dValue, apTokens[1]->u.Number.dValue);
    bool const fEqualTo = EssentiallyEqual(apTokens[0]->u.Number.dValue, apTokens[1]->u.Number.dValue);
    apTokens[0]->u.Number.dValue = (long double)(fGreaterThan || fEqualTo);
    return RINF_SUCCESS;
}

static int OpNotEqualTo(PEVALUATOR pEval, PTOKEN apTokens[])
{
    int rc = OpEqualTo(pEval, apTokens);
    if (RC_SUCCESS(rc))
    {
        apTokens[0]->u.Number.uValue = !apTokens[0]->u.Number.uValue;
        apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    }
    return RINF_SUCCESS;
}

static int OpLogicalNot(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = !apTokens[0]->u.Number.dValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpBitwiseAnd(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue & apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpBitwiseXor(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue ^ apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpBitwiseOr(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = apTokens[0]->u.Number.uValue | apTokens[1]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpLogicalAnd(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = (apTokens[0]->u.Number.uValue && apTokens[1]->u.Number.dValue);
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int OpLogicalOr(PEVALUATOR pEval, PTOKEN apTokens[])
{
    apTokens[0]->u.Number.uValue = (apTokens[0]->u.Number.uValue || apTokens[1]->u.Number.dValue);
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}


/**
 * Searches the global list of function for commands.
 * @todo later this must probably be part of a Evaluator object so that
 * it may find variables? Probably but that's after I implemented proper
 * variable support.
 *
 * @return  Pointer to the full command
 * @param   pszCommand  Partial command to search for.
 * @param   cchCommand  Number of characters in command to search. Usually this is
 *                      the string length since @a pszCommand itself is partial.
 * @param   iStart      Starting index to search from.
 * @param   piEnd       Where to store the last index from the search.
 */
const char *EvaluatorFindFunction(const char *pszCommand, uint32_t cchCommand, uint32_t iStart, uint32_t *piEnd)
{
    for (uint32_t i = iStart; i < g_cFunctions; i++)
    {
        PCFUNCTION pFunction = &g_aFunctions[i];
        if (!pFunction)
            break;

        *piEnd = i + 1;
        if (!StrNCmp(pszCommand, pFunction->pszFunction, cchCommand))
            return pFunction->pszFunction;
    }

    return NULL;
}


/**
 * Returns the syntax and description of a function.
 *
 * @return  RINF_SUCCESS on success, othwerise appropriate status code.
 * @param   uIndex      The index of the requested function.
 * @param   ppszName    Where to store the name of the function, caller frees with StrFree()
 * @param   ppszSyntax  Where to store the syntax for the function, caller frees with StrFree()
 * @param   ppszHelp    Where to store the description for the function, caller frees with StreFree()
 */
int EvaluatorFunctionHelp(unsigned uIndex, char **ppszName, char **ppszSyntax, char **ppszHelp)
{
    if (uIndex > g_cFunctions)
        return RERR_NO_DATA;

    *ppszName   = StrDup(g_paSortedFunctions[uIndex].pszFunction);
    *ppszSyntax = StrDup(g_paSortedFunctions[uIndex].pszSyntax);
    *ppszHelp   = StrDup(g_paSortedFunctions[uIndex].pszDesc);

    return RINF_SUCCESS;
}

/**
 * Returns the total number of functions.
 *
 * @return  The total number of functions.
 */
unsigned EvaluatorFunctionCount(void)
{
    return g_cFunctions;
}


/**
 * Returns the total number of commands.
 *
 * @return  The total number of commands.
 */
unsigned EvaluatorCommandCount(void)
{
    return g_cCommands;
}


/**
 * Returns the syntax and description of an operator.
 *
 * @return  RINF_SUCCESS on success, othwerise appropriate status code.
 * @param   uIndex          The index of the requested operator.
 * @param   ppszName        Where to store the name of the operator, caller frees
 *                          with StrFree().
 * @param   ppszSyntax      Where to store the syntax for the operator, caller frees
 *                          with StrFree().
 * @param   ppszHelp        Where to store the description for the operator, caller
 *                          frees with StreFree().
 */
int EvaluatorOperatorHelp(unsigned uIndex, char **ppszName, char **ppszSyntax, char **ppszHelp)
{
    if (uIndex > g_cOperators)
        return RERR_NO_DATA;

    *ppszName   = StrDup(g_aOperators[uIndex].pszOperator);
    *ppszSyntax = StrDup(g_aOperators[uIndex].pszSyntax);
    *ppszHelp   = StrDup(g_aOperators[uIndex].pszDesc);

    return RINF_SUCCESS;
}


/**
 * Finds a variable for the given index.
 *
 * @return  RINF_SUCCESS on success, otherwise appropriate status code.
 * @param   uIndex      The index of the requested variable.
 * @param   ppszName    Where to store the name of the variable, caller frees with
 *                      StrFree().
 * @param   ppszExpr    Where to store the expression assigned to the variable,
 *                      caller frees with StrFree().
 */
int EvaluatorVariableValue(unsigned uIndex, char **ppszName, char **ppszExpr)
{
    if (uIndex >= ListSize(&g_VarList))
        return RERR_NO_DATA;
    PVARIABLE pVariable = ListItemAt(&g_VarList, uIndex);
    Assert(pVariable);
    *ppszName = StrDup(pVariable->szVariable);
    *ppszExpr = StrDup(pVariable->pszExpr);
    return RINF_SUCCESS;
}


/**
 * Returns the total number of operators.
 *
 * @return  The total number of operators.
 */
unsigned EvaluatorOperatorCount(void)
{
    return g_cOperators;
}


/**
 * Initializes the globals.
 *
 * @return  RINF_SUCCESS on success, otherwise an appropriate status code.
 */
int EvaluatorInitGlobals(void)
{
    ListInit(&g_VarList);

    static struct
    {
        const char *pszVarName;     /* Name of global variable. */
        const char *pszExpr;        /* Expression to pre-evaluate and assign to variable. */
    } const s_aVars [] =
    {
        { "INT8_MAX",           "127" },
        { "UINT8_MAX",          "255" },
        { "INT16_MAX",          "32767" },
        { "UINT16_MAX",         "65535" },
        { "INT32_MAX",          "2147483647" },
        { "UINT32_MAX",         "4294967295"  },
        { "INT64_MAX",          "9223372036854775807" },
        { "UINT64_MAX",         "18446744073709551615" },
        { "_1K",                "0x00000400" },
        { "_4K",                "0x00001000" },
        { "_32K",               "0x00008000" },
        { "_64K",               "0x00010000" },
        { "_128K",              "0x00020000" },
        { "_256K",              "0x00040000" },
        { "_512K",              "0x00080000" },
        { "_1M",                "0x00100000" },
        { "_2M",                "0x00200000" },
        { "_4M",                "0x00400000" },
        { "_1G",                "0x40000000" },
        { "_2G",                "0x80000000" },
        { "_4G",                "0x0000000100000000" },
        { "_1T",                "0x0000010000000000" },
        { "_1P",                "0x0004000000000000" },
        { "_1E",                "0x1000000000000000" },
        { "_2E",                "0x2000000000000000" },
        { "PAGE_SHIFT",         "12" },
        { "PAGE_SIZE",          "4096" },
        { "PAGE_OFFSET_MASK",   "0xfff" },
    };

    EVALUATOR SubExprEval;
    EvaluatorInitInternal(&SubExprEval);
    for (size_t i = 0; i < R_ARRAY_ELEMENTS(s_aVars); i++)
    {
        PVARIABLE pVar = MemAlloc(sizeof(VARIABLE));
        if (!pVar)
        {
            EvaluatorDestroy(&SubExprEval);
            EvaluatorDestroyGlobals();
            return RERR_NO_MEMORY;
        }

        int rc = EvaluatorParse(&SubExprEval, s_aVars[i].pszExpr);
        if (RC_SUCCESS(rc))
        {
            StrCopy(pVar->szVariable, sizeof(pVar->szVariable), s_aVars[i].pszVarName);
            pVar->pszExpr = StrDup(s_aVars[i].pszExpr);

            /** @todo Transfer queue ownership to variable from SubExprEval. This is bad
             *        style, fix it later. */
            pVar->pvRPNQueue = SubExprEval.pvRPNQueue;
            pVar->fCanReinit = false;
            SubExprEval.pvRPNQueue = NULL;
            ListAdd(&g_VarList, pVar);
        }
        else
        {
            MemFree(pVar);

            /*
             * Free whatever was assigned so far.
             */
            PVARIABLE pVariable = NULL;
            while ((pVariable = ListRemoveItemAt(&g_VarList, 0)) != NULL)
                EvaluatorDestroyVariable(pVariable);

            DEBUGPRINTF(("Failed to parse expression for variable '%s' expr='%s' i=%u\n", pVar->szVariable, pVar->pszExpr, i));
            return RERR_VARIABLE_UNDEFINED;
        }
    }
    EvaluatorDestroy(&SubExprEval);

#ifdef _DEBUG
    EvaluatorPrintVarList(&g_VarList);
#endif
    return RINF_SUCCESS;
}


/**
 * Destroys the globals (currently just the variable data).
 */
void EvaluatorDestroyGlobals(void)
{
    /*
     * Free the queue of variables.
     */
    PVARIABLE pVariable = NULL;
    while ((pVariable = ListRemoveItemAt(&g_VarList, 0)) != NULL)
        EvaluatorDestroyVariable(pVariable);
}

