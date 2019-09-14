/** @file
 * Generic double linked list, implementation.
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

#include "List.h"

#include "Assert.h"
#include "Errors.h"
#include "StringOps.h"

void ListInit(PLIST pList)
{
    pList->pHead = NULL;
    pList->pTail = NULL;
    pList->cItems = 0;
}

uint32_t ListSize(PLIST pList)
{
    return pList->cItems;
}

bool ListIsEmpty(PLIST pList)
{
    return pList->cItems == 0 ? true : false;
}

int ListAdd(PLIST pList, void *pvData)
{
    PLISTITEM pNode = MemAlloc(sizeof(LISTITEM));
    if (pNode)
    {
        if (!pList->pHead)
        {
            pList->pHead = pNode;
            pList->pTail = pNode;
            pNode->pPrev = NULL;
        }
        else
        {
            pNode->pPrev = pList->pTail;
            pList->pTail->pNext = pNode;
            pList->pTail = pNode;
        }
        pNode->pNext = NULL;
        pNode->pvData = pvData;
        pList->cItems++;
        return RINF_SUCCESS;
    }
    return RERR_NO_MEMORY;
}

static void ListRemoveNode(PLIST pList, PLISTITEM pNode)
{
    if (pNode->pNext)
        pNode->pNext->pPrev = pNode->pPrev;
    if (pNode->pPrev)
        pNode->pPrev->pNext = pNode->pNext;
    if (pNode == pList->pHead)
        pList->pHead = pNode->pNext;
    else if (pNode == pList->pTail)
        pList->pTail = pNode->pPrev;
    MemFree(pNode);
    pList->cItems--;
}

void ListRemove(PLIST pList, void *pvData)
{
    PLISTITEM pNode = pList->pHead;
    while (pNode)
    {
        if (pNode->pvData == pvData)
        {
            ListRemoveNode(pList, pNode);
            return;
        }
        pNode = pNode->pNext;
    }
}

void *ListRemoveItemAt(PLIST pList, uint32_t uIndex)
{
    if (uIndex >= pList->cItems)
        return NULL;

    PLISTITEM pNode = pList->pHead;
    uint32_t i = 0;
    while (pNode)
    {
        if (i == uIndex)
        {
            void *pvData = pNode->pvData;
            ListRemoveNode(pList, pNode);
            return pvData;
        }
        pNode = pNode->pNext;
        ++i;
    }
    return NULL;
}

void *ListItemAt(PLIST pList, uint32_t uIndex)
{
    if (uIndex >= pList->cItems)
        return NULL;

    PLISTITEM pNode = pList->pHead;
    uint32_t i = 0;
    while (pNode)
    {
        if (i == uIndex)
            return pNode->pvData;

        pNode = pNode->pNext;
        ++i;
    }
    return NULL;
}

void ListAppend(PLIST pList, PLIST pSrcList)
{
    Assert(pSrcList);
    PLISTITEM pNode = pSrcList->pHead;
    while (pNode)
    {
        ListAdd(pList, pNode->pvData);
        pNode = pNode->pNext;
    }
}

