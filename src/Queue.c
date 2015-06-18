/* $Id: Queue.c 168 2011-10-12 17:24:32Z marshan $ */
/** @file
 * Generic queue (fifo - add tail, remove head) implementation.
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

#include "Queue.h"
#include "Errors.h"
#include "StringOps.h"

inline void QueueInit(PQUEUE pQueue)
{
    pQueue->pTail = NULL;
    pQueue->pHead = NULL;
    pQueue->cItems = 0;
}

uint32_t QueueSize(PQUEUE pQueue)
{
    return pQueue->cItems;
}

inline bool QueueIsEmpty(PQUEUE pQueue)
{
    return pQueue->pHead ? false : true;
}

inline int QueueAdd(PQUEUE pQueue, void *pvData)
{
    PQUEUEITEM pNode = MemAlloc(sizeof(QUEUEITEM));
    if (pNode)
    {
        pNode->pvData = pvData;
        if (QueueIsEmpty(pQueue))
        {
            pNode->pNext = NULL;
            pQueue->pHead = pNode;
            pQueue->pTail = pNode;
        }
        else
        {
            pNode->pNext = NULL;
            pQueue->pTail->pNext = pNode;
            pQueue->pTail = pNode;
        }
        ++pQueue->cItems;
        return RINF_SUCCESS;
    }
    else
        return RERR_NO_MEMORY;
}

inline void *QueueRemove(PQUEUE pQueue)
{
    if (!QueueIsEmpty(pQueue))
    {
        PQUEUEITEM pNode = pQueue->pHead;
        pQueue->pHead = pNode->pNext;
        void *pvData = pNode->pvData;
        MemFree(pNode);
        --pQueue->cItems;
        return pvData;
    }

    return NULL;
}
	
inline void *QueuePeekHead(PQUEUE pQueue)
{
    if (pQueue->pHead)
        return pQueue->pHead->pvData;
    return NULL;
}

inline void *QueuePeekTail(PQUEUE pQueue)
{
    if (pQueue->pTail)
        return pQueue->pTail->pvData;
    return NULL;
}

inline void *QueueItemAt(PQUEUE pQueue, uint32_t uIndex)
{
    if (uIndex >= pQueue->cItems)
        return NULL;

    PQUEUEITEM pNode = pQueue->pHead;
    uint32_t i = 0;
    while (pNode)
    {
        if (i == uIndex)
            return pNode->pvData;
        ++i;
        pNode = pNode->pNext;
    }
    return NULL;
}

