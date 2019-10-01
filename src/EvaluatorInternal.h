/** @file
 * Evaluator, hopefully should be fairly generic, internal header.
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

#ifndef EVALUATOR_INTERNAL_H___
#define EVALUATOR_INTERNAL_H___

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <inttypes.h>

#include "Types.h"
#include "Evaluator.h"

/** Maximum number of parameters that can be passed to an Operator */
#define MAX_OPERATOR_PARAMETERS     2
/** Maximum number of parameters that can be passed to a Function. */
#define MAX_FUNCTION_PARAMETERS     1024
/** Open paranthesis Operator Id.   */
#define OPEN_PAREN_ID               INT_MAX - 1
/** Close paranthesis Operator Id.   */
#define CLOSE_PAREN_ID              INT_MAX - 2
/** Parameter separator Operator Id.   */
#define PARAM_SEP_ID                INT_MAX - 3
/** Variable assignment Operator Id. */
#define VAR_ASSIGN_ID               INT_MAX - 4
/** Maximum length of a Variable name. */
#define MAX_VARIABLE_NAME_LENGTH    128

/** More handy constant definitions */
/** 1 Kilo                          (1024). */
#define _1K                         0x00000400L
/** 4 Kilo                          (4096). */
#define _4K                         0x00001000L
/** 1 Mega                          (1048576). */
#define _1M                         0x00100000L
/** 1 Giga                          (1073741824). */
#define _1G                         0x40000000L
/** 1 Tera                          (1099511627776). */
#define _1T                         0x0000010000000000LL
/** 1 Peta                          (1125899906842624). */
#define _1P                         0x0004000000000000LL
/** 1 Exa                           (1152921504606846976). */
#define _1E                         0x1000000000000000LL
/** Page size                       (4K pages). */
#define _MEM_PAGESIZE               _4K
/** Page offset. */
#define _MEM_PAGEOFFSET             (_MEM_PAGESIZE - 1)
/** Page shift                      log2(_MEM_PAGESIZE)*/
#define _MEM_PAGESHIFT              12

/** 1 Milli */
#define _1MILLI                     1000L
/** 1 Micro */
#define _1MICRO                     1000000L
/** 1 Nano */
#define _1NANO                      1000000000LL


/** NUMBER: A number. */
typedef struct NUMBER
{
    UINTEGER    uValue;     /**< Value represented as an unsigned integer. */
    FLOAT       dValue;     /**< Value represented as floating point. */
} NUMBER;
/** Pointer to an Number object. */
typedef NUMBER *PNUMBER;
/** Pointer to a const Number object. */
typedef const NUMBER *PCNUMBER;


/**
 * ATOMTYPE: The type of Atom.
 */
typedef enum ATOMTYPE
{
    enmAtomEmpty = 1,  /**< Empty/invalid. */
    enmAtomNumber,     /**< Number Atom. */
    enmAtomOperator,   /**< Operator Atom. */
    enmAtomFunction,   /**< Function Atom. */
    enmAtomVariable,   /**< Variable Atom. */
    enmAtomCommand     /**< Command Atom. */
} ATOMTYPE;

/**
 * ATOM: An Atom.
 * An Atom represents the smallest unit of parsing.
 */
typedef struct ATOM
{
    ATOMTYPE     Type;                                  /**< The type. */
    uint32_t     Position;                              /**< Cursor position, an Index used to flag errors. */
    uint32_t     cFunctionParams;                       /**< Number of parameters to pass to the Function atom. */
    char         szVariable[MAX_VARIABLE_NAME_LENGTH];  /**< Variable Name if this is a Variable atom. */
    const char  *pszCommandExpr;                        /**< The expression of a command if this is a command. */
    void        *pvCommandParamAtom;                    /**< The Number Atom argument for a Command Atom. */

    /** The data union. */
    union
    {
        struct NUMBER            Number;        /**< The NUMBER for a Number Atom. */
        struct OPERATOR const   *pOperator;     /**< Pointer to the OPERATOR for an Operator Atom. */
        struct FUNCTION const   *pFunction;     /**< Pointer to the FUNCTION for a Function Atom. */
        struct VARIABLE         *pVariable;     /**< Pointer to the VARIABLE entry for a Variable Atom. */
        struct COMMAND          *pCommand;      /**< Pointer to the COMMAND entry for the Command Atom. */
    } u;
} ATOM;
/** Pointer to an Atom object. */
typedef ATOM *PATOM;
/** Pointer to a const Atom object. */
typedef const ATOM *PCATOM;


/** An Operator function. */
typedef int FNOPERATOR(PEVALUATOR pEval, PATOM apAtoms[]);
/** Pointer to an Operator function. */
typedef FNOPERATOR *PFNOPERATOR;

/**
 * OPERATORDIR: Operator direction.
 * The associativity associated with the Operator.
 */
typedef enum OPERATORDIR
{
    enmDirNone = 1,     /**< None/invalid. */
    enmDirLeft,         /**< Operator is left associative. */
    enmDirRight         /**< Operator is right associative. */
} OPERATORDIR;

/**
 * OPERATOR: An Operator.
 * An Operator performs an operation on one or more operands.
 */
typedef struct OPERATOR
{
    int             OperatorId;     /**< The operator Id, used to identify certain key Operators. */
    int             Priority;       /**< Operator priority, value is relative to Operators. */
    OPERATORDIR     Direction;      /**< Operator associativity. */
    uint8_t         cParams;        /**< Number of parameters to the operator, valid values: (0-2). */
    bool            fUIntParams;    /**< Whether the parameters must all fit into UINTEGER */
    const char     *pszOperator;    /**< Name of the Operator as seen in the expression. */
    PFNOPERATOR     pfnOperator;    /**< Pointer to the Operator evaluator function. */
    const char     *pszSyntax;      /**< Short description of the Operator, NULL if already described. */
    const char     *pszDesc;        /**< Long description of the Operator, NULL if already described. */

} OPERATOR;
/** Pointer to an Operator object. */
typedef OPERATOR *POPERATOR;
/** Pointer to a const Operator object. */
typedef const OPERATOR *PCOPERATOR;


/** A function. */
typedef int FNFUNCTION(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms);
/** Pointer to a Function function. */
typedef FNFUNCTION *PFNFUNCTION;

/**
 * FUNCTION: A function.
 * A function takes one or more parameters and produces a result.
 */
typedef struct FUNCTION
{
    const char     *pszFunction;    /**< Name of the Function as seen in the expression. */
    PFNFUNCTION     pfnFunction;    /**< Pointer to the Function evaluator function. */
    bool            fUIntParams;    /**< Whether the parameters must all fit into UINTEGER */
    uint32_t        cMinParams;     /**< Minimum parameters accepted by @a pfnFunction. */
    uint32_t        cMaxParams;     /**< Maximum paramaters accepted by @a pfnFunction. */
    const char     *pszSyntax;      /**< Short description of the Function, NULL if already described. */
    const char     *pszDesc;        /**< Long description of the Function, NULL if already described. */
} FUNCTION;
/** Pointer to a Function object. */
typedef FUNCTION *PFUNCTION;
/** Pointer to a const Function object. */
typedef const FUNCTION *PCFUNCTION;


/**
 * VARIABLE: Variable table entry to match names to values.
 */
typedef struct VARIABLE
{
    char    szVariable[MAX_VARIABLE_NAME_LENGTH]; /**< Name of the variable as seen in the expression. */
    char   *pszExpr;                              /**< The expression assigned to the variable. */
    bool    fCanReinit;                           /**< Whether this variable can be re-assigned. */
    void   *pvRPNQueue;                           /**< Pointer to the RPN Queue. */
} VARIABLE;
/** Pointer to a Varbucket object. */
typedef VARIABLE *PVARIABLE;
/** Pointer to a const Varbucket object. */
typedef const VARIABLE *PCVARIABLE;


/** A Command function. */
typedef int FNCOMMAND(PEVALUATOR pEval, PATOM pAtom, const char **ppszResult);
/** Pointer to a Command function. */
typedef FNCOMMAND *PFNCOMMAND;

/**
 * COMMAND: An evaluator command.
 * A command performs an operation on zero or one experssion and
 * produces a single non-evaluatable result (like a formatted
 * string etc.)
 */
typedef struct COMMAND
{
    const char     *pszCommand;    /**< Name of the Command as seen in the expression. */
    PFNCOMMAND      pfnCommand;    /**< Pointer to the Command evaluator function. */
    const char     *pszSyntax;     /**< Short description of the Command, NULL if already described. */
    const char     *pszDesc;       /** Long description of the Command, NULL if already described. */
} COMMAND;
/** Pointer to a Command object. */
typedef COMMAND *PCOMMAND;
/** Pointer to a const Command object. */
typedef const COMMAND *PCCOMMAND;


static inline bool AtomIsNumber(PCATOM pAtom)
{
    return (pAtom && pAtom->Type == enmAtomNumber);
}

#endif /* EVALUATOR_INTERNAL_H___ */

