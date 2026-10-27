/** ****************************************************************************
 *
 * 	\file	lwl_mdlList.h
 *
 * 	\brief	Header file for the multiple doubly linked lists implementation.
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

#ifndef LWL_MDLLIST_H_INCLUDED
#define LWL_MDLLIST_H_INCLUDED

/*******************************************************************************
;
;	I N C L U D E S
;
;-----------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "lwl/lwl_port.h"
#include "lwl/lwl_dlList.h"
#include "lwl/lwl_mdlListTypes.h"

/*******************************************************************************
;
;	D E F I N E S
;
;-----------------------------------------------------------------------------*/

/**
 * Invalid list index value. Used for nodes not on the list.
 */
#define LWL__MDLLIST_INVALID_IDX 255U

/*******************************************************************************
;
;	T Y P E S
;
;-----------------------------------------------------------------------------*/

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

/** ****************************************************************************
 *
 * \brief		Initialize list node.
 *
 * \param[out]	pNode	Pointer to the node.
 *
 ******************************************************************************/
inline void lwl_mdlListNodeInit(
	lwl_MdlListNode * pNode
) {
	lwl__portAssert(pNode != NULL);

	lwl_dlListNodeInit(&pNode->dlListNode);
	pNode->listIdx = LWL__MDLLIST_INVALID_IDX;
}

/** ****************************************************************************
 *
 * \brief		Initialize list.
 *
 * \param[out]	pMdlList	Pointer to the list.
 * \param[out]	pDlLists	Pointer to array of lists.
 * \param		listCount	Number of lists in the array. Must be a value from
 * 							1 to 16.
 *
 ******************************************************************************/
inline void lwl_mdlListInit(
	lwl_MdlList * pMdlList,
	lwl_DlList *  pDlLists,
	uint8_t		  listCount
) {
	lwl__portAssert(pMdlList != NULL);
	lwl__portAssert(pDlLists != NULL);

	if (listCount > 0U && listCount <= sizeof(uint16_t) * CHAR_BIT) {
		for (uint8_t listIdx = 0; listIdx < listCount; listIdx++) {
			lwl_dlListInit(&pDlLists[listIdx]);
		}

	} else {
		lwl__portAssert(false);
	}

	pMdlList->pLists = pDlLists;
	pMdlList->listMap = 0U;
}

/** ****************************************************************************
 *
 * \brief		Insert the supplied node to the list.
 *
 * \param		pMdlList	Pointer to list.
 * \param		pNew		Pointer to node that should be inserted.
 * \param		listIdx		The list index. This determines which list should be
 *							used. Higher numbers will be popped first. The value
 *							may not be equal or exceed the listCount supplied to
 *							lwl_mdlListInit().
 *
 * \retval		true		The Node was inserted successfully.
 * \retval		false		The node could not be inserted.
 *
 ******************************************************************************/
inline bool lwl_mdlListInsert(
	lwl_MdlList *	  pMdlList,
	lwl_MdlListNode * pNew,
	uint8_t			  listIdx
) {
	lwl__portAssert(pMdlList != NULL);
	lwl__portAssert(pNew != NULL);

	bool wasInserted = false;

	if (listIdx < sizeof(uint16_t) * CHAR_BIT) {
		wasInserted = lwl_dlListPushLast(
			&pMdlList->pLists[listIdx],
			&pNew->dlListNode
		);

		if (wasInserted) {
			pNew->listIdx = listIdx;
			pMdlList->listMap |= (uint16_t)(1U << listIdx);
		}

	} else {
		lwl__portAssert(false);
	}

	return wasInserted;
}

/** ****************************************************************************
 *
 * \brief		Store a pointer to an empty list in the fastest possible way.
 *
 * \param		pMdlList	Pointer to list.
 * \param		pNode		Pointer to node that should be stored.
 *
 * \details		This can only be done with an empty list. The stored pointer
 * 				must be cleared by storing a NULL pointer before nodes can be
 * 				inserted to the list again.
 *
 * 				The stored pointer can be read with lwl_mdlListGetStoredPtr().
 *
 ******************************************************************************/
inline void lwl_mdlListStorePtr(
	lwl_MdlList *	  pMdlList,
	lwl_MdlListNode * pNew
) {
	lwl_dlListSneekLast(&pMdlList->pLists[0], &pNew->dlListNode);
}

/** ****************************************************************************
 *
 * \brief		Get the pointer stored with lwl_mdlListStorePtr.
 *
 * \param[in]	pMdlList	Pointer to list.
 *
 * \return		The pointer that has been stored with lwl_mdlListStorePtr().
 *
 ******************************************************************************/
inline lwl_MdlListNode * lwl_mdlListGetStoredPtr(
	const lwl_MdlList * pMdlList
) {
	return (lwl_MdlListNode *)lwl_dlListPeekLast(&pMdlList->pLists[0]);
}

/** ****************************************************************************
 *
 * \brief		Remove a node from the list.
 *
 * \param		pMdlList	Pointer to a list.
 * \param		pNode		Pointer to node that should be removed.
 *
 * \retval		true		The Node was removed successfully.
 * \retval		false		The node could not be removed.
 *
 ******************************************************************************/
inline bool lwl_mdlListRemove(
	lwl_MdlList *	  pMdlList,
	lwl_MdlListNode * pNode
) {
	lwl__portAssert(pMdlList != NULL);
	lwl__portAssert(pNode != NULL);
	lwl__portAssert(pNode->listIdx != LWL__MDLLIST_INVALID_IDX);

	bool wasRemoved = lwl_dlListRemove(&pNode->dlListNode);

	if (wasRemoved) {
		uint_fast8_t	 listIdx = pNode->listIdx;
		lwl_DlList *	 pList = &pMdlList->pLists[listIdx];
		lwl_DlListNode * pLastNode = lwl_dlListPeekLast(pList);
		if (pLastNode == NULL) {
			/*
			 * The last node was removed from this list. Clear the bit in the list
			 * map.
			 */
			pMdlList->listMap &= (uint16_t)(~(1U << listIdx));
		}

		pNode->listIdx = LWL__MDLLIST_INVALID_IDX;
	}

	return wasRemoved;
}

/** ****************************************************************************
 *
 * 	\brief		Peek at the first node.
 *
 * 	\param[in]	pMdlList	Pointer to a list.
 *
 * 	\return		Pointer to the first node. Returns NULL if the list is empty.
 *
 ******************************************************************************/
static inline lwl_MdlListNode * lwl_mdlListPeekFirst(
	const lwl_MdlList * pMdlList
) {
	lwl__portAssert(pMdlList != NULL);

	lwl_MdlListNode * pFirst = NULL;

	uint16_t listMap = pMdlList->listMap;
	if (listMap != 0U) {
		uint_fast8_t listIdx =
			((sizeof(listMap) * CHAR_BIT) - 1U) - lwl__portClz16(listMap);

		pFirst = (lwl_MdlListNode *)lwl_dlListPeekFirst(
			&pMdlList->pLists[listIdx]
		);
	}

	return pFirst;
}

/** ****************************************************************************
 *
 * 	\brief		Pop the first node.
 *
 * 	\param		pMdlList	Pointer to a list.
 *
 * 	\return		Pointer to the popped node. NULL if the list was empty.
 *
 ******************************************************************************/
static inline lwl_MdlListNode * lwl_mdlListPopFirst(
	lwl_MdlList * pMdlList
) {
	lwl__portAssert(pMdlList != NULL);

	lwl_MdlListNode * pNode = lwl_mdlListPeekFirst(pMdlList);
	if (pNode != NULL) {
		lwl_mdlListRemove(pMdlList, pNode);
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
inline bool lwl_mdlListIsNodeInList(
	const lwl_MdlListNode * pNode
) {
	lwl__portAssert(pNode != NULL);

	return lwl_dlListIsNodeInList(&pNode->dlListNode);
}

#endif /* LWL_MDLLIST_H_INCLUDED */
