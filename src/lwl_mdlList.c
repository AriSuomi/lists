/** ****************************************************************************
 *
 * \file	lwl_mdlList.c
 *
 * \brief	Source file for the multiple doubly linked lists implementation.
 *
 * \details	This file contains a priority queue implementation using multiple
 *			doubly linked lists.
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

#include <lwl/lwl_mdlList.h>
#include <lwl/lwl_dlList.h>

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

extern inline void lwl_mdlListNodeInit(
	lwl_MdlListNode * pNode
);

extern inline void lwl_mdlListInit(
	lwl_MdlList * pMdlList,
	lwl_DlList *  pDlLists,
	uint8_t		  listCount
);

extern inline bool lwl_mdlListInsert(
	lwl_MdlList *	  pMdlList,
	lwl_MdlListNode * pNewNode,
	uint8_t			  listIdx
);

extern inline bool lwl_mdlListRemove(
	lwl_MdlList *	  pMdlList,
	lwl_MdlListNode * pNode
);

extern inline lwl_MdlListNode * lwl_mdlListPopFirst(
	lwl_MdlList * pMdlList
);

extern inline void lwl_mdlListStorePtr(
	lwl_MdlList *	  pMdlList,
	lwl_MdlListNode * pNew
);

extern inline lwl_MdlListNode * lwl_mdlListGetStoredPtr(
	const lwl_MdlList * pMdlList
);

extern inline bool lwl_mdlListIsNodeInList(
	const lwl_MdlListNode * pNode
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
 * 	\brief		Peek at the first node.
 *
 * 	\param[in]	pMdlList	Pointer to a list.
 *
 * 	\return		Pointer to the first node.
 *
 ******************************************************************************/
lwl_MdlListNode * lwl_mdlListPeekFirst(
	const lwl_MdlList * pMdlList
) {
	lwl__portAssert(pMdlList != NULL);

	lwl_MdlListNode * pFirst = NULL;

	uint16_t listMap = pMdlList->listMap;
	if (listMap != 0) {
		uint_fast8_t listIdx =
			(sizeof(listMap) * LWL__MDLLIST_BITSPERBYTE - 1U) -
			lwl__portClz16(listMap);

		pFirst = (lwl_MdlListNode *)lwl_dlListPeekFirst(
			&pMdlList->pLists[listIdx]
		);
	}

	return pFirst;
}
