/* $Id: Types.h 192 2014-05-14 06:52:35Z marshan $ */
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

/** Evaluator numeric types */
typedef long double FLOAT;
typedef const long double CFLOAT;
typedef FLOAT *PFLOAT;
typedef const FLOAT *PCFLOAT;

typedef uint64_t UINTEGER;
typedef const UINTEGER CUINTEGER;

typedef int64_t INTEGER;
typedef const INTEGER CINTEGER;

typedef UINTEGER U64INTEGER;
typedef INTEGER  S64INTEGER;

typedef uint32_t U32INTEGER;
typedef int32_t  S32INTEGER;

#define FABSFLOAT            fabsl
#define FPOWFLOAT            powl

/** Format specifiers for the evaluator types. */
#define FMTFLOAT             "Lg"

#define FMTU32INTEGER_NAT    PRIu32
#define FMTS32INTEGER_NAT    PRId32

#define FMTU64INTEGER_NAT    PRIu64
#define FMTS64INTEGER_NAT    PRId64

#define FMTUINTEGER_NAT      FMTU64INTEGER_NAT
#define FMTINTEGER_NAT       FMTS64INTEGER_NAT

#define FMTU32INTEGER_HEX    PRIx32
#define FMTU64INTEGER_HEX    PRIx64

#define FMTU32INTEGER_OCT    PRIo32
#define FMTU64INTEGER_OCT    PRIo64

/** Maximum ranges for the evaluator types. */
#define MAX_S32INTEGER       INT32_MAX
#define MAX_U32INTEGER       UINT32_MAX
#define MAX_S64INTEGER       INT64_MAX
#define MAX_U64INTEGER       UINT64_MAX

#define MIN_S32INTEGER       INT32_MIN
#define MIN_U32INTEGER       UINT32_MIN
#define MIN_S64INTEGER       INT64_MIN
#define MIN_U64INTEGER       UINT64_MIN

#define MAX_UINTEGER         MAX_U64INTEGER
#define MAX_INTEGER          MAX_S64INTEGER

#define MIN_UINTEGER         MIN_U64INTEGER
#define MIN_INTEGER          MIN_S64INTEGER

extern CFLOAT g_MachEpsilon;

bool EssentiallyEqual(FLOAT a, FLOAT b);
bool ApproximatelyEqual(FLOAT a, FLOAT b);
bool DefinitelyGreaterThan(FLOAT a, FLOAT b);
bool DefinitelyLessThan(FLOAT a, FLOAT b);
bool CanCastTo(FLOAT dValue, FLOAT dMaxValueForDst);

#endif /* TYPES_H___ */
