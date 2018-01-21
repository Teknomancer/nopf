/* $Id: EvaluatorInternal.h 192 2014-05-14 06:52:35Z marshan $ */
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

/** Maximum number of parameters that can be passed to an operator */
#define MAX_OPERATOR_PARAMETERS     2
#define MAX_FUNCTOR_PARAMETERS      65536
#define OPEN_PAREN_ID               INT_MAX - 1
#define CLOSE_PAREN_ID              INT_MAX - 2
#define PARAM_SEP_ID                INT_MAX - 3
#define VAR_ASSIGN_ID               INT_MAX - 4
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

/**
 * ATOMTYPE: The type of Atom.
 */
typedef enum ATOMTYPE
{
    enmAtomEmpty = 0x10,
    enmAtomNumber,
    enmAtomOperator,
    enmAtomFunctor,
    enmAtomVariable,
    enmAtomCommand
} ATOMTYPE;

/**
 * ATOM: An Atom.
 * An Atom represents the smallest unit of parsing.
 */
typedef struct ATOM
{
    /** The type. */
    ATOMTYPE       Type;
    /** Cursor position, an Index used to flag errors. */
    uint32_t       Position;
    /** Holds number of parameters to pass to the Functor. */
    uint32_t       cFunctorParams;
    /** Variable Name if this is a variable. */
    char           szVariable[MAX_VARIABLE_NAME_LENGTH];
    /** The expression of a command if this is a command. */
    const char    *pszCommandExpr;
    /** Pointer to the Number Atom argument for a command Atom. */
    void           *pvCommandParamAtom;
    /** The data union. */
    union
    {
        /** Value of the number for a Number Atom. */
        /* If this needs to change, don't forget VARBUCKET */
        FLOAT                    dValue;
        /** Pointer to the OPERATOR for an Operator Atom. */
        struct OPERATOR const   *pOperator;
        /** Pointer to the FUNCTOR for a Functor Atom. */
        struct FUNCTOR  const   *pFunctor;
        /** Pointer to the VARIABLE entry for a Variable Atom. */
        struct VARIABLE         *pVariable;
        /** Pointer to the COMMAND entry for the Command Atom. */
        struct COMMAND          *pCommand;
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
    enmDirNone = 0x30,
    enmDirLeft,
    enmDirRight
} OPERATORDIR;

/**
 * OPERATOR: An Operator.
 * An Operator performs an operation on one or more operands.
 */
typedef struct OPERATOR
{
    /** The operator Id, used to identify certain key Operators. */
    int             OperatorId;
    /** Operator priority, value is relative to Operators. */
    int             Priority;
    /** Operator associativity. */
    OPERATORDIR     Direction;
    /** Number of parameters to the operator, valid values: (0-2). */
    uint8_t         cParams;
    /** Whether the parameters must all fit into UINTEGER */
    bool            fUIntParams;
    /** Name of the Operator as seen in the expression. */
    const char     *pszOperator;
    /** Pointer to the Operator evaluator function. */
    PFNOPERATOR     pfnOperator;
    /** Short description of the Operator, NULL if already described. */
    const char     *pszSyntax;
    /** Long description of the Operator, NULL if already described. */
    const char     *pszDesc;
} OPERATOR;
/** Pointer to an Operator object. */
typedef OPERATOR *POPERATOR;
/** Pointer to a const Operator object. */
typedef const OPERATOR *PCOPERATOR;




/** A Functor function. */
typedef int FNFUNCTOR(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms);
/** Pointer to a Functor function. */
typedef FNFUNCTOR *PFNFUNCTOR;

/**
 * FUNCTOR: Sounds cooler than function.
 */
typedef struct FUNCTOR
{
    /** Name of the Functor as seen in the expression. */
    const char     *pszFunctor;
    /** Pointer to the Functor evaluator function. */
    PFNFUNCTOR      pfnFunctor;
    /** Whether the parameters must all fit into UINTEGER */
    bool            fUIntParams;
    /** Minimum parameters accepted by @a pfnFunctor. */
    uint32_t        cMinParams;
    /** Maximum paramaters accepted by @a pfnFunctor. */
    uint32_t        cMaxParams;
    /** Short description of the Functor, NULL if already described. */
    const char     *pszSyntax;
    /** Long description of the Functor, NULL if already described. */
    const char     *pszDesc;
} FUNCTOR;
/** Pointer to a Functor object. */
typedef FUNCTOR *PFUNCTOR;
/** Pointer to a const Functor object. */
typedef const FUNCTOR *PCFUNCTOR;


/**
 * VARIABLE: Variable table entry to match names to values.
 */
typedef struct VARIABLE
{
    /** Name of the variable as seen in the expression. */
    char                     szVariable[MAX_VARIABLE_NAME_LENGTH];
    /** The expression assigned to the variable. */
    char                    *pszExpr;
    /** Whether this variable can be re-assigned. */
    bool                     fCanReinit;
    /** Pointer to the RPN Queue. */
    void                    *pvRPNQueue;
} VARIABLE;
/** Pointer to a Varbucket object. */
typedef VARIABLE *PVARIABLE;
/** Pointer to a const Varbucket object. */
typedef const VARIABLE *PCVARIABLE;



/** A Command function. */
typedef int FNCOMMAND(PEVALUATOR pEval, PATOM pAtom, char **ppszResult);
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
    /** Name of the Command as seen in the expression. */
    const char     *pszCommand;
    /** Pointer to the Command evaluator function. */
    PFNCOMMAND      pfnCommand;
    /** Short description of the Command, NULL if already
     *  described. */
    const char     *pszSyntax;
    /** Long description of the Command, NULL if already
     *  described. */
    const char     *pszDesc;
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

