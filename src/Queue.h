/* $Id: Queue.h 168 2011-10-12 17:24:32Z marshan $ */
/** @file
 * Generic queue (fifo - add tail, remove head) implementation header.
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

#ifndef NOPFQUEUE_H___
#define NOPFQUEUE_H___

#include <stdbool.h>
#include <inttypes.h>

typedef struct QUEUEITEM
{
    void                *pvData;
    struct QUEUEITEM    *pNext;
} QUEUEITEM;
typedef QUEUEITEM *PQUEUEITEM;
typedef const QUEUEITEM *PCQUEUEITEM;

typedef struct QUEUE
{
    QUEUEITEM           *pHead;
    QUEUEITEM           *pTail;
    uint32_t             cItems;
} QUEUE;
typedef QUEUE *PQUEUE;
typedef const QUEUE *PCQUEUE;

void QueueInit(PQUEUE pQueue);
uint32_t QueueSize(PQUEUE pQueue);
bool QueueIsEmpty(PQUEUE pQueue);
int QueueAdd(PQUEUE pQueue, void *pvData);
void *QueueRemove(PQUEUE pQueue);
void *QueuePeekHead(PQUEUE pQueue);
void *QueuePeekTail(PQUEUE pQueue);
void *QueueItemAt(PQUEUE pQueue, uint32_t uIndex);

#endif /* NOPFQUEUE_H___ */

