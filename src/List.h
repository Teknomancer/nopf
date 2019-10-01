/** @file
 * Generic double linked list, header.
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

#ifndef NOPFLIST_H___
#define NOPFLIST_H___

#include <stdbool.h>
#include <inttypes.h>

typedef struct LISTITEM
{
    void               *pvData;     /**< Pointer to the data. */
    struct LISTITEM    *pNext;      /**< Pointer to the next item. */
    struct LISTITEM    *pPrev;      /**< Pointer to the previous item. */
} LISTITEM;
/** Pointer to a list item. */
typedef LISTITEM *PLISTITEM;
/** Pointer to a const list item. */
typedef const LISTITEM *PCLISTITEM;

typedef struct LIST
{
    PLISTITEM           pHead;      /**< Pointer to the head. */
    PLISTITEM           pTail;      /**< Pointer to the tail. */
    uint32_t            cItems;     /**< Number of items. */
} LIST;
/** Pointer to a list. */
typedef LIST *PLIST;
/** Pointer to a const list. */
typedef const LIST *PCLIST;

void        ListInit(PLIST pList);
uint32_t    ListSize(PLIST pList);
bool        ListIsEmpty(PLIST pList);
int         ListAdd(PLIST pList, void *pvData);
void        ListRemove(PLIST pList, void *pvData);
void       *ListRemoveItemAt(PLIST pList, uint32_t uIndex);
void       *ListItemAt(PLIST pList, uint32_t uIndex);
void        ListAppend(PLIST pList, PLIST pSrcList);

#endif /* NOPFLIST_H___ */

