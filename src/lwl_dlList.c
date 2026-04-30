/** ****************************************************************************
 *
 * \file	lwl_dlList.c
 *
 * \brief	Source file for the doubly linked list implementation.
 *
 * \details
 *
 ******************************************************************************/
/*
 *  2026-04-29
 *
 *  Copyright (c) Ari Suomi
 *
 *------------------------------------------------------------------------------
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/*******************************************************************************
;
;	I N C L U D E S
;
;-----------------------------------------------------------------------------*/

#include <stdbool.h>

#include "lwl/lwl_dlList.h"

/*******************************************************************************
;
;	D E F I N E S
;
;-----------------------------------------------------------------------------*/

/*******************************************************************************
;
;	T Y P E S
;
;-----------------------------------------------------------------------------*/

/*******************************************************************************
;
;	S T A T I C   F U N C T I O N S
;
;-----------------------------------------------------------------------------*/

static void lwl__dlListInsertFront(
	lwl_DlList *		  pList,
	lwl_DlListNode *	  pListNode,
	lwl_DlListNode *	  pNewNode,
	const lwl_DlListCmp * pCmp
);

/*******************************************************************************
;
;	I N L I N E   F U N C T I O N S
;
;-----------------------------------------------------------------------------*/

extern inline void lwl_dlListNodeInit(
	lwl_DlListNode * pNode
);

extern inline void lwl_dlListInit(
	lwl_DlList * pList
);

extern inline lwl_DlListNode * lwl_dlListPeekLast(
	const lwl_DlList * pList
);

extern inline void lwl_dlListSneekLast(
	lwl_DlList *	 pList,
	lwl_DlListNode * pNode
);

extern inline void lwl_dlListSneekFirst(
	lwl_DlList *	 pList,
	lwl_DlListNode * pNode
);

extern inline lwl_DlListNode * lwl_dlListPopLast(
	lwl_DlList * pList
);

extern inline bool lwl_dlListIsNodeInList(
	const lwl_DlListNode * pNode
);

extern inline void lwl_dlListInsertBefore(
	lwl_DlListNode * pListNode,
	lwl_DlListNode * pNewNode
);

extern inline bool lwl_dlListPushLast(
	lwl_DlList *	 pList,
	lwl_DlListNode * pNew
);

extern inline lwl_DlListNode * lwl_dlListPeekFirst(
	const lwl_DlList * pList
);

extern inline lwl_DlListNode * lwl_dlListPopFirst(
	lwl_DlList * pList
);

extern inline lwl_DlListNode * lwl_dlListPeekNext(
	const lwl_DlList *	   pList,
	const lwl_DlListNode * pNode
);

extern inline bool lwl_dlListRemove(
	lwl_DlListNode * pNode
);

extern inline bool lwl_dlListInsert(
	lwl_DlList *		  pList,
	lwl_DlListNode *	  pNewNode,
	const lwl_DlListCmp * pCmp
);

/*******************************************************************************
;
;	D A T A
;
;-----------------------------------------------------------------------------*/

/*******************************************************************************
;
;	F U N C T I O N   D E F I N I T I O N S
;
;-----------------------------------------------------------------------------*/

/** ****************************************************************************
 *
 * \brief	Update node position in the list.
 *
 * \param	pList		Pointer to the heap.
 * \param	pNode		The node that should be moved.
 * \param	pCmp		Poninter to comparison information.
 *
 * \retval	true		The node position was updated.
 * \retval	false		Could not update node position.
 *
 * \details	This function should be called if the node has been modified so that
 * 			the comparison function might return another result than before.
 *
 ******************************************************************************/
bool lwl_dlListMoveNode(
	lwl_DlList *		  pList,
	lwl_DlListNode *	  pNode,
	const lwl_DlListCmp * pCmp
) {
	/*
	 * First remove the node from the list.
	 */
	lwl_DlListNode * pNext = pNode->pNext;
	bool			 success = lwl_dlListRemove(pNode);

	if (success) {
		/*
		 * Check if it should be moved forward or backward in the list. Insert
		 * the node again by searching for the new spot in the right direction.
		 */
		bool toTheBack = pCmp->pCmpFn(pNext, pNode, pCmp->pCmpParam);

		if (toTheBack) {
			lwl__dlListInsertBack(pList, pNext, pNode, pCmp);
		} else {
			lwl__dlListInsertFront(pList, pNext, pNode, pCmp);
		}
	}

	return success;
}

/** ****************************************************************************
 *
 * \brief	Insert node to the correct spot after the specified node.
 *
 * \param	pList		Pointer to the heap.
 * \param	pListNode	Pointer to node on the list. The search for the correct
 * 						insertion spot will start from this node.
 * \param	pNewNode	The new node that should be inserte.
 * \param	pCmp		Pointer to comparison information.
 *
 ******************************************************************************/
void lwl__dlListInsertBack(
	lwl_DlList *		  pList,
	lwl_DlListNode *	  pListNode,
	lwl_DlListNode *	  pNewNode,
	const lwl_DlListCmp * pCmp
) {
	/*
	 * Traverse the list until the right insertion spot is found.
	 */
	while (pListNode != &pList->sentinel) {
		bool notFound = pCmp->pCmpFn(pListNode, pNewNode, pCmp->pCmpParam);

		if (!notFound) {
			/* Found the spot. The new node should be before this list node. */
			break;
		}

		pListNode = pListNode->pNext;
	}

	lwl_dlListInsertBefore(pListNode, pNewNode);
}

/** ****************************************************************************
 *
 * \brief	Insert node to the correct spot before the specified node.
 *
 * \param	pList		Pointer to the heap.
 * \param	pListNode	Pointer to node on the list. The search for the correct
 * 						insertion spot will start from this node.
 * \param	pNewNode	The new node that should be inserte.
 * \param	pCmp		Pointer to comparison information.
 *
 ******************************************************************************/
static void lwl__dlListInsertFront(
	lwl_DlList *		  pList,
	lwl_DlListNode *	  pListNode,
	lwl_DlListNode *	  pNewNode,
	const lwl_DlListCmp * pCmp
) {
	/*
	 * Traverse the list until the right insertion spot is found.
	 */
	while (pListNode != &pList->sentinel) {
		bool foundSpot = pCmp->pCmpFn(pListNode, pNewNode, pCmp->pCmpParam);

		if (foundSpot) {
			break;
		}

		pListNode = pListNode->pPrev;
	}

	lwl_dlListInsertBefore(pListNode->pNext, pNewNode);
}
