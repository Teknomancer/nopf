/** @file
 * Evaluator Commands, header.
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

#ifndef EVALUATOR_COMMANDS_H___
#define EVALUATOR_COMMANDS_H___

#include "EvaluatorInternal.h"
#include "Assert.h"

/**
 * Register type bit.
 */
typedef enum REGBITTYPE
{
    enmRsvd = 0,    /**< Reserved */
    enmRsvdMB1,     /**< Reserved Must Be 1.*/
    enmRsvdMBZ,     /**< Reserved Must Be 0. */
    enmInRange,     /**< Part of a bit range, previously recognized (normal) bit. */
    enmNormal       /**< Normal bit. */
} REGBITTYPE;


/**
 * Register bit descriptor for.
 */
typedef struct REGBITDESC
{
    REGBITTYPE      enmType;            /**< Register bit type.  */
    char            szName[64];         /**< Name of this bit. */
    char            szLongName[128];    /**< Description name. */
    char            szDesc[256];        /**< Long description of this bit. */
} REGBITDESC;
/** Pointer to a register bit descriptor object. */
typedef REGBITDESC *PREGBITDESC;
/** Pointer to a const register bit descriptor object. */
typedef const REGBITDESC *PCREGBITDESC;


/**
 * A 32-bit register descriptor.
 */
typedef struct REGDESC32
{
    char            szName[64];         /**< Name of the register. */
    char            szDesc[256];        /**< Long description of the register. */
    char            szAddDesc[2048];    /**< Additional description (notes, tables etc.) */
    PREGBITDESC     paRegBitDesc;       /**< Pointer to bit descriptor array for 32 bits. */
} REGDESC32;
/** Pointer to a 32-bit register descriptor object. */
typedef REGDESC32 *PREGDESC32;
/** Pointer to a const 32-bit register descriptor object. */
typedef const REGDESC32 *PCREGDESC32;


extern COMMAND g_aCommands[];
extern const unsigned g_cCommands;

#endif /* EVALUATOR_COMMANDS_H___ */

