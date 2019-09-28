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
    /** Is this a variable assignment? */
    bool            fVariableAssignment;
    /** Is this an evaluated command? */
    bool            fCommandEvaluated;
    /** Name of assigned variable if any. */
    char            szVariable[MAX_VARIABLE_NAME_LENGTH];
    /** Name of the function if it's a single function call (truncated to 128 characters for now) */
    char            szFunction[MAX_VARIABLE_NAME_LENGTH];
    /** Name of evaluated command if any. */
    char            szCommand[MAX_COMMAND_NAME_LENGTH];
    /** Output of the command if it's a command. */
    char            szCommandResult[MAX_COMMAND_RESULT_LENGTH];
    /** Value of the parse/evaluation phase. */
    FLOAT           dValue;
    /** Index into the original expression if an error occurred. */
    int             ErrorIndex;
} EVALRESULT;
typedef EVALRESULT *PEVALRESULT;
typedef const EVALRESULT *PCCEVALRESULT;


/**
 * EVALUATOR: The main evaluator object.
 */
typedef struct EVALUATOR
{
    /** Magic (RMAG_EVALUATOR). */
    uint32_t        u32Magic;
    /** The result of the last parse/evaluation pass. */
    EVALRESULT      Result;
    /** The current expression. */
    const char     *pszExpr;
    /** Internal RPN representation (Queue) done by the parse phase. */
    void           *pvRPNQueue;
    /** List of variables being evaluated, used for circular dependency prevention. */
    LIST            VarList;
} EVALUATOR;
typedef EVALUATOR *PEVALUATOR;
typedef const EVALUATOR *PCEVALUATOR;


int EvaluatorInitGlobals(void);
void EvaluatorDestroyGlobals(void);

int EvaluatorInit(PEVALUATOR pEval, char *pszError, size_t cbError);
void EvaluatorDestroy(PEVALUATOR pEval);
int EvaluatorParse(PEVALUATOR pEval, const char *pszExpr);
int EvaluatorEvaluate(PEVALUATOR pEval);


const char *EvaluatorFindFunctor(const char *pszCommand, uint32_t cchCommand, uint32_t iStart, uint32_t *piEnd);
unsigned EvaluatorFunctorCount(void);
int EvaluatorFunctorHelp(unsigned uIndex, char **ppszName, char **ppszSyntax, char **ppszHelp);
unsigned EvaluatorOperatorCount(void);
int EvaluatorOperatorHelp(unsigned uIndex, char **ppszName, char **ppszSyntax, char **ppszHelp);
int EvaluatorVariableValue(unsigned uIndex, char **ppszName, char **ppszExpr);
unsigned EvaluatorCommandCount(void);

#endif /* EVALUATOR_H___ */

