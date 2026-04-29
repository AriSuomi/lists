/** ****************************************************************************
 *
 * \file	lwl_slList.c
 *
 * \brief	Source file for the singly linked list implementation.
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

#include <stddef.h>
#include <stdbool.h>

#include <lwl/lwl_port.h>
#include <lwl/lwl_slList.h>

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

/*******************************************************************************
;
;	I N L I N E   F U N C T I O N S
;
;-----------------------------------------------------------------------------*/

extern inline void lwl_slListNodeInit(
	lwl_SlListNode * pNode
);

extern inline void lwl_slListInit(
	lwl_SlList * pList
);

extern inline lwl_SlListNode * lwl_slListPeekLast(
	const lwl_SlList * pList
);

extern inline void lwl_slListSneekLast(
	lwl_SlList *	 pList,
	lwl_SlListNode * pNode
);

extern inline lwl_SlListNode * lwl_slListPopLast(
	lwl_SlList * pList
);

extern inline bool lwl_slListIsNodeInList(
	const lwl_SlListNode * pNode
);

extern inline bool lwl_slListPushLast(
	lwl_SlList *	 pList,
	lwl_SlListNode * pNew
);

extern inline lwl_SlListNode * lwl_slListPeekFirst(
	const lwl_SlList * pList
);

extern inline lwl_SlListNode * lwl_slListPopFirst(
	lwl_SlList * pList
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
 * \brief		Insert the supplied node to the list.
 *
 * \param[in]	pList		Pointer to list.
 * \param[in]	pNew		Pointer to node that should be pushed.
 * \param[in]	pCmp		Pointer to node comparison information.
 *
 * \retval		true		The Node was inserted successfully.
 * \retval		false		The node could not be inserted.
 *
 ******************************************************************************/
bool lwl_slListInsert(
	lwl_SlList *		  pList,
	lwl_SlListNode *	  pNewNode,
	const lwl_SlListCmp * pCmp
) {
	bool wasInserted = false;

	lwl__portAssert(pList != NULL);
	lwl__portAssert(pNewNode != NULL);

	bool isInList = lwl_slListIsNodeInList(pNewNode);

	if (!isInList) {
		/*
		 * The node is not yet in the list. Iterate the list until the correct
		 * spot is found.
		 */

		lwl_SlListNode * pPrevListNode = &pList->sentinel;
		lwl_SlListNode * pListNode = pPrevListNode->pNext;

		while (pListNode != &pList->sentinel) {
			bool foundSpot = pCmp->pCmpFn(pListNode, pNewNode, pCmp->pCmpParam);

			if (foundSpot) {
				break;
			}

			pPrevListNode = pListNode;
			pListNode = pListNode->pNext;
		}

		/*
		 * Insert the new node into the list.
		 */
		pNewNode->pNext = pListNode;

		if (pListNode == &pList->sentinel) {
			/*
			 * The node is inserted as the last node.
			 */
			pList->pLast = pNewNode;
		}

		pPrevListNode->pNext = pNewNode;
		wasInserted = true;
	}

	return wasInserted;
}

/** ****************************************************************************
 *
 * \brief		Remove a node from the list.
 *
 * \param[in]	pList	Pointer to a list.
 * \param[in]	pNode	Pointer to node that should be removed.
 *
 * \retval		true	The Node was removed successfully.
 * \retval		false	The node could not be removed. It was not in any list or
 * 						it was not in the supplied list.
 *
 ******************************************************************************/
bool lwl_slListRemove(
	lwl_SlList *	 pList,
	lwl_SlListNode * pNode
) {
	bool wasRemoved = false;

	lwl__portAssert(pList != NULL);
	lwl__portAssert(pNode != NULL);
	lwl__portAssert(pList->sentinel.pNext != &pList->sentinel);

	bool isInList = lwl_slListIsNodeInList(pNode);

	if (!isInList && pList->sentinel.pNext != &pList->sentinel) {
		/*
		 * The node is in a list and the supplied list is not empty.
		 * Iterate the list until the node is found.
		 */
		lwl_SlListNode * pPrevListNode = &pList->sentinel;
		lwl_SlListNode * pListNode = pList->sentinel.pNext;

		while (pListNode != pNode && pListNode != &pList->sentinel) {
			pPrevListNode = pListNode;
			pListNode = pListNode->pNext;
		}

		if (pListNode != &pList->sentinel) {
			/*
			 * The node was found in the list. Remove it.
			 */
			pPrevListNode->pNext = pNode->pNext;
			if (pNode->pNext == &pList->sentinel) {
				/*
				 * The last node was removed.
				 */
				pList->pLast = &pList->sentinel;
			}

			/*
			 * Clear next pointer of the node.
			 */
			pNode->pNext = NULL;
			wasRemoved = true;
		}
	}

	return wasRemoved;
}
