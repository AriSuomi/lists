/** ****************************************************************************
 *
 * 	\file	lwl_slList.h
 *
 * 	\brief	Header file for the singly linked list implementation.
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

#ifndef LWL_SLLIST_H_INCLUDED
#define LWL_SLLIST_H_INCLUDED

/*******************************************************************************
;
;	I N C L U D E S
;
;-----------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "lwl/lwl_port.h"

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

/**
 * Type used by each list node.
 */
typedef struct lwl__slListNode lwl_SlListNode;

/**
 * Type used by each list node.
 */
struct lwl__slListNode {
	lwl_SlListNode * pNext; /**< Pointer to the next node.		*/
};

/**
 * Type for the singly linked list.
 */
typedef struct {
	lwl_SlListNode	 sentinel;
	lwl_SlListNode * pLast; /**< Pointer to last node.			*/
} lwl_SlList;

/** ****************************************************************************
 *
 * \brief		Type used for node compare functions.
 *
 * \param[in]	pNode1	Pointer to the first node. This is the node already in
 * 						the list when inserting new nodes into a list
 * \param[in]	pNode2 	Pointer to the second node. This is the new node when
 * 						inserting new nodes in lists.
 * \param[in]	pParam	Pointer to comparison parameter.
 *
 * \retval		true	The first node should be before the second node.
 * \retval		false	The second node should be before the first node.
 *
 * \details		Used for functions that determine the order of the list. The
 * 				second node is always the new node when inserting nodes on a
 * 				list. This means that returning true for equal nodes causes the
 * 				list to be first in first out.
 *
 ******************************************************************************/
typedef bool lwl_SlListCmpFn(
	const lwl_SlListNode * pNode1,
	const lwl_SlListNode * pNode2,
	const void *		   pParam
);

/**
 * Type used to store node comparing function and parameter.
 */
typedef struct {
	lwl_SlListCmpFn * pCmpFn;	 /**< Pointer to node comparing function. */
	const void *	  pCmpParam; /**< Node comparing function parameter. */
} lwl_SlListCmp;

/*******************************************************************************
;
;	D A T A
;
;-----------------------------------------------------------------------------*/

/*******************************************************************************
;
;	F U N C T I O N S
;
;-----------------------------------------------------------------------------*/

bool lwl_slListRemove(
	lwl_SlList *	 pList,
	lwl_SlListNode * pNode
);

/** ****************************************************************************
 *
 * 	\brief		Initialize list node.
 *
 * 	\param[out]	pNode	Pointer to the node.
 *
 ******************************************************************************/
inline void lwl_slListNodeInit(
	lwl_SlListNode * pNode
) {
	pNode->pNext = NULL;
}

/** ****************************************************************************
 *
 * 	\brief		Initialize list.
 *
 * 	\param[out]	pList	Pointer to the list.
 *
 ******************************************************************************/
inline void lwl_slListInit(
	lwl_SlList * pList
) {
	pList->sentinel.pNext = &pList->sentinel;
	pList->pLast = &pList->sentinel;
}

/** ****************************************************************************
 *
 * 	\brief		Peek at the last node.
 *
 * 	\param[in]	pList		Pointer to a list.
 *
 * 	\return		Pointer to the last node. NULL if the list is empty.
 *
 ******************************************************************************/
inline lwl_SlListNode * lwl_slListPeekLast(
	const lwl_SlList * pList
) {
	lwl_SlListNode * pLast = pList->pLast;
	if (pLast == &pList->sentinel) {
		pLast = NULL;
	}

	return pLast;
}

/** ****************************************************************************
 *
 * \brief		Quickly store a node pointer to the list.
 *
 * \param		pList		Pointer to a list.
 * \param[in]	pNode		The pointer that should be stored.
 *
 * \details		This function can be used to store one node pointer to the list
 * 				instead of using it as a list. The stored pointer can be read
 * 				with lwl_slListPeekLast(). It is a much lighter operation than
 * 				lwl_slListPushLast().
 *
 * \note		The list must be empty before storing a node pointer with this
 * 				function. A pointer to the sentinel node of the list must be
 * 				stored back before anything is inserted or pushed to the list.
 *
 ******************************************************************************/
inline void lwl_slListStorePtr(
	lwl_SlList *	 pList,
	lwl_SlListNode * pNode
) {
	lwl__portAssert(
		(pList->pLast == &pList->sentinel) ||
		(pNode == &pList->sentinel)
	);

	pList->pLast = pNode;
}

/** ****************************************************************************
 *
 * \brief		Pop the last node.
 *
 * \param		pList 		Pointer to list.
 *
 * \return		Pointer to the popped node. NULL if the list was empty.
 *
 ******************************************************************************/
inline lwl_SlListNode * lwl_slListPopLast(
	lwl_SlList * pList
) {
	lwl__portAssert(pList != NULL);

	lwl_SlListNode * pNode = lwl_slListPeekLast(pList);
	if (pNode != NULL) {
		lwl_slListRemove(pList, pNode);
	}

	return pNode;
}

/** ****************************************************************************
 *
 * 	\brief		Check if the supplied node is in a list.
 *
 * 	\param[in]	pNode		The node to check.
 *
 * 	\retval		true		The node is in a list.
 * 	\retval		false		The node is not in a list.
 *
 ******************************************************************************/
inline bool lwl_slListIsNodeInList(
	const lwl_SlListNode * pNode
) {
	return pNode->pNext != NULL;
}

/** ****************************************************************************
 *
 * \brief		Push new node to the tail of the list.
 *
 * \param		pList 	Pointer to list.
 * \param		pNew	Pointer to the new node.
 *
 * \retval		true	The Node was pushed successfully.
 * \retval		false	The node could not be pushed.
 *
 ******************************************************************************/
inline bool lwl_slListPushLast(
	lwl_SlList *	 pList,
	lwl_SlListNode * pNew
) {
	bool wasPushed = false;

	bool isInList = lwl_slListIsNodeInList(pNew);

	if (!isInList) {
		pList->pLast->pNext = pNew;
		pList->pLast = pNew;
		pNew->pNext = &pList->sentinel;

		wasPushed = true;
	}

	return wasPushed;
}

/** ****************************************************************************
 *
 * 	\brief		Peek at the first node.
 *
 * 	\param[in]	pList		Pointer to a list.
 *
 * 	\return		Pointer to the first node. NULL if the list is empty.
 *
 ******************************************************************************/
inline lwl_SlListNode * lwl_slListPeekFirst(
	const lwl_SlList * pList
) {
	lwl__portAssert(pList != NULL);

	lwl_SlListNode * pHead = pList->sentinel.pNext;
	if (pHead == &pList->sentinel) {
		pHead = NULL;
	}

	return pHead;
}

/** ****************************************************************************
 *
 * 	\brief		Pop the first node.
 *
 * 	\param		pList		Pointer to a list.
 *
 * 	\return		Pointer to the popped node. NULL if the list was empty.
 *
 ******************************************************************************/
inline lwl_SlListNode * lwl_slListPopFirst(
	lwl_SlList * pList
) {
	lwl__portAssert(pList != NULL);

	lwl_SlListNode * pNode = lwl_slListPeekFirst(pList);
	if (pNode != NULL) {
		lwl_slListRemove(pList, pNode);
	}

	return pNode;
}

/** ****************************************************************************
 *
 * \brief		Insert the supplied node to the list.
 *
 * \param		pList		Pointer to list.
 * \param		pNewNode	Pointer to node that should be inserted.
 * \param[in]	pCmp		Pointer to node comparison information.
 *
 * \retval		true		The Node was inserted successfully.
 * \retval		false		The node could not be inserted.
 *
 ******************************************************************************/
inline bool lwl_slListInsert(
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
		lwl_SlListCmpFn * pCmpFn = pCmp->pCmpFn;
		const void *	  pCmpParam = pCmp->pCmpParam;
		lwl_SlListNode *  pPrevListNode = &pList->sentinel;
		lwl_SlListNode *  pListNode = pPrevListNode->pNext;

		while (pListNode != &pList->sentinel) {
			bool notFound = pCmpFn(pListNode, pNewNode, pCmpParam);

			if (!notFound) {
				/* Found the spot. The new node should be before this list node. */
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

#endif // LWL_SLLIST_H_INCLUDED
