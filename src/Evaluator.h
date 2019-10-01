/** @file
 * Expression evaluator, hopefully should be fairly generic header.
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

#ifndef EVALUATOR_H___
#define EVALUATOR_H___

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

#include "Types.h"
#include "Queue.h"
#include "List.h"

#define MAX_VARIABLE_NAME_LENGTH    128
#define MAX_COMMAND_NAME_LENGTH     64
#define MAX_COMMAND_RESULT_LENGTH   4096

/**
 * EVALRESULT: Holds the result of a parsed/evaluated expression.
 */
typedef struct EVALRESULT
{
    bool        fVariableAssignment;                           /**< Whether this is a Variable assignment. */
    bool        fCommandEvaluated;                             /**< Whether this is an evaluated Command. */
    char        szVariable[MAX_VARIABLE_NAME_LENGTH];          /**< Name of assigned Variable if any. */
    char        szFunction[MAX_VARIABLE_NAME_LENGTH];          /**< Name of the Function if it's a single function call. */
    char        szCommand[MAX_COMMAND_NAME_LENGTH];            /**< Name of evaluated Command if any. */
    char        szCommandResult[MAX_COMMAND_RESULT_LENGTH];    /**< Output of the Command if it's a Command. */
    UINTEGER    uValue;                                        /**< Integer value of the parse/evaluation phase. */
    FLOAT       dValue;                                        /**< Float value of the parse/evaluation phase. */
    int         ErrorIndex;                                    /**< Index into the original expression if case of an error. */
} EVALRESULT;
/** Pointer to an evaluation result. */
typedef EVALRESULT *PEVALRESULT;
/** Pointer to a const evaluation result. */
typedef const EVALRESULT *PCCEVALRESULT;


/**
 * EVALUATOR: The main evaluator object.
 */
typedef struct EVALUATOR
{
    uint32_t        u32Magic;       /**< Magic (RMAG_EVALUATOR). */
    EVALRESULT      Result;         /**< The result of the last parse/evaluation pass. */
    const char     *pszExpr;        /**< The current expression. */
    void           *pvRPNQueue;     /**< Internal RPN representation (Queue) done by the parse phase. */
    LIST            VarList;        /**< List of Variables being evaluated, used for circular dependency prevention. */
} EVALUATOR;
/** Pointer to an evaluator. */
typedef EVALUATOR *PEVALUATOR;
/** Pointer to a const evaluator. */
typedef const EVALUATOR *PCEVALUATOR;


int         EvaluatorInitGlobals(void);
void        EvaluatorDestroyGlobals(void);

int         EvaluatorInit(PEVALUATOR pEval, char *pszError, size_t cbError);
void        EvaluatorDestroy(PEVALUATOR pEval);
int         EvaluatorParse(PEVALUATOR pEval, const char *pszExpr);
int         EvaluatorEvaluate(PEVALUATOR pEval);

const char *EvaluatorFindFunction(const char *pszCommand, uint32_t cchCommand, uint32_t iStart, uint32_t *piEnd);
unsigned    EvaluatorFunctionCount(void);
int         EvaluatorFunctionHelp(unsigned uIndex, char **ppszName, char **ppszSyntax, char **ppszHelp);
unsigned    EvaluatorOperatorCount(void);
int         EvaluatorOperatorHelp(unsigned uIndex, char **ppszName, char **ppszSyntax, char **ppszHelp);
int         EvaluatorVariableValue(unsigned uIndex, char **ppszName, char **ppszExpr);
unsigned    EvaluatorCommandCount(void);

#endif /* EVALUATOR_H___ */

