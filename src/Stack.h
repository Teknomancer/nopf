/** @file
 * Generic stack (lifo) implementation header.
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

#ifndef NOPFSTACK_H___
#define NOPFSTACK_H___

#include <stdbool.h>
#include <inttypes.h>

/**
 * STACKITEM: Represents an item on the stack.
 */
typedef struct STACKITEM
{
    /** Opaque private data pointer. */
    void                *pvData;
    /** Pointer to the next item on the stack. */
    struct STACKITEM    *pNext;
} STACKITEM;
/** Pointer to a Stack item object. */
typedef STACKITEM *PSTACKITEM;
/** Pointer to a const Stack item object. */
typedef const STACKITEM *PCSTACKITEM;

/**
 * STACK: A stack object.
 */
typedef struct STACK
{
    /** Pointer to the top of the stack. */
    STACKITEM           *pTop;
    /** Number of items on the stack. */
    uint32_t             cItems;
} STACK;
/** Pointer to a Stack object. */
typedef STACK *PSTACK;
/** Pointer to a const Stack object. */
typedef const STACK *PCSTACK;

/** Stack access routines. */
void StackInit(PSTACK pStack);
uint32_t StackSize(PSTACK pStack);
bool StackIsEmpty(PSTACK pStack);
int StackPush(PSTACK pStack, void *pvData);
void *StackPop(PSTACK pStack);
void *StackPeek(PSTACK pStack);

#endif /* NOPFSTACK_H___ */

