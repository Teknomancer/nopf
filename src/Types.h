/** @file
 * Internal types, header.
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

#ifndef TYPES_H___
#define TYPES_H___

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>

/** Printf format specifiers for the various integers. */
#define FMT_FLT_NAT    "Lg"

#define FMT_U32_NAT    "PRIu32"
#define FMT_S32_NAT    "PRId32"

#define FMT_U64_NAT    "PRIu64"
#define FMT_S64_NAT    "PRId64"

#define FMT_U32_HEX    "PRIx32"
#define FMT_U64_HEX    "PRIx64"

#define FMT_U32_OCT    "PRIo32"
#define FMT_U64_OCT    "PRIo64"

extern long double const g_MachEpsilon;

bool    EssentiallyEqual(long double a, long double b);
bool    ApproximatelyEqual(long double a, long double b);
bool    DefinitelyGreaterThan(long double a, long double b);
bool    DefinitelyLessThan(long double a, long double b);
bool    CanCastTo(long double dValue, long double dMaxValueForDst);

#endif /* TYPES_H___ */

