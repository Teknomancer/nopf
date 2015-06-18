/* $Id: Assert.h 189 2013-12-26 14:41:41Z marshan $ */
/** @file
 * Generic Assert routines header.
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

#ifndef ASSERT_H___
#define ASSERT_H___

#include <stdlib.h>

#ifdef _DEBUG
# define Assert(expr)  \
    do { \
        if (!(expr)) \
        { \
            AssertMsg(#expr, __LINE__, __FILE__, __PRETTY_FUNCTION__); \
            abort(); \
        } \
    } while (0)
#else
# define Assert(expr)     do { } while (0)
#endif

#define AssertReturn(expr, rc)  \
    do { \
        if (!(expr)) \
        { \
            AssertMsg(#expr, __LINE__, __FILE__, __PRETTY_FUNCTION__); \
            return (rc); \
        } \
    } while (0)

#define AssertReturnVoid(expr)  \
    do { \
        if (!(expr)) \
        { \
            AssertMsg(#expr, __LINE__, __FILE__, __PRETTY_FUNCTION__); \
            return; \
        } \
    } while (0)

#define AssertCompile(expr) \
    { \
       char achCompileTimeAssertFailed_[(expr) ? 1 : -1]; \
       achCompileTimeAssertFailed_[0] = 0; \
    }

extern void AssertMsg(char *pszExpr, unsigned uLine, char *pszFile, const char *pszFunction);

#endif /* ASSERT_H___ */

