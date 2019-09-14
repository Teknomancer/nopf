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
#include "EvaluatorInternal.h"
#include "EvaluatorFunctors.h"
#include "EvaluatorCommands.h"
#include "Stack.h"
#include "Queue.h"
#include "List.h"
#include "Assert.h"
#include "GenericDefs.h"
#include "Errors.h"
#include "Magics.h"
#include "StringOps.h"
#include "InputOutput.h"

#include <errno.h>

/*******************************************************************************
 *   Globals, Typedefs & Defines                                               *
 *******************************************************************************/
PCOPERATOR g_pOperatorOpenParenthesis = NULL;
PCOPERATOR g_pOperatorCloseParenthesis = NULL;

int OpAdd(PEVALUATOR, PATOM[]);
int OpSubtract(PEVALUATOR, PATOM[]);
int OpNegate(PEVALUATOR, PATOM[]);
int OpMultiply(PEVALUATOR, PATOM[]);
int OpDivide(PEVALUATOR, PATOM[]);
int OpIncrement(PEVALUATOR, PATOM[]);
int OpDecrement(PEVALUATOR, PATOM[]);
int OpShiftLeft(PEVALUATOR, PATOM[]);
int OpShiftRight(PEVALUATOR, PATOM[]);
int OpBitNegate(PEVALUATOR, PATOM[]);
int OpModulo(PEVALUATOR, PATOM[]);
int OpLessThan(PEVALUATOR, PATOM[]);
int OpGreaterThan(PEVALUATOR, PATOM[]);
int OpEqualTo(PEVALUATOR, PATOM[]);
int OpLessThanOrEqualTo(PEVALUATOR, PATOM[]);
int OpGreaterThanOrEqualTo(PEVALUATOR, PATOM[]);
int OpNotEqualTo(PEVALUATOR, PATOM[]);
int OpLogicalNot(PEVALUATOR, PATOM[]);
int OpBitwiseAnd(PEVALUATOR, PATOM[]);
int OpBitwiseXor(PEVALUATOR, PATOM[]);
int OpBitwiseOr(PEVALUATOR, PATOM[]);
int OpLogicalAnd(PEVALUATOR, PATOM[]);
int OpLogicalOr(PEVALUATOR, PATOM[]);

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

/** Alphabetically sorted array of Functors */
PFUNCTOR g_paSortedFunctors = NULL;


/*******************************************************************************
*   Helper Functions                                                           *
*******************************************************************************/
static inline bool CanCastAtom(PCATOM pAtom, FLOAT MinValue, FLOAT MaxValue)
{
    return (DefinitelyLessThan(pAtom->u.dValue, MaxValue) && DefinitelyGreaterThan(pAtom->u.dValue, MinValue));
}

static inline void AtomInit(PATOM pAtom)
{
    pAtom->Type = enmAtomEmpty;
    pAtom->Position = 0;
}

static PATOM AtomDup(PCATOM pSrcAtom)
{
    PATOM pDstAtom = MemAlloc(sizeof(ATOM));
    if (pDstAtom)
    {
        pDstAtom->Type           = pSrcAtom->Type;
        pDstAtom->Position       = pSrcAtom->Position;
        pDstAtom->cFunctorParams = pSrcAtom->cFunctorParams;
        StrCopy(pDstAtom->szVariable, sizeof(pDstAtom->szVariable), pSrcAtom->szVariable);
        pDstAtom->u              = pSrcAtom->u; /* yeah implicit memcpy */
    }
    return pDstAtom;
}

static inline bool AtomIsOperator(PCATOM pAtom)
{
    return (pAtom && pAtom->Type == enmAtomOperator);
}

static inline bool AtomIsFunctor(PCATOM pAtom)
{
    return (pAtom && pAtom->Type == enmAtomFunctor);
}

static inline bool AtomIsVariable(PCATOM pAtom)
{
    return (pAtom && pAtom->Type == enmAtomVariable);
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

static inline bool NumberIsNegative(PATOM pAtom)
{
    return DefinitelyLessThan(pAtom->u.dValue, (FLOAT)0);
}

static inline bool AtomIsCloseParenthesis(PCATOM pAtom)
{
    if (   !pAtom
        || !AtomIsOperator(pAtom))
        return false;
    return OperatorIsCloseParenthesis(pAtom->u.pOperator);
}

static inline bool AtomIsOpenParenthesis(PCATOM pAtom)
{
    if (   !pAtom
        || !AtomIsOperator(pAtom))
        return false;
    return OperatorIsOpenParenthesis(pAtom->u.pOperator);
}

static inline bool AtomIsAssignment(PCATOM pAtom)
{
    if (   !pAtom
        || !AtomIsOperator(pAtom))
        return false;
    return OperatorIsAssignment(pAtom->u.pOperator);
}

static inline bool AtomIsParamSeparator(PCATOM pAtom)
{
    if (   !pAtom
        || !AtomIsOperator(pAtom))
        return false;
    return OperatorIsParamSeparator(pAtom->u.pOperator);
}

static inline bool AtomIsParenthesis(PCATOM pAtom)
{
    if (   !pAtom
        || !AtomIsOperator(pAtom))
        return false;
    return (AtomIsOpenParenthesis(pAtom) || AtomIsCloseParenthesis(pAtom));
}


static void EvaluatorInvertAtomArray(PATOM apAtoms[], uint32_t cAtoms)
{
    if (cAtoms < 2)
        return;

    uint32_t i = 0;
    PATOM pUnstableAtom = NULL;
    for (i = 0; i < cAtoms / 2; i++)
    {
        --cAtoms;
        pUnstableAtom = apAtoms[i];
        apAtoms[i] = apAtoms[cAtoms];
        apAtoms[cAtoms] = pUnstableAtom;
    }
}


/**
 * Searches for a variable and returns a Variable Atom if found.
 *
 * @param   pszVariable             Name of the variable to find.
 * @returns Pointer to an allocated Variable Atom or NULL if @a pszVariable could not be found.
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
 * @param   pVariable               Pointer to the Variable to destroy.
 */
static void EvaluatorDestroyVariable(PVARIABLE pVariable)
{
    if (pVariable)
    {
        if (pVariable->pvRPNQueue)
        {
            PATOM pAtom = NULL;
            while ((pAtom = (PATOM)QueueRemove(pVariable->pvRPNQueue)) != NULL)
                MemFree(pAtom);
        }
        MemFree(pVariable->pvRPNQueue);

        if (pVariable->pszExpr)
            StrFree(pVariable->pszExpr);

        MemFree(pVariable);
    }
}


/**
 * Cleans up the variable queue of unassigned variables.
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
void EvaluatorPrintVarList(PLIST pList)
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
 * Parses a number and returns a Numeric Atom.
 *
 * @param   pszExpr                 The whitespace skipped expression to parse.
 * @param   ppszEnd                 Where to store till what point in pszExpr was scanned.
 * @returns Pointer to an allocated Number Atom or NULL if @a pszExpr was not a number.
 */
static PATOM EvaluatorParseNumber(const char *pszExpr, const char **ppszEnd)
{
    char szNum[2048];
    int i = 0;
    const char *pszStart = pszExpr;
    int iRadix = 0;

    /*
     * Binary prefix.
     */
    if (*pszExpr == 'b' || *pszExpr == 'B')
    {
        ++pszExpr;
        while (*pszExpr)
        {
            if (*pszExpr == '1' || *pszExpr == '0')
                szNum[i++] = *pszExpr;
            else if (!isspace(*pszExpr))
                break;

            pszExpr++;
            if (i >= sizeof(szNum) - 1)
                break;
        }
        iRadix = 2;
    }
    else if (*pszExpr == '0')
    {
        /*
         * Octal prefix.
         */
        ++pszExpr;
        while (*pszExpr)
        {
            if (*pszExpr >= '0' && *pszExpr < '8')
                szNum[i++] = *pszExpr;
            else if (!isspace(*pszExpr))
                break;

            pszExpr++;
            if (i >= sizeof(szNum) - 1)
                break;
        }
        iRadix = 8;

        /*
         * Hexadecimal prefix.
         */
        if (   i == 0
            && (*pszExpr == 'x' || *pszExpr == 'X'))
        {
            ++pszExpr;
            while (*pszExpr)
            {
                if (   isdigit(*pszExpr)
                    || (*pszExpr >= 'a' && *pszExpr <= 'f')
                    || (*pszExpr >= 'A' && *pszExpr <= 'F'))
                {
                    szNum[i++] = *pszExpr;
                }
                else if (!isspace(*pszExpr))
                    break;

                pszExpr++;
                if (i >= sizeof(szNum) - 1)
                    break;
            }
            iRadix = 16;
        }
    }

    /*
     * If nothing has been accumulated in our 'szNum' array, we've not
     * got any recognized numeric under whatever radices we've checked.
     * Reset pointer to the original & try parsing without any prefix.
     */
    if (i == 0)
    {
        pszExpr = pszStart;
        iRadix = 0;

        /*
         * Hexadecimal sans prefix, or Decimal.
         */
        while (*pszExpr)
        {
            if (isdigit(*pszExpr))
            {
                szNum[i++] = *pszExpr;
            }
            else if (*pszExpr == '.')
            {
                if (iRadix == 0)    /* eg:  ".5" */
                {
                    szNum[i++] = *pszExpr;
                    iRadix = 10;
                }
                else                /* eg: "fa.5" or "10.5.5" */
                {
                    iRadix = -1;
                    break;
                }
            }
            else if (   (*pszExpr >= 'A' && *pszExpr <= 'F')
                     || (*pszExpr >= 'a' && *pszExpr <= 'f'))
            {
                if (iRadix != 10)   /* eg: "af" or "53a"  */
                    iRadix = 16;
                else                /* eg: ".af" */
                {
                    iRadix = -1;
                    break;
                }
                szNum[i++] = *pszExpr;
            }
            else if (!isspace(*pszExpr))
                break;

            pszExpr++;
            if (i >= sizeof(szNum) - 1)
                break;
        }

        if (i > 0 && iRadix == 0)
            iRadix = 10;
    }

    if (   i == 0
        || iRadix == 0)
    {
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
    szNum[i] = '\0';
    char *pszEndTmp = NULL;
    FLOAT dValue = 0;
    errno = 0;
    if (iRadix != 10)
        dValue = (FLOAT)strtoull(szNum, &pszEndTmp, iRadix);
    else
        dValue = (FLOAT)strtold(szNum, &pszEndTmp);

    /*
     * An error while converting the number.
     */
    if (errno)
    {
        DEBUGPRINTF(("Error while string to unsigned conversion of %s\n", szNum));
        return NULL;
    }

    PATOM pAtom = MemAlloc(sizeof(ATOM));
    if (!pAtom)
        return NULL;
    pAtom->Type = enmAtomNumber;
    pAtom->u.dValue = dValue;
    *ppszEnd = pszExpr;
    return pAtom;

#if 0
    const char *pszStart = pszExpr;

    /*
     * Binary prefix.
     */
    if (*pszExpr == 'b' || *pszExpr == 'B')
    {
        ++pszExpr;
        pszStart = pszExpr;
        while (*pszExpr == '0' || *pszExpr == '1')
            pszExpr++;

        if (pszExpr - pszStart > 0)
        {
            /* Sigh, constness, gotta love it. */
            char *pszEnd = NULL;
            FLOAT dValue = (FLOAT)strtoul(pszStart, &pszEnd, 2);
            *ppszEnd = pszEnd;
            PATOM pAtom = MemAlloc(sizeof(ATOM));
            if (!pAtom)
                return NULL;
            pAtom->Type = enmAtomNumber;
            pAtom->u.dValue = dValue;
            return pAtom;
        }
        pszExpr = pszStart - 1;
    }
    else if (*pszExpr == '0')
    {
        /*
         * Octal prefix.
         */
        ++pszExpr;
        pszStart = pszExpr;
        while (*pszExpr >= '0' && *pszExpr < '8')
            ++pszExpr;

        if (pszExpr - pszStart > 0)
        {
            PATOM pAtom = MemAlloc(sizeof(ATOM));
            if (!pAtom)
                return NULL;
            char *pszEnd = NULL;
            FLOAT dValue = (FLOAT)strtoul(pszStart, &pszEnd, 8);
            *ppszEnd = pszEnd;
            pAtom->Type = enmAtomNumber;
            pAtom->u.dValue = dValue;
            return pAtom;
        }
        pszExpr = pszStart - 1;
    }

    /*
     * Hexadecimal.
     */
    if (isdigit(*pszExpr))
    {
        ++pszExpr;
        if (*pszExpr == 'x' || *pszExpr == 'X')
        {
            pszStart = pszExpr;
            pszExpr++;
            while (   (*pszExpr >= '0' && *pszExpr <= 'F')
                   || (*pszExpr >= 'a' && *pszExpr <= 'f'))
            {
                ++pszExpr;
            }

            if (pszExpr - pszStart > 1)
            {
                PATOM pAtom = MemAlloc(sizeof(ATOM));
                if (!pAtom)
                    return NULL;
                char *pszEnd = NULL;
                ++pszStart;
                FLOAT dValue = (FLOAT)strtoull(pszStart, &pszEnd, 16);
                *ppszEnd = pszEnd;
                pAtom->Type = enmAtomNumber;
                pAtom->u.dValue = dValue;
                return pAtom;
            }
            pszExpr = pszStart - 1;
        }
        else
            --pszExpr;
    }

    /*
     * Decimal.
     */
    if (isdigit(*pszExpr) || *pszExpr == '.')
    {
        PATOM pAtom = MemAlloc(sizeof(ATOM));
        if (!pAtom)
            return NULL;
        char *pszEnd = NULL;
        /* I don't remember why I chose to use strtold instead of strtod, there was some issue on Darwin? Anyway.*/
        FLOAT dValue = (FLOAT)strtold(pszExpr, &pszEnd);
        *ppszEnd = pszEnd;
        pAtom->Type = enmAtomNumber;
        pAtom->u.dValue = dValue;
        DEBUGPRINTF(("decimal\n"));
        return pAtom;
    }
    return NULL;
#endif
}


/**
 * Parses an operator and returns an Operator Atom.
 *
 * @param   pszExpr                 The whitespace skipped expression to parse.
 * @param   ppszEnd                 Where to store till what point in pszExpr was scanned.
 * @param   pPreviousAtom           Pointer to the previously passed Atom in @a pszExpr if any, can be NULL.
 * @returns Pointer to an allocated Operator Atom or NULL if @a pszExpr was not an operator.
 */
static PATOM EvaluatorParseOperator(const char *pszExpr, const char **ppszEnd, PCATOM pPreviousAtom)
{
    for (unsigned i = 0; i < g_cOperators; i++)
    {
        size_t cbOperator = StrLen(g_aOperators[i].pszOperator);
        if (!StrNCmp(g_aOperators[i].pszOperator, pszExpr, cbOperator))
        {
            /*
             * Verify if there are enough parameters on the queue for left associative operators.
             * e.g for binary '-', the previous atom must exist and must not be an open parenthesis or any
             * other operator.
             */
            if (g_aOperators[i].Direction == enmDirLeft)
            {
                /* e.g: "-4" */
                if (!pPreviousAtom)
                    continue;

                /*
                 * pPreviousAtom should never  be close parantheis as it's deleted in EvaluatorParse(),
                 * but included in here for the logical condition.
                 */

                /* e.g: "(-4"  and "=-4" and ",-4" */
                if (   AtomIsOperator(pPreviousAtom)
                    && !AtomIsCloseParenthesis(pPreviousAtom))
                {
                    DEBUGPRINTF(("Here2 Id=%d\n", g_aOperators[i].OperatorId));
                    continue;
                }
            }

            PATOM pAtom = MemAlloc(sizeof(ATOM));
            if (!pAtom)
                return NULL;
            pAtom->Type = enmAtomOperator;
            pAtom->u.pOperator = &g_aOperators[i];
            pszExpr += cbOperator;
            *ppszEnd = pszExpr;
            return pAtom;
        }
    }
    return NULL;
}


/**
 * Parses a functor and returns a Functor Atom.
 *
 * @param   pszExpr                 The whitespace skipped expression to parse.
 * @param   ppszEnd                 Where to store till what point in pszExpr was scanned.
 * @param   pPreviousAtom           Pointer to the previously passed Atom in @a pszExpr if any, can be NULL.
 * @returns Pointer to an allocated Functor Atom or NULL if @ pszExpr was not a functor.
 */
static PATOM EvaluatorParseFunctor(const char *pszExpr, const char **ppszEnd, PCATOM pPreviousAtom)
{
    for (unsigned i = 0; i < g_cFunctors; i++)
    {
        size_t cbFunctor = StrLen(g_aFunctors[i].pszFunctor);
        if (!StrNCmp(g_aFunctors[i].pszFunctor, pszExpr, cbFunctor))
        {
            /*
             * Skip over whitespaces till we encounter an open parenthesis.
             */
            pszExpr += cbFunctor;
            while (isspace(*pszExpr))
                pszExpr++;

            if (!StrNCmp(pszExpr, g_pOperatorOpenParenthesis->pszOperator,
                            StrLen(g_pOperatorOpenParenthesis->pszOperator)))
            {
                PATOM pAtom = MemAlloc(sizeof(ATOM));
                if (!pAtom)
                    return NULL;
                pAtom->Type = enmAtomFunctor;
                pAtom->u.pFunctor = &g_aFunctors[i];
                pAtom->cFunctorParams = 0;
                *ppszEnd = pszExpr;
                return pAtom;
            }
        }
    }
    return NULL;
}


/**
 * Parses a variable and returns a Variable Atom.
 *
 * @param   pEval                   Pointer to the Evaluator object.
 * @param   pszExpr                 The whitespace skipped expression to parse.
 * @param   ppszEnd                 Where to store till what point in pszExpr was scanned.
 * @param   pPreviousAtom           Pointer to the previously passed Atom in @pszExpr if any, can be NULL.
 * @param   prc                     Where to store the status code while identifying the variable.
 * @returns Pointer to an allocated Variable Atom or NULL if @a pszExpr was not a variable.
 */
static PATOM EvaluatorParseVariable(PEVALUATOR pEval, const char *pszExpr, const char **ppszEnd, PCATOM pPreviousAtom, int *prc)
{
    /*
     * A variable is a stream of contiguous alpha numerics, i.e. only [_][a-z][0-9], nothing else.
     */
    DEBUGPRINTF(("Parse variable\n"));
    char szBuf[MAX_VARIABLE_NAME_LENGTH];
    unsigned i = 0;
    bool fValid = true;
    for (i = 0; *pszExpr != '\0' && i < MAX_VARIABLE_NAME_LENGTH; i++)
    {
        if (i == 0 && (!isalpha(*pszExpr) && *pszExpr != '_'))
        {
            fValid = false;
            break;
        }

        if (isalnum(*pszExpr) || *pszExpr == '_')
            szBuf[i] = *pszExpr;
        else
        {
            if (i == 0)
                fValid = false;
            break;
        }

        pszExpr++;
    }
    szBuf[i] = '\0';
    *ppszEnd = pszExpr;

    if (!fValid)
    {
        *prc = RERR_VARIABLE_NAME_INVALID;
        return NULL;
    }

    PATOM pAtom = MemAlloc(sizeof(ATOM));
    if (!pAtom)
    {
        *prc = RERR_NO_MEMORY;
        return NULL;
    }
    pAtom->Type = enmAtomVariable;

    /*
     * Associate the Atom with a predefined Variable, if not just record the name.
     * When we assign the Variable, we will create the actual Variable entry.
     */
    pAtom->u.pVariable = EvaluatorFindVariable(szBuf, &g_VarList);
    StrCopy(pAtom->szVariable, sizeof(pAtom->szVariable), szBuf);

    /*
     * Determine error code. For variables too long we return a successful (name truncated) Variable Atom
     * and the caller deals with exiting, cleaning-up etc. If we just return a NULL Atom, the caller cannot
     * identify the exact variable name.
     */
    if (i >= MAX_VARIABLE_NAME_LENGTH)
        *prc = RERR_VARIABLE_NAME_TOO_LONG;
    else
        *prc = RINF_SUCCESS;
    return pAtom;
}


/**
 * Parses a command and returns a Command Atom.
 *
 * @param   pEval                   Pointer to the Evaluator object.
 * @param   pszExpr                 The whitespace skipped expression to parse.
 * @param   ppszEnd                 Where to store till what point in pszExpr was scanned.
 * @param   pPreviousAtom           Pointer to the previously passed Atom in @pszExpr if any, can be NULL.
 * @param   prc                     Where to store the status code while identifying the variable.
 * @returns Pointer to an allocated Command Atom or NULL if @a
 *        pszExpr was not a command.
 */
static PATOM EvaluatorParseCommand(PEVALUATOR pEval, const char *pszExpr, const char **ppszEnd, PCATOM pPreviousAtom, int *prc)
{
    /*
     * A command must be the first atom in the expression.
     */
    if (pPreviousAtom)
        return NULL;

    /*
     * A command is a stream of contiguous alpha numerics, i.e. only [_][a-z][0-9], nothing else.
     */
    for (unsigned i = 0; i < g_cCommands; i++)
    {
        DEBUGPRINTF(("Parse Command %s\n", g_aCommands[i].pszCommand));
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

                PATOM pAtom = MemAllocZ(sizeof(ATOM));
                if (!pAtom)
                    return NULL;
                pAtom->Type = enmAtomCommand;
                pAtom->u.pCommand = &g_aCommands[i];
                *ppszEnd = pszExpr;
                return pAtom;
            }
        }
    }
    return NULL;
}


/**
 * Parses an Atom.
 *
 * @param   pEval                   Pointer to the Evaluator object.
 * @param   pszExpr                 The expression to parse.
 * @param   ppszEnd                 Where to store till what point in pszExpr was scanned.
 * @param   pPreviousAtom           Pointer to the previously passed Atom in @a pszExpr if any, can be NULL.
 * @param   prc                     Where to store the status code during parsing.
 * @returns Pointer to an allocated Atom or NULL if @a pszExpr has run out of identifying atoms.
 */
static PATOM EvaluatorParseAtom(PEVALUATOR pEval, const char *pszExpr, const char **ppszEnd, PCATOM pPreviousAtom, int *prc)
{
    DEBUGPRINTF(("GetAtom %s\n", pszExpr));
    PATOM pAtom = NULL;
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
        pAtom = EvaluatorParseFunctor(pszExpr, ppszEnd, pPreviousAtom);
        if (pAtom)
            break;

        /*
         * Parse command.
         */
        pAtom = EvaluatorParseCommand(pEval, pszExpr, ppszEnd, pPreviousAtom, prc);
        if (pAtom)
            break;

        /*
         * Parse number.
         */
        pAtom = EvaluatorParseNumber(pszExpr, ppszEnd);
        if (pAtom)
            break;

        /*
         * Parse operator.
         */
        pAtom = EvaluatorParseOperator(pszExpr, ppszEnd, pPreviousAtom);
        if (pAtom)
            break;

        /*
         * Parse variable.
         */
        pAtom = EvaluatorParseVariable(pEval, pszExpr, ppszEnd, pPreviousAtom, prc);
        if (pAtom)
            break;

        /** @todo hmm, think about this!? */
        *ppszEnd = pszExpr;
        break;
    }
    return pAtom;
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


static int FunctorSortCompare(const void *pvFunctor1, const void *pvFunctor2)
{
    PCFUNCTOR pFunctor1 = (PCFUNCTOR)pvFunctor1;
    PCFUNCTOR pFunctor2 = (PCFUNCTOR)pvFunctor2;
    const char *pszFunctor1 = pFunctor1->pszFunctor;
    const char *pszFunctor2 = pFunctor2->pszFunctor;
    return -StrNCmp(pszFunctor1, pszFunctor2, R_MAX(StrLen(pszFunctor1), StrLen(pszFunctor2)));
}


static int AscendingFunctorSortCompare(const void *pvFunctor1, const void *pvFunctor2)
{
    PCFUNCTOR pFunctor1 = (PCFUNCTOR)pvFunctor1;
    PCFUNCTOR pFunctor2 = (PCFUNCTOR)pvFunctor2;
    const char *pszFunctor1 = pFunctor1->pszFunctor;
    const char *pszFunctor2 = pFunctor2->pszFunctor;

    /*
     * For commands (i.e. no syntax description) we always sort them last.
     * For functors (i.e. with syntax description) we sort alphabetically.
     */
    bool fIsCommand1 = !StrCmp(pFunctor1->pszSyntax, "");
    bool fIsCommand2 = !StrCmp(pFunctor2->pszSyntax, "");
    if (fIsCommand1 == fIsCommand2)
        return StrCmp(pszFunctor1, pszFunctor2);

    if (fIsCommand1)
        return 1;
    else
        return -1;
}


/**
 * Internal, essential Evaluator initialization. The other one does a lot of
 * extra work like global initializations. This is the real deal.
 *
 * @param   pEval                   Pointer to the Evaluator object, cannot be NULL.
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
 * @param   pEval                   Pointer to an Evaluator object, cannot be NULL.
 * @param   pStack                  Pointer to any Stack to empty, can be NULL.
 */
static void EvaluatorCleanUp(PEVALUATOR pEval, PSTACK pStack)
{
    Assert(pEval);

    PATOM pAtom = NULL;
    PQUEUE pQueue = (PQUEUE)pEval->pvRPNQueue;
    if (pQueue)
    {
        while ((pAtom = QueueRemove(pQueue)) != NULL)
            MemFree(pAtom);
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
        while ((pAtom = StackPop(pStack)) != NULL)
            MemFree(pAtom);
    }
}


/**
 * Parses the expression into a modified reverse polish notation form. The logic
 * is mostly based on the shunting yard algorithm with modifications for extra
 * elements. This constitutes the first pass in evaluating an expression.
 * The @a pEval object is internally updated with the intermediate representation
 * which will be used in the next pass, which is evaluation.
 *
 * @param   pEval               Pointer to the Evaluator object.
 * @returns Status code on result of the parsing pass.
 */
int EvaluatorParse(PEVALUATOR pEval, const char *pszExpr)
{
    Assert(pEval);
    AssertReturn(pEval->u32Magic == RMAG_EVALUATOR, RERR_BAD_MAGIC);

    const char *pszEnd   = NULL;
    PCATOM pPreviousAtom = NULL;
    PATOM pAtom          = NULL;

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
     * Parse atoms onto the stack or queue.
     */
    int rc = RERR_UNDEFINED;
    while ((pAtom = EvaluatorParseAtom(pEval, pszExpr, &pszEnd, pPreviousAtom, &rc)) != NULL)
    {
        if (AtomIsCloseParenthesis(pPreviousAtom))
        {
            MemFree((void *)pPreviousAtom);
            pPreviousAtom = NULL;
        }

        if (pAtom->Type == enmAtomNumber)
        {
            DEBUGPRINTF(("Adding number %" FMTFLOAT " to queue\n", pAtom->u.dValue));
            QueueAdd(pQueue, pAtom);
        }
        else if (pAtom->Type == enmAtomOperator)
        {
            PCOPERATOR pOperator = pAtom->u.pOperator;
            if (OperatorIsOpenParenthesis(pOperator))
            {
                if (   pPreviousAtom
                    && !AtomIsFunctor(pPreviousAtom))
                {
                    DEBUGPRINTF(("Paranthesis begins when previous atom is not a functor!\n"));
                    MemFree(pAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_EXPRESSION_INVALID;
                }

                /*
                 * Open parenthesis.
                 */
                DEBUGPRINTF(("Parenthesis begin '%s' pushing to stack\n", pAtom->u.pOperator->pszOperator));
                StackPush(&Stack, pAtom);
            }
            else if (OperatorIsCloseParenthesis(pOperator))
            {
                /*
                 * Close paranthesis.
                 */
                DEBUGPRINTF(("Parenthesis end '%s'\n", pAtom->u.pOperator->pszOperator));
                PATOM pStackAtom = NULL;
                while ((pStackAtom = StackPeek(&Stack)) != NULL
                        && !AtomIsOpenParenthesis(pStackAtom))
                {
                    DEBUGPRINTF(("Popping '%s' to queue\n", pStackAtom->u.pOperator->pszOperator));
                    StackPop(&Stack);
                    QueueAdd(pQueue, pStackAtom);
                }

                if (pStackAtom == NULL)
                {
                    /*
                     * No matching open parenthesis for close parenthesis found.
                     */
                     DEBUGPRINTF(("Missing open paranthesis\n"));
                     MemFree(pAtom);
                     EvaluatorCleanUp(pEval, &Stack);
                     return RERR_PARENTHESIS_UNBALANCED;
                }

                /*
                 * This means "pStackAtom" is a left parenthesis, double check, then zap it.
                 */
                if (AtomIsOpenParenthesis(pStackAtom))
                {
                    StackPop(&Stack);
                    MemFree(pStackAtom);
                    pStackAtom = NULL;
                }

                /*
                 * If the left parenthesis is preceeded by a functor, pop it to the Queue incrementing number
                 * of parameters the functor already has.
                 */
                pStackAtom = StackPeek(&Stack);
                if (   pStackAtom
                    && AtomIsFunctor(pStackAtom))
                {
                    DEBUGPRINTF(("Popping functor '%s' to queue\n", pStackAtom->u.pFunctor->pszFunctor));
                    StackPop(&Stack);
                    QueueAdd(pQueue, pStackAtom);

                    ++pStackAtom->cFunctorParams;
                    if (pStackAtom->cFunctorParams > MAX_FUNCTOR_PARAMETERS)
                    {
                        DEBUGPRINTF(("Error! too many parameters to functor '%s'\n", pStackAtom->u.pFunctor->pszFunctor));

                        /*
                         * Too many parameters to functor. Get 0wt.
                         */
                        MemFree(pAtom);
                        EvaluatorCleanUp(pEval, &Stack);
                        return RERR_TOO_MANY_PARAMETERS;
                    }

                    if (pStackAtom->cFunctorParams < pStackAtom->u.pFunctor->cMinParams)
                    {
                        DEBUGPRINTF(("Error! too few parameters to functor '%s'\n", pStackAtom->u.pFunctor->pszFunctor));

                        /*
                         * Too few parameters to functor. 0wttie.
                         */
                        MemFree(pAtom);
                        EvaluatorCleanUp(pEval, &Stack);
                        return RERR_TOO_FEW_PARAMETERS;
                    }

                    DEBUGPRINTF(("Functor '%s' cParams=%u\n", pStackAtom->u.pFunctor->pszFunctor, (unsigned)pStackAtom->cFunctorParams));
                }
            }
            else if (OperatorIsParamSeparator(pOperator))
            {
                /*
                 * Function parameter separator.
                 */
                DEBUGPRINTF(("Parameter separator '%s'\n", pAtom->u.pOperator->pszOperator));
                PATOM pStackAtom = NULL;
                while ((pStackAtom = StackPeek(&Stack)) != NULL)
                {
                    if (AtomIsOpenParenthesis(pStackAtom))
                        break;
                    StackPop(&Stack);
                    QueueAdd(pQueue, pStackAtom);
                }
                if (!AtomIsOpenParenthesis(pStackAtom))
                {
                    DEBUGPRINTF(("Operator '%s' parameter mismatch\n", pOperator->pszOperator));
                    MemFree(pAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_PARANTHESIS_SEPARATOR_UNEXPECTED;
                }

                /*
                 * Check if the paranthesis is part of a functor, if so increment the parameter count
                 * in the functor structure.
                 */
                PATOM pOpenParenthesisAtom = StackPop(&Stack);
                PATOM pFunctorAtom = StackPop(&Stack);
                if (!AtomIsFunctor(pFunctorAtom))
                {
                    DEBUGPRINTF(("No function specified\n"));
                    MemFree(pAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_PARANTHESIS_SEPARATOR_UNEXPECTED;
                }

                ++pFunctorAtom->cFunctorParams;
                if (pFunctorAtom->cFunctorParams >= pFunctorAtom->u.pFunctor->cMaxParams)
                {
                    DEBUGPRINTF(("Error too many parameters to functor '%s' maximum=%d\n", pFunctorAtom->u.pFunctor->pszFunctor,
                                pFunctorAtom->u.pFunctor->cMaxParams));

                    /*
                     * Too many parameters to functor. Exit, stage 0wt.
                     */
                    MemFree(pAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_TOO_MANY_PARAMETERS;
                }

                /*
                 * Now that we've recorded the info into the functor Atom, restore the stack items as though
                 * nothing happened :)
                 */
                StackPush(&Stack, pFunctorAtom);
                StackPush(&Stack, pOpenParenthesisAtom);
                DEBUGPRINTF(("Functor '%s' cParams=%u\n", pFunctorAtom->u.pFunctor->pszFunctor, (unsigned)pFunctorAtom->cFunctorParams));
            }
            else if (OperatorIsAssignment(pOperator))
            {
                /*
                 * Variable assignment operator. This is going to be fun.
                 */
                PATOM pVarAtom = QueuePeekTail(pQueue);
                if (   pVarAtom
                    && AtomIsVariable(pVarAtom))
                {
                    EVALUATOR SubExprEval;
                    EvaluatorInitInternal(&SubExprEval);
                    const char *pszRightExpr = pszEnd;
                    DEBUGPRINTF(("Parsing subexpression '%s'\n", pszRightExpr));
                    int rc2 = EvaluatorParse(&SubExprEval, pszRightExpr);
                    if (RC_SUCCESS(rc2))
                    {
                        DEBUGPRINTF(("-- Done subexpression assignment '%s'\n", pszRightExpr));

                        if (!pVarAtom->u.pVariable)
                        {
                            DEBUGPRINTF(("Creating global variable entry for '%s'\n", pVarAtom->szVariable));

                            /*
                             * Create a variable entry for the Variable Atom.
                             */
                            PVARIABLE pVariable = MemAlloc(sizeof(VARIABLE));
                            if (!pVariable)
                            {
                                MemFree(pAtom);
                                EvaluatorCleanUp(pEval, &Stack);
                                EvaluatorDestroy(&SubExprEval);
                                return RERR_NO_MEMORY;
                            }
                            StrCopy(pVariable->szVariable, sizeof(pVariable->szVariable), pVarAtom->szVariable);
                            pVariable->pszExpr = StrDup(pszRightExpr);  /* @todo check for failure */
                            pVariable->pvRPNQueue = SubExprEval.pvRPNQueue;
                            pVariable->fCanReinit = true;
                            SubExprEval.pvRPNQueue = NULL;  /* tricky shit, i know... */

                            /*
                             * Add variable entry to the 'global' list & connect Atom to the variable.
                             * Heh, good thing we are not multi-threaded.
                             */
                            ListAdd(&g_VarList, pVariable);
                            MemFree(pAtom);
                        }
                        else if (pVarAtom->u.pVariable->fCanReinit)
                        {
                            /*
                             * Reassigning existing variable.
                             */
                            if (pVarAtom->u.pVariable->pszExpr)
                                StrFree(pVarAtom->u.pVariable->pszExpr);
                            pVarAtom->u.pVariable->pszExpr = StrDup(pszRightExpr);
                            if (!pVarAtom->u.pVariable->pszExpr)
                            {
                                MemFree(pAtom);
                                EvaluatorCleanUp(pEval, &Stack);
                                EvaluatorDestroy(&SubExprEval);
                                return RERR_NO_MEMORY;
                            }

                            pVarAtom->u.pVariable->pvRPNQueue = SubExprEval.pvRPNQueue;
                            SubExprEval.pvRPNQueue = NULL;  /* tricky shit, i know... */
                        }
                        else
                        {
                            StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), pVarAtom->szVariable);
                            MemFree(pAtom);
                            EvaluatorCleanUp(pEval, &Stack);
                            EvaluatorDestroy(&SubExprEval);
                            return RERR_VARIABLE_CANNOT_REASSIGN;
                        }

                        StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), pVarAtom->szVariable);
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
                        MemFree(pAtom);
                        EvaluatorCleanUp(pEval, &Stack);
                        EvaluatorDestroy(&SubExprEval);
                        return RERR_EXPRESSION_INVALID;
                    }
                }
                else
                {
                    MemFree(pAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_INVALID_ASSIGNMENT;
                }
            }
            else
            {
                /*
                 * Regular operator, handle precedence.
                 */
                PATOM pStackAtom = NULL;
                while ((pStackAtom = StackPeek(&Stack)) != NULL)
                {
                    if (  !AtomIsOperator(pStackAtom)
                        || AtomIsParenthesis(pStackAtom))
                        break;

                    PCOPERATOR pStackOperator = pStackAtom->u.pOperator;
                    if (   (pOperator->Direction == enmDirLeft && pOperator->Priority <= pStackOperator->Priority)
                        || (pOperator->Direction == enmDirRight && pOperator->Priority < pStackOperator->Priority))
                    {
                        DEBUGPRINTF(("Moving operator '%s' cParams=%d from stack to queue\n", pStackAtom->u.pOperator->pszOperator,
                                    pStackAtom->u.pOperator->cParams));
                        StackPop(&Stack);
                        QueueAdd(pQueue, pStackAtom);
                    }
                    else
                        break;
                }

                DEBUGPRINTF(("Pushing operator '%s' (id=%d) cParams=%d to stack\n", pAtom->u.pOperator->pszOperator,
                        pAtom->u.pOperator->OperatorId, pAtom->u.pOperator->cParams));
                StackPush(&Stack, pAtom);
            }
        }
        else if (pAtom->Type == enmAtomFunctor)
        {
            DEBUGPRINTF(("Pushing functor '%s' to stack\n", pAtom->u.pFunctor->pszFunctor));
            StackPush(&Stack, pAtom);
        }
        else if (pAtom->Type == enmAtomVariable)
        {
            if (rc == RERR_VARIABLE_NAME_TOO_LONG)
            {
                DEBUGPRINTF(("Variable name '%s' too long\n", pAtom->u.pVariable->szVariable));
                MemFree(pAtom);
                EvaluatorCleanUp(pEval, &Stack);
                return rc;
            }
            else if (rc == RERR_VARIABLE_NAME_INVALID)
            {
                DEBUGPRINTF(("Variable name '%s' invalid\n", pAtom->u.pVariable->szVariable));
                MemFree(pAtom);
                EvaluatorCleanUp(pEval, &Stack);
                return rc;
            }

            DEBUGPRINTF(("Adding variable '%s' to queue\n", pAtom->szVariable));
            QueueAdd(pQueue, pAtom);
        }
        else if (pAtom->Type == enmAtomCommand)
        {
            DEBUGPRINTF(("Adding command '%s' to queue\n", pAtom->u.pCommand->pszCommand));
            QueueAdd(pQueue, pAtom);

            /*
             * Evaluate the rest of the expression as an argument to the command atom if any.
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
                        MemFree(pAtom);
                        EvaluatorCleanUp(pEval, &Stack);
                        EvaluatorDestroy(&SubExprEval);
                        return RERR_CANT_ASSIGN_VARIABLE_FOR_COMMAND;
                    }

                    Assert(!pAtom->pszCommandExpr);
                    pAtom->pszCommandExpr = pszRightExpr;
                    DEBUGPRINTF(("pszCommandExpr=%s\n", pAtom->pszCommandExpr));
                    if (!pAtom->pszCommandExpr)
                    {
                        MemFree(pAtom);
                        EvaluatorCleanUp(pEval, &Stack);
                        EvaluatorDestroy(&SubExprEval);
                        return RERR_NO_MEMORY;
                    }

                    /*
                     * Evaluate it and add resulting atom to the RPN queue.
                     */
                    rc2 = EvaluatorEvaluate(&SubExprEval);
                    if (RC_SUCCESS(rc2))
                    {
                        if (SubExprEval.Result.fCommandEvaluated)
                        {
                            MemFree(pAtom);
                            EvaluatorCleanUp(pEval, &Stack);
                            EvaluatorDestroy(&SubExprEval);
                            return RERR_INVALID_COMMAND_PARAMETER;
                        }

                        PATOM pParamAtom = MemAlloc(sizeof(ATOM));
                        if (!pParamAtom)
                        {
                            MemFree(pAtom);
                            EvaluatorCleanUp(pEval, &Stack);
                            EvaluatorDestroy(&SubExprEval);
                            return RERR_NO_MEMORY;
                        }
                        pParamAtom->Type = enmAtomNumber;
                        pParamAtom->u.dValue = SubExprEval.Result.dValue;
                        pAtom->pvCommandParamAtom = pParamAtom;
                    }
                    else
                    {
                        MemFree(pAtom);
                        EvaluatorCleanUp(pEval, &Stack);
                        EvaluatorDestroy(&SubExprEval);
                        return RERR_INVALID_PARAMETER;
                    }
                }
                else
                {
                    MemFree(pAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    EvaluatorDestroy(&SubExprEval);
                    return RERR_EXPRESSION_INVALID;
                }
                EvaluatorDestroy(&SubExprEval);
            }
            else
            {
                Assert(!pAtom->pszCommandExpr);
                Assert(!pAtom->pvCommandParamAtom);
                DEBUGPRINTF(("Command: -- Parsing completed sucessfully no params.\n"));
            }

            /*
             * Stop parsing now that we've parsed a command and prepared any parameters atoms.
             */
            break;
        }
        else
        {
            DEBUGPRINTF(("unknown!\n"));
            MemFree(pAtom);
            pAtom = NULL;
            break;
        }
        pszExpr = pszEnd;
        pPreviousAtom = pAtom;
    }

    /*
     * If there any atoms we must pop them onto the queue, but
     * an open parenthesis on the stop of stack means we've got
     * unbalanced parenthesis, just get 0wt.
     */
    pAtom = StackPeek(&Stack);
    if (   pAtom
        && AtomIsOpenParenthesis(pAtom))
    {
        DEBUGPRINTF(("Unbalanced paranthesis\n"));
        StackPop(&Stack);
        MemFree(pAtom);
        return RERR_PARENTHESIS_UNBALANCED;
    }

    /*
     * Pop remainder operators/functions to the queue.
     */
    while ((pAtom = StackPop(&Stack)) != NULL)
    {
        DEBUGPRINTF(("Popping stack and adding to queue\n"));
        QueueAdd(pQueue, pAtom);
    }

    /*
     * Clear old queue if any, and store the new one.
     */
    PQUEUE pPrevQueue = (PQUEUE)pEval->pvRPNQueue;
    if (pPrevQueue)
    {
        while ((pAtom = QueueRemove(pPrevQueue)) != NULL)
            MemFree(pAtom);
        MemFree(pPrevQueue);
    }
    pEval->pvRPNQueue = pQueue;

    if (QueueSize(pQueue) == 0)
    {
        EvaluatorCleanUp(pEval, &Stack);
        DEBUGPRINTF(("Error, no atoms detected!\n"));
        return RERR_EXPRESSION_INVALID;
    }
    return RINF_SUCCESS;
}


/**
 * Evaluates an internal representation of a parsed expression. The logic is
 * reverse polish notation evaluation but modified to support variables, variable
 * parameters to functions and more.
 *
 * @param   pEval               Pointer to the Evaluator object.
 * @returns Status code on the result of the evaluation.
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
    PATOM pAtom = NULL;

    /*
     * Evaluate RPN from Queue to the Stack.
     */
    DEBUGPRINTF(("EvaluatorEvaluate: RPN: \n"));
    while ((pAtom = QueueRemove(pQueue)) != NULL)
    {
        if (pAtom->Type == enmAtomNumber)
        {
            DEBUGPRINTF(("Number: %" FMTFLOAT " ", pAtom->u.dValue));
            StackPush(&Stack, pAtom);
        }
        else if (pAtom->Type == enmAtomOperator)
        {
            PCOPERATOR pOperator = pAtom->u.pOperator;
            DEBUGPRINTF(("%s ", pOperator->pszOperator));
            if (StackSize(&Stack) < pOperator->cParams)
            {
                DEBUGPRINTF(("Error StackSize=%u Operator '%s' cParams=%d\n",
                                    (unsigned)StackSize(&Stack), pOperator->pszOperator, pOperator->cParams));

                /*
                 * Insufficient parameters to operator.
                 * Destroy remained of the queue, the stack and bail.
                 */
                MemFree(pAtom);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_TOO_FEW_PARAMETERS;
            }

            /*
             * Construct the array of PATOMS parameters and pass it to the Operator evaluator if any, otherwise
             * just push the first parameter as the result.
             */
            PATOM apAtoms[MAX_OPERATOR_PARAMETERS];
            Assert(pOperator->cParams <= MAX_OPERATOR_PARAMETERS);
            int rc = RINF_SUCCESS;
            PATOM pResultantAtom = NULL;
            for (int i = 0; i < pOperator->cParams; i++)
            {
                apAtoms[i] = StackPop(&Stack);

                /*
                 * Check if operator can cast to required type to perform it's operation.
                 * If not, we cannot proceed because it would invoke undefined behaviour.
                 */
                if (   pOperator->fUIntParams
                    && !CanCastAtom(apAtoms[i], (FLOAT)MIN_INTEGER, (FLOAT)MAX_UINTEGER))
                {
                    /*
                     * Bleh, operator cannot handle this big a number. Exit, stage whatever.
                     */
                    rc = RERR_UNDEFINED_BEHAVIOUR;
                    DEBUGPRINTF(("Operand to '%s' cannot be cast to integer without UB. rc=%d\n", pOperator->pszOperator, rc));
                    MemFree(pAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    return rc;
                }
            }

            if (RC_SUCCESS(rc))
            {
                if (pOperator->pfnOperator)
                {
                    EvaluatorInvertAtomArray(apAtoms, pOperator->cParams);
                    rc = pOperator->pfnOperator(pEval, apAtoms);
                    if (RC_SUCCESS(rc))
                        pResultantAtom = apAtoms[0];
                }
                else
                    pResultantAtom = apAtoms[0];
            }

            if (RC_SUCCESS(rc))
            {
                Assert(pResultantAtom);
                for (int i = 1; i < pOperator->cParams; i++)
                    MemFree(apAtoms[i]);
                StackPush(&Stack, pResultantAtom);
            }
            else
            {
                Assert(RC_FAILURE(rc));
                DEBUGPRINTF(("Operator '%s' on given operands failed. rc=%d\n", pOperator->pszOperator, rc));
                MemFree(pAtom);
                EvaluatorCleanUp(pEval, &Stack);
                return rc;
            }
        }
        else if (pAtom->Type == enmAtomFunctor)
        {
            PCFUNCTOR pFunctor = pAtom->u.pFunctor;
            DEBUGPRINTF(("%s ", pFunctor->pszFunctor));
            if (StackSize(&Stack) < pAtom->cFunctorParams)
            {
                DEBUGPRINTF(("Error StackSize=%u Functor '%s' cParams=%d cMinParams=%d cMaxParams=%d\n",
                             (unsigned)StackSize(&Stack), pFunctor->pszFunctor, pAtom->cFunctorParams,
                             pFunctor->cMinParams, pFunctor->cMaxParams));

                /*
                 * Insufficient parameters to functor. Buh bye.
                 */
                MemFree(pAtom);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_TOO_FEW_PARAMETERS;
            }

            /*
             * Construct an array of maximum possible PATOMS parameters and
             * pass it to the Functor evaluator if any, otherwise
             * just push the first parameter as the result.
             */
            Assert(pFunctor->cMaxParams <= MAX_FUNCTOR_PARAMETERS);
            uint32_t cParams = R_MIN(pAtom->cFunctorParams, MAX_FUNCTOR_PARAMETERS);
            PATOM *papAtoms = MemAlloc(cParams * sizeof(ATOM));
            if (!papAtoms)
            {
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_NO_MEMORY;
            }

            int rc = RERR_UNDEFINED;
            for (uint32_t i = 0; i < cParams; i++)
            {
                papAtoms[i] = StackPop(&Stack);

                /*
                 * Check if functor can cast to required type to perform it's operation.
                 * If not, we cannot proceed because it would invoke undefined behaviour.
                 */
                if (   pFunctor->fUIntParams
                    && !CanCastAtom(papAtoms[i], (FLOAT)MIN_INTEGER, (FLOAT)MAX_UINTEGER))
                {
                    /*
                     * Bleh, functor cannot handle this big a number. 0wT.
                     */
                    rc = RERR_UNDEFINED_BEHAVIOUR;
                    DEBUGPRINTF(("Parameter to '%s' cannot be cast to integer without UB. rc=%d\n", pFunctor->pszFunctor, rc));
                    MemFree(pAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    return rc;
                }
            }

            PATOM pResultantAtom = NULL;
            if (pFunctor->pfnFunctor)
            {
                EvaluatorInvertAtomArray(papAtoms, cParams);
                rc = pFunctor->pfnFunctor(pEval, papAtoms, cParams);
                if (RC_SUCCESS(rc))
                    pResultantAtom = papAtoms[0];
            }
            else
                pResultantAtom = papAtoms[0];

            if (pResultantAtom)
            {
                for (uint32_t k = 1; k < cParams; k++)
                    MemFree(papAtoms[k]);

                MemFree(papAtoms);
                StackPush(&Stack, pResultantAtom);
            }
            else
            {
                Assert(RC_FAILURE(rc));
                DEBUGPRINTF(("Functor '%s' on given operands failed! rc=%d\n", pFunctor->pszFunctor, rc));
                MemFree(pAtom);
                MemFree(papAtoms);
                EvaluatorCleanUp(pEval, &Stack);
                return rc;
            }
        }
        else if (pAtom->Type == enmAtomCommand)
        {
            PCOMMAND pCommand = pAtom->u.pCommand;
            Assert(pCommand);

            DEBUGPRINTF(("EvaluatorEvaluate: Command %s\n", pCommand->pszCommand));

            char *pszResult  = NULL;
            PATOM pParamAtom = NULL;
            int rc;
            if (pAtom->pvCommandParamAtom)
            {
                pParamAtom = pAtom->pvCommandParamAtom;
                if (!AtomIsNumber(pParamAtom))
                {
                    DEBUGPRINTF(("Not a number atom for command argument.\n"));
                    MemFree(pAtom);
                    MemFree(pParamAtom);
                    EvaluatorCleanUp(pEval, &Stack);
                    return RERR_INVALID_COMMAND_PARAMETER;
                }
            }

            rc = pCommand->pfnCommand(pEval, pParamAtom, &pszResult);
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

            MemFree(pAtom);
            if (pParamAtom)
                MemFree(pParamAtom);
            EvaluatorCleanUp(pEval, &Stack);
            break;
        }
        else if (pAtom->Type == enmAtomVariable)
        {
            PVARIABLE pVariable = pAtom->u.pVariable;
            if (!pVariable)
            {
                /*
                 * Try find the variable if the Variable atom was created BEFORE the creation of
                 * the variable entry. e.g. _a=_b+1, _b=5, _a and we are evaluating "_a" now whose
                 * RPN Atom "_b" has no association with the global variable entry "_b" yet.
                 */
                pAtom->u.pVariable = EvaluatorFindVariable(pAtom->szVariable, &g_VarList);
                if (!pAtom->u.pVariable)
                {
                    StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), pAtom->szVariable);
                    EvaluatorCleanVariables();
                    EvaluatorCleanUp(pEval, &Stack);
                    MemFree(pAtom);
                    return RERR_VARIABLE_UNDEFINED;
                }

                pVariable = pAtom->u.pVariable;
            }

            PQUEUE pVarQueue = pAtom->u.pVariable->pvRPNQueue;
            if (!pVarQueue)
            {
                /*
                 * Huh? User typed probably typed some crap and we formed variables out of it.
                 * Delete them and bail.
                 */
                EvaluatorCleanVariables();
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_EXPRESSION_INVALID;
            }

            /*
             * Avoid circular variable dependencies. We know which variable we are evaluating, add it to
             * a list of variables. As we recurse into sub-variables we check the list to make sure we are
             * not evaluating a variable being evaluated.
             */
            PVARIABLE pAncestorVar = EvaluatorFindVariable(pAtom->u.pVariable->szVariable, &pEval->VarList);
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
                    PATOM pSrcAtom = QueueItemAt(pVarQueue, i);
                    Assert(pSrcAtom);
                    PATOM pTmpAtom = AtomDup(pSrcAtom);
                    if (!pTmpAtom)
                    {
                        MemFree(pAtom);
                        EvaluatorCleanUp(pEval, &Stack);
                        return RERR_NO_MEMORY;
                    }
                    QueueAdd(pEvalQueue, pTmpAtom);
                }

                /*
                 * Construct a temporary Evaluator object and try evaluate the Variable.
                 */
                EVALUATOR VarEval;
                EvaluatorInitInternal(&VarEval);
                VarEval.pvRPNQueue = pEvalQueue;
                ListAppend(&VarEval.VarList, &pEval->VarList);
                ListAdd(&VarEval.VarList, pVariable);

                DEBUGPRINTF(("Evaluating variable '%s'\n", pAtom->u.pVariable->szVariable));
#ifdef _DEBUG
                EvaluatorPrintVarList(&VarEval.VarList);
#endif
                int rc = EvaluatorEvaluate(&VarEval);
                if (RC_SUCCESS(rc))
                {
                    /*
                     * Reuse Variable Atom as a Number Atom & push it to the Stack.
                     */
                    DEBUGPRINTF(("Variable '%s' is %" FMTFLOAT "\n", pAtom->u.pVariable->szVariable, VarEval.Result.dValue));
                    pAtom->Type = enmAtomNumber;
                    pAtom->u.dValue = VarEval.Result.dValue;

                    /*
                     * Remove the variable from the dependency list.
                     */
                    StackPush(&Stack, pAtom);
                    EvaluatorDestroy(&VarEval);
                }
                else
                {
                    /** Do -NOT- alter rc, it could be circular dependency error. */
                    DEBUGPRINTF(("Failed to evaluate right-hand expression for variable '%s'\n", pAtom->u.pVariable->szVariable));
                    StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), VarEval.Result.szVariable);
                    MemFree(pAtom);
                    EvaluatorDestroy(&VarEval);
                    EvaluatorCleanUp(pEval, &Stack);
                    return rc;
                }
            }
            else
            {
                DEBUGPRINTF(("Circular variable depedency on variable '%s'\n", pAncestorVar->szVariable));
                StrCopy(pEval->Result.szVariable, sizeof(pEval->Result.szVariable), pAncestorVar->szVariable);
                MemFree(pAtom);
                EvaluatorCleanUp(pEval, &Stack);
                return RERR_CIRCULAR_DEPENDENCY;
            }
        }
        else
        {
            DEBUGPRINTF(("UnknownAtom!\n"));
            MemFree(pAtom);
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
        pAtom = StackPop(&Stack);
        DEBUGPRINTF(("Result = %" FMTFLOAT "\n", pAtom->u.dValue));
        pEval->Result.ErrorIndex = -1;
        pEval->Result.dValue = pAtom->u.dValue;
        MemFree(pAtom);
        return RINF_SUCCESS;
    }
    else
        DEBUGPRINTF(("Here\n"));

    while ((pAtom = StackPop(&Stack)) != NULL)
    {
        DEBUGPRINTF(("PATOM free %" FMTFLOAT "\n", pAtom->u.dValue));
        MemFree(pAtom);
    }

    DEBUGPRINTF(("Too many atoms, invalid expression\n"));
    pEval->Result.ErrorIndex = -1;
    return RERR_EXPRESSION_INVALID;
}


/**
 * Destroys the Evaluator object.
 *
 * @param   pEval                   Pointer to an Evaluator object, cannot be NULL.
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
 * @param   pEval                   Pointer to an Evaluator object, cannot be NULL.
 * @param   pszError                Where to write a descriptive error if one should occur while initializing.
 * @param   cbError                 Size of the @pszError buffer including NULL terminator.
 * @returns Status code of initialization.
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
         * but we don't have a parameter separator for Operators unlike Functors.
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
     * Check for functor duplicates.
     */
    for (unsigned i = 0; i < g_cFunctors; i++)
    {
        PCFUNCTOR pFunctor = &g_aFunctors[i];
        for (unsigned k = 0; k < g_cFunctors; k++)
        {
            if (i == k)
                continue;

            PCFUNCTOR pCur = &g_aFunctors[k];
            if (!StrCmp(pCur->pszFunctor, pFunctor->pszFunctor))
            {
                StrNPrintf(pszError, cbError, "Functor '%s' is duplicated. at [%d] and [%d].", pFunctor->pszFunctor, i, k);
                return RERR_DUPLICATE_FUNCTOR;
            }
        }
    }

    /*
     * Sort functor list to workaround overlapping functor names, eg: "sqr" and "sqrt".
     */
    qsort(g_aFunctors, g_cFunctors, sizeof(FUNCTOR), FunctorSortCompare);
    DEBUGPRINTF(("Sorted Functors\n"));
    for (unsigned i = 0; i < g_cFunctors; i++)
    {
        if (   !g_aFunctors[i].pszFunctor
            || !g_aFunctors[i].pszSyntax
            || !g_aFunctors[i].pszDesc)
        {
            StrNPrintf(pszError, cbError, "Functor with missing name/syntax or description. index=%d.", i);
            return RERR_INVALID_FUNCTOR;
        }

        DEBUGPRINTF(("%s cMinParams=%d cMaxParams=%d\n", g_aFunctors[i].pszFunctor, g_aFunctors[i].cMinParams, g_aFunctors[i].cMaxParams));
    }

    /*
     * Create alphabetically sorted Functor list (for help listing)
     */
    if (g_paSortedFunctors)
        MemFree(g_paSortedFunctors);
    g_paSortedFunctors = MemAlloc(sizeof(FUNCTOR) * g_cFunctors);
    if (!g_paSortedFunctors)
        return RERR_NO_MEMORY;

    MemCpy(g_paSortedFunctors, &g_aFunctors, sizeof(FUNCTOR) * g_cFunctors);
    qsort(g_paSortedFunctors, g_cFunctors, sizeof(FUNCTOR), AscendingFunctorSortCompare);

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

int OpAdd(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = apAtoms[0]->u.dValue + apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpSubtract(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = apAtoms[0]->u.dValue - apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpNegate(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = -apAtoms[0]->u.dValue;
    return RINF_SUCCESS;
}

int OpMultiply(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = apAtoms[0]->u.dValue * apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpDivide(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = apAtoms[0]->u.dValue / apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpIncrement(PEVALUATOR pEval, PATOM apAtoms[])
{
    ++apAtoms[0]->u.dValue;
    return RINF_SUCCESS;
}

int OpDecrement(PEVALUATOR pEval, PATOM apAtoms[])
{
    --apAtoms[0]->u.dValue;
    return RINF_SUCCESS;
}

int OpShiftLeft(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (UINTEGER)apAtoms[0]->u.dValue << (UINTEGER)apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpShiftRight(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (UINTEGER)apAtoms[0]->u.dValue >> (UINTEGER)apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpBitNegate(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = ~(UINTEGER)apAtoms[0]->u.dValue;
    return RINF_SUCCESS;
}

int OpModulo(PEVALUATOR pEval, PATOM apAtoms[])
{
    if (NumberIsNegative(apAtoms[0]))
    {
        if (   !CanCastAtom(apAtoms[0], (FLOAT)MIN_INTEGER, (FLOAT)MAX_INTEGER)
            || !CanCastAtom(apAtoms[1], (FLOAT)MIN_INTEGER, (FLOAT)MAX_INTEGER))
        {
            return RERR_UNDEFINED_BEHAVIOUR;
        }
        apAtoms[0]->u.dValue = (INTEGER)apAtoms[0]->u.dValue % (INTEGER)apAtoms[1]->u.dValue;
    }
    else
        apAtoms[0]->u.dValue = (UINTEGER)apAtoms[0]->u.dValue % (UINTEGER)apAtoms[1]->u.dValue;

    return RINF_SUCCESS;
}

int OpLessThan(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (FLOAT)DefinitelyLessThan(apAtoms[0]->u.dValue, apAtoms[1]->u.dValue);
    return RINF_SUCCESS;
}

int OpGreaterThan(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (FLOAT)DefinitelyGreaterThan(apAtoms[0]->u.dValue, apAtoms[1]->u.dValue);
    return RINF_SUCCESS;
}

int OpEqualTo(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (FLOAT)EssentiallyEqual(apAtoms[0]->u.dValue, apAtoms[1]->u.dValue);
    return RINF_SUCCESS;
}

int OpLessThanOrEqualTo(PEVALUATOR pEval, PATOM apAtoms[])
{
    bool fLessThan = DefinitelyLessThan(apAtoms[0]->u.dValue, apAtoms[1]->u.dValue);
    bool fEqualTo = EssentiallyEqual(apAtoms[0]->u.dValue, apAtoms[1]->u.dValue);
    apAtoms[0]->u.dValue = (FLOAT)(fLessThan || fEqualTo);
    return RINF_SUCCESS;
}

int OpGreaterThanOrEqualTo(PEVALUATOR pEval, PATOM apAtoms[])
{
    bool fGreaterThan = DefinitelyGreaterThan(apAtoms[0]->u.dValue, apAtoms[1]->u.dValue);
    bool fEqualTo = EssentiallyEqual(apAtoms[0]->u.dValue, apAtoms[1]->u.dValue);
    apAtoms[0]->u.dValue = (FLOAT)(fGreaterThan || fEqualTo);
    return RINF_SUCCESS;
}

int OpNotEqualTo(PEVALUATOR pEval, PATOM apAtoms[])
{
    int rc = OpEqualTo(pEval, apAtoms);
    if (RC_SUCCESS(rc))
        apAtoms[0]->u.dValue = !((int)apAtoms[0]->u.dValue);
    return RINF_SUCCESS;
}

int OpLogicalNot(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = !(UINTEGER)apAtoms[0]->u.dValue;
    return RINF_SUCCESS;
}

int OpBitwiseAnd(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (UINTEGER)apAtoms[0]->u.dValue & (UINTEGER)apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpBitwiseXor(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (UINTEGER)apAtoms[0]->u.dValue ^ (UINTEGER)apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpBitwiseOr(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (UINTEGER)apAtoms[0]->u.dValue | (UINTEGER)apAtoms[1]->u.dValue;
    return RINF_SUCCESS;
}

int OpLogicalAnd(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (FLOAT)((UINTEGER)apAtoms[0]->u.dValue && (UINTEGER)apAtoms[1]->u.dValue ? true : false);
    return RINF_SUCCESS;
}

int OpLogicalOr(PEVALUATOR pEval, PATOM apAtoms[])
{
    apAtoms[0]->u.dValue = (FLOAT)((UINTEGER)apAtoms[0]->u.dValue || (UINTEGER)apAtoms[1]->u.dValue ? true : false);
    return RINF_SUCCESS;
}


/**
 * Searches the global list of functor for commands.
 * @todo later this must probably be part of a Evaluator object so that
 * it may find variables? Probably but that's after I implemented proper
 * variable support.
 *
 * @param   pszCommand      Partial command to search for.
 * @param   cchCommand      Number of characters in command to search. Usually this
 *                          is the string length since @a pszCommand itself is partial.
 * @param   iStart          Starting index to search from.
 * @param   piEnd           Where to store the last index from the search.
 * @returns Pointer to the full command
 */
const char *EvaluatorFindFunctor(const char *pszCommand, size_t cchCommand, unsigned iStart, unsigned *piEnd)
{
    for (unsigned i = iStart; i < g_cFunctors; i++)
    {
        PCFUNCTOR pFunctor = &g_aFunctors[i];
        if (!pFunctor)
            break;

        *piEnd = i + 1;
        if (!StrNCmp(pszCommand, pFunctor->pszFunctor, cchCommand))
            return pFunctor->pszFunctor;
    }

    return NULL;
}


/**
 * Returns the syntax and description of a functor.
 *
 * @param   uIndex          The index of the requested functor.
 * @param   ppszName        Where to store the name of the functor, caller frees with StrFree()
 * @param   ppszSyntax      Where to store the syntax for the functor, caller frees with StrFree()
 * @param   ppszHelp        Where to store the description for the functor, caller frees with StreFree()
 * @returns RINF_SUCCESS on success, othwerise appropriate status code.
 */
int EvaluatorFunctorHelp(unsigned uIndex, char **ppszName, char **ppszSyntax, char **ppszHelp)
{
    if (uIndex > g_cFunctors)
        return RERR_NO_DATA;

    *ppszName   = StrDup(g_paSortedFunctors[uIndex].pszFunctor);
    *ppszSyntax = StrDup(g_paSortedFunctors[uIndex].pszSyntax);
    *ppszHelp   = StrDup(g_paSortedFunctors[uIndex].pszDesc);

    return RINF_SUCCESS;
}

/**
 * Returns the total number of functors.
 *
 * @returns The total number of functors.
 */
unsigned EvaluatorFunctorCount(void)
{
    return g_cFunctors;
}


/**
 * Returns the total number of commands.
 *
 * @returns The total number of commands.
 */
unsigned EvaluatorCommandCount(void)
{
    return g_cCommands;
}


/**
 * Returns the syntax and description of an operator.
 *
 * @param   uIndex          The index of the requested operator.
 * @param   ppszName        Where to store the name of the operator, caller frees with StrFree()
 * @param   ppszSyntax      Where to store the syntax for the operator, caller frees with StrFree()
 * @param   ppszHelp        Where to store the description for the operator, caller frees with StreFree()
 * @returns RINF_SUCCESS on success, othwerise appropriate status code.
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
 * @param   uIndex          The index of the requested variable.
 * @param   ppszName        Where to store the name of the variable, caller frees with StrFree()
 * @param   ppszExpr        Where to store the expression assigned to the variable, caller frees with StrFree()
 * @returns RINF_SUCCESS on success, otherwise appropriate status code.
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
 * @returns The total number of operators.
 */
unsigned EvaluatorOperatorCount(void)
{
    return g_cOperators;
}


/**
 * Initializes the globals.
 *
 * @returns RINF_SUCCESS on success, otherwise an appropriate status code.
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

