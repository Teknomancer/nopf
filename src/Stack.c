/** @file
 * Generic stack (lifo) implementation.
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

#include "Stack.h"
#include "Errors.h"
#include "StringOps.h"

void StackInit(PSTACK pStack)
{
    pStack->pTop = NULL;
    pStack->cItems = 0;
}

bool StackIsEmpty(PSTACK pStack)
{
    return pStack->pTop ? false : true;
}

int StackPush(PSTACK pStack, void *pvData)
{
    PSTACKITEM pNode = MemAlloc(sizeof(STACKITEM));
    if (pNode)
    {
        pNode->pvData = pvData;
        if (StackIsEmpty(pStack))
        {
            pNode->pNext = NULL;
            pStack->pTop = pNode;
        }
        else
        {
            pNode->pNext = pStack->pTop;
            pStack->pTop = pNode;
        }

        ++pStack->cItems;
        return RINF_SUCCESS;
    }
    else
        return RERR_NO_MEMORY;
}

void *StackPeek(PSTACK pStack)
{
    if (!StackIsEmpty(pStack))
    {
        PSTACKITEM pNode = pStack->pTop;
        return pNode->pvData;
    }

    return NULL;
}

void *StackPop(PSTACK pStack)
{
    if (   pStack
        && !StackIsEmpty(pStack))
    {
        PSTACKITEM pNode = pStack->pTop;
        pStack->pTop = pNode->pNext;
        void *pvData = pNode->pvData;
        MemFree(pNode);

        --pStack->cItems;
        return pvData;
    }

    return NULL;
}

uint32_t StackSize(PSTACK pStack)
{
    return pStack->cItems;
}

