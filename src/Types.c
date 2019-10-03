/** @file
 * Internal types.
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

#include "Types.h"

/** IEEE 754 - 2008 Epsilon for long double precision. */
long double const g_MachEpsilon = 9.63e-35;
/* 1.11e-16   --- double */

/*
 * The double comparison code is lifted from the "Art of Computer Programming".
 * "Beware of bugs in the above code; I have only proved it correct, not tried it"
 * -- Donald Knuth
 */
bool ApproximatelyEqual(long double a, long double b)
{
    return fabsl(a - b) <= ((fabsl(a) < fabsl(b) ? fabsl(b) : fabsl(a)) * g_MachEpsilon);
}

bool EssentiallyEqual(long double a, long double b)
{
    return fabsl(a - b) <= ((fabsl(a) > fabsl(b) ? fabsl(b) : fabsl(a)) * g_MachEpsilon);
}

bool DefinitelyGreaterThan(long double a, long double b)
{
    return (a - b) > ((fabsl(a) < fabsl(b) ? fabsl(b) : fabsl(a)) * g_MachEpsilon);
}

bool DefinitelyLessThan(long double a, long double b)
{
    return (b - a) > ((fabsl(a) < fabsl(b) ? fabsl(b) : fabsl(a)) * g_MachEpsilon);
}

bool CanCastTo(long double dValue, long double dMaxValueForDst)
{
    return !DefinitelyGreaterThan(dValue, dMaxValueForDst);
}

