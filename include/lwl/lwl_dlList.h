/** ****************************************************************************
 *
 * 	\file	lwl_dlList.h
 *
 * 	\brief	Header file for the doubly linked list implementation.
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

#ifndef LWL_DLLIST_H_INCLUDED
#define LWL_DLLIST_H_INCLUDED

/*******************************************************************************
;
;	I N C L U D E S
;
;-----------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <lwl/lwl_port.h>

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
typedef struct lwl__dlListNode lwl_DlListNode;

/**
 * Type used by each list node.
 */
struct lwl__dlListNode {
	lwl_DlListNode * pNext; /**< Pointer to the next node.		*/
	lwl_DlListNode * pPrev; /**< Pointer to the previous node.	*/
};

/**
 * Type for the doubly linked list.
 */
typedef struct {
	lwl_DlListNode sentinel; /**< Sentinel first and last node. */
} lwl_DlList;

/** ****************************************************************************
 *
 * \brief	Type used for node compare functions.
 *
 * \param	pNode1	Pointer to the first node. This is the node already in the
 * 					list when inserting new nodes into a list
 * \param	pNode2 	Pointer to the second node. This is the new node when
 * 					inserting new nodes in lists.
 * \param	pParam	Pointer to comparison parameter.
 *
 * \retval	true	The first node should be before the second node.
 * \retval	false	The second node should be before the first node.
 *
 * \details	Used for functions that determine the order of the list. The second
 * 			node is always the new node when inserting nodes on a list.
 *
 ******************************************************************************/
typedef bool lwl_DlListCmpFn(
	const lwl_DlListNode * pNode1,
	const lwl_DlListNode * pNode2,
	const void *		   pParam
);

/**
 * Type used to store node comparing function and parameter.
 */
typedef struct {
	lwl_DlListCmpFn * pCmpFn;	 /**< Pointer to node comparing function. */
	const void *	  pCmpParam; /**< Node comparing function parameter. */
} lwl_DlListCmp;

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

bool lwl_dlListMoveNode(
	lwl_DlList *		  pList,
	lwl_DlListNode *	  pNode,
	const lwl_DlListCmp * pCmp
);

void lwl__dlListInsertBack(
	lwl_DlList *		  pList,
	lwl_DlListNode *	  pListNode,
	lwl_DlListNode *	  pNewNode,
	const lwl_DlListCmp * pCmp
);

/** ****************************************************************************
 *
 * 	\brief		Initialize list node.
 *
 * 	\param		pNode	Pointer to the node.
 *
 * 	\details	Initializes list node members to valid values for a node that
 * 				has not been added to any list yet.
 *
 ******************************************************************************/
inline void lwl_dlListNodeInit(
	lwl_DlListNode * pNode
) {
	if (pNode != NULL) {
		/* Nodes not in any list should have all pointers initilized to NULL. */
		pNode->pNext = NULL;
		pNode->pPrev = NULL;
	}
}

/** ****************************************************************************
 *
 * 	\brief		Initialize list.
 *
 * 	\param		pList	Pointer to the list.
 *
 * 	\details	Initializes the list member variables to their initial values.
 * 				The list is empty after this call.
 *
 ******************************************************************************/
inline void lwl_dlListInit(
	lwl_DlList * pList
) {
	/*
	 * Make the pointers of the sentinel node point to the sentinel node itself.
	 * This makes it possible to add and remove nodes without any conditional
	 * branches.
	 */
	pList->sentinel.pNext = &pList->sentinel;
	pList->sentinel.pPrev = &pList->sentinel;
}

/** ****************************************************************************
 *
 * 	\brief		Peek at the last node.
 *
 * 	\param		pList		Pointer to a list.
 *
 * 	\return		Pointer to the last node. NULL if the list is empty.
 *
 * 	\details	This function dos not modify the list in any way.
 *
 ******************************************************************************/
inline lwl_DlListNode * lwl_dlListPeekLast(
	const lwl_DlList * pList
) {
	lwl_DlListNode * pRetVal = pList->sentinel.pPrev;

	if (pRetVal == &pList->sentinel) {
		/*
		 * The pPrev pointer points to the sentinel node. The list is empty.
		 */
		pRetVal = NULL;
	}

	return pRetVal;
}

/** ****************************************************************************
 *
 * \brief	Sneek in a node to the last node pointer.
 *
 * \param	pList		Pointer to a list.
 * \param	pNode		Pointer to node that should be set as root node.
 *
 * \details	This is a much lighter operation than lwl_dlListPushLast. The
 * 			function can be used in conjunction with lwl_dlListPeekLast to
 * 			store and read arbitrary node pointers to the list while it is
 * 			empty.
 *
 * 			A NULL pointer must be snuck back before anything is inserted or
 * 			pushed to the list.
 *
 ******************************************************************************/
inline void lwl_dlListSneekLast(
	lwl_DlList *	 pList,
	lwl_DlListNode * pNode
) {
	pList->sentinel.pPrev = pNode;
}

/** ****************************************************************************
 *
 * 	\brief		Check if the supplied node is in a list.
 *
 * 	\param		pNode		The node to check.
 *
 * 	\retval		true		The node is in a list.
 * 	\retval		false		The node is not in a list.
 *
 ******************************************************************************/
inline bool lwl_dlListIsNodeInList(
	const lwl_DlListNode * pNode
) {
	/* A NULL value is only used for nodes not in a list. */
	return pNode->pNext != NULL;
}

/** ****************************************************************************
 *
 * \brief	Remove a node from the list.
 *
 * \param	pNode		Pointer to node that should be removed.
 *
 * \retval	true		The Node was removed successfully.
 * \retval	false		The node could not be removed.
 *
 ******************************************************************************/
inline bool lwl_dlListRemove(
	lwl_DlListNode * pNode
) {
	lwl__portAssert(pNode != NULL);
	bool isInList = lwl_dlListIsNodeInList(pNode);

	if (isInList) {
		lwl_DlListNode * pPrev = pNode->pPrev;
		lwl_DlListNode * pNext = pNode->pNext;

		pPrev->pNext = pNext;
		pNext->pPrev = pNode->pPrev;

		/*
		 * Set both pointers of the node to NULL to mark that it is not in any
		 * list anymore.
		 */
		pNode->pPrev = NULL;
		pNode->pNext = NULL;
	}

	return isInList;
}

/** ****************************************************************************
 *
 * 	\brief		Pop the last node.
 *
 * 	\param		pList 		Pointer to list.
 *
 * 	\return		Pointer to the popped node. NULL if the list was empty.
 *
 * 	\details	The last node is removed from the list and supplied as the
 * 				return value.
 *
 ******************************************************************************/
inline lwl_DlListNode * lwl_dlListPopLast(
	lwl_DlList * pList
) {
	lwl__portAssert(pList != NULL);

	lwl_DlListNode * pNode = lwl_dlListPeekLast(pList);
	if (pNode != NULL) {
		lwl_dlListRemove(pNode);
	}

	return pNode;
}

/** ****************************************************************************
 *
 * 	\brief		Insert a new node into the list before the specified node.
 *
 * 	\param		pListNode	Pointer to node on the list. The new node will be
 * 							inserted before this node.
 * 	\param		pNewNode	Pointer to the node that shoud be inserted.
 *
 ******************************************************************************/
inline void lwl_dlListInsertBefore(
	lwl_DlListNode * pListNode,
	lwl_DlListNode * pNewNode
) {
	/*
	 * pListNode may be the sentinel node but that does not matter. In that case
	 * the new node will be inserted last in the list.
	 */
	lwl_DlListNode * pPrevNode = pListNode->pPrev;
	pPrevNode->pNext = pNewNode;
	pNewNode->pPrev = pPrevNode;
	pListNode->pPrev = pNewNode;
	pNewNode->pNext = pListNode;
}

/** ****************************************************************************
 *
 * \brief		Push new node to the tail of the list.
 *
 * \param[in]	pList 		Pointer to list.
 * \param[in]	pNew		Pointer to the new node.
 *
 * \retval		true		The Node was pushed successfully.
 * \retval		false		The node could not be pushed.
 *
 ******************************************************************************/
inline bool lwl_dlListPushLast(
	lwl_DlList *	 pList,
	lwl_DlListNode * pNew
) {
	bool isInList = lwl_dlListIsNodeInList(pNew);

	if (isInList == false) {
		lwl_dlListInsertBefore(&pList->sentinel, pNew);
	}

	return !isInList;
}

/** ****************************************************************************
 *
 * \brief		Push new node to the front of the list.
 *
 * \param[in]	pList 		Pointer to list.
 * \param[in]	pNew		Pointer to the new node.
 *
 * \retval		true		The Node was pushed successfully.
 * \retval		false		The node could not be pushed.
 *
 ******************************************************************************/
inline bool lwl_dlListPushFirst(
	lwl_DlList *	 pList,
	lwl_DlListNode * pNew
) {
	bool isInList = lwl_dlListIsNodeInList(pNew);

	if (isInList == false) {
		lwl_dlListInsertBefore(pList->sentinel.pNext, pNew);
	}

	return !isInList;
}

/** ****************************************************************************
 *
 * 	\brief		Peek at the first node.
 *
 * 	\param		pList		Pointer to a list.
 *
 * 	\return		Pointer to the first node.
 *
 ******************************************************************************/
inline lwl_DlListNode * lwl_dlListPeekFirst(
	const lwl_DlList * pList
) {
	lwl__portAssert(pList != NULL);

	lwl_DlListNode * pHead = pList->sentinel.pNext;
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
inline lwl_DlListNode * lwl_dlListPopFirst(
	lwl_DlList * pList
) {
	lwl__portAssert(pList != NULL);

	lwl_DlListNode * pNode = lwl_dlListPeekFirst(pList);
	if (pNode != NULL) {
		lwl_dlListRemove(pNode);
	}

	return pNode;
}

/** ****************************************************************************
 *
 * 	\brief		Peek at the next node in the list.
 *
 * 	\param		pList		Pointer to the list.
 * 	\param		pNode		Pointer to a list node.
 *
 * 	\return		Pointer to the next node.
 *
 * 	\details	This function can be used to enumerate all nodes.
 *
 ******************************************************************************/
inline lwl_DlListNode * lwl_dlListPeekNext(
	const lwl_DlList *	   pList,
	const lwl_DlListNode * pNode
) {
	lwl_DlListNode * pNext = NULL;

	lwl__portAssert(pNode != NULL);

	bool isInList = lwl_dlListIsNodeInList(pNode);
	if (isInList == true) {
		if (pNode->pNext != &pList->sentinel) {
			pNext = pNode->pNext;
		}
	}

	return pNext;
}

/** ****************************************************************************
 *
 * \brief		Insert the supplied node to the list.
 *
 * \param[in]	pList		Pointer to list.
 * \param[in]	isInList	Pointer to node that should be pushed.
 * \param[in]	pCmp		Pointer to node comparison information.
 *
 * \retval		true		The Node was inserted successfully.
 * \retval		false		The node could not be inserted.
 *
 * \details		The node will be inserted into the list to the correct spot
 * 				determined by the compare function.
 *
 ******************************************************************************/
inline bool lwl_dlListInsert(
	lwl_DlList *		  pList,
	lwl_DlListNode *	  pNewNode,
	const lwl_DlListCmp * pCmp
) {
	lwl__portAssert(pList != NULL);
	lwl__portAssert(pNewNode != NULL);

	bool isInList = lwl_dlListIsNodeInList(pNewNode);

	if (isInList == false) {
		lwl__dlListInsertBack(
			pList,
			pList->sentinel.pNext,
			pNewNode,
			pCmp
		);
	}

	return !isInList;
}

#endif // LWL_DLLIST_H_INCLUDED
