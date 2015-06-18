/* $Id: GenericDefs.h 201 2014-05-17 17:47:00Z marshan $ */
/** @file
 * Generic helper macros and routines header.
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

#ifndef GENERICS_H___
#define GENERICS_H___

/** @def R_ARRAY_ELEMENTS
 * Returns the number of elements in an array.
 *
 * @param   aArray      An array.
 * @returns The number of elements in an array object.
 */
#define R_ARRAY_ELEMENTS(aArray)                     ( sizeof(aArray) / sizeof((aArray)[0]) )

/** @def R_MAX
 * Returns the greater of the two objects.
 *
 * @param   a           First object or POD type.
 * @param   b           Second object or POD type.
 * @returns The greater of the two objects.
 */
#define R_MAX(a,b)                                   ((a) >= (b) ? (a) : (b))

/** @def R_MIN
 * Returns the lesser of the two objects.
 *
 * @param   a           First object or POD type.
 * @param   b           Second object or POD type.
 * @returns The lesser of the two objects.
 */
#define R_MIN(a,b)                                   ((a) <= (b) ? (a) : (b))

/** @def NOREF
 * A no reference stub to shut up compiler warnings about unused variables.
 *
 * @param   a           Any unused variable.
 */
#define NOREF(a)                                     (void)(a)

/** @def R_BIT
 * Convert a bit number into an integer bitmask (unsigned).
 * @param   a           The bit number.
 */
#define R_BIT(a)                                     (1U << (a))

#endif /* GENERICS_H___ */

