/** ****************************************************************************
 *
 * \file	lwl_ptrHeap.h
 *
 * \brief	Header file for a heap implementation using pointers.
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

#ifndef LWL_PTRHEAP_H_INCLUDED
#define LWL_PTRHEAP_H_INCLUDED

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
 * Type used by each heap node.
 */
typedef struct lwl__ptrHeapNode lwl_PtrHeapNode;

/**
 * Type used by each heap node.
 */
struct lwl__ptrHeapNode {
	lwl_PtrHeapNode * pLeft;   /**< Pointer to left node. */
	lwl_PtrHeapNode * pRight;  /**< Pointer to right node. */
	lwl_PtrHeapNode * pParent; /**< Pointer to parent node. */
};

/**
 * Type for a pointer heap.
 */
typedef struct {
	lwl_PtrHeapNode * pRoot;	 /**< Pointer to root node. */
	uint16_t		  nodeCount; /**< Number of nodes in tree. */
} lwl_PtrHeap;

/** ****************************************************************************
 *
 * \brief	Function type used for node compare functions.
 *
 * \param	pNode1	Pointer to the first node.
 * \param	pNode2 	Pointer to the second node.
 * \param	pParam	Pointer to comparison parameter.
 *
 * \retval	true	The first node should be before the second node.
 * \retval	false	The second node should be before the first node.
 *
 * \details	Used for functions that determine the order of the tree.
 *
 ******************************************************************************/
typedef bool lwl_PtrHeapCmpFn(
	const lwl_PtrHeapNode * pNode1,
	const lwl_PtrHeapNode * pNode2,
	const void *			pParam
);

/**
 * Type used to store node comparing function and parameter.
 */
typedef struct {
	lwl_PtrHeapCmpFn * pCmpFn;	  /**< Pointer to node comparing function. */
	const void *	   pCmpParam; /**< Node comparing function parameter. */
} lwl_PtrHeapCmp;

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

void lwl__ptrHeapMoveInternal(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
);

bool lwl_ptrHeapInsert(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNew,
	const lwl_PtrHeapCmp * pCmp
);

bool lwl_ptrHeapRemove(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
);

/** ****************************************************************************
 *
 * \brief		Check if the supplied node is in a heap.
 *
 * \param[in]	pNode		The node to check.
 *
 * \retval		true		The node is in a heap.
 * \retval		false		The node is not in a heap.
 *
 * \details
 *
 ******************************************************************************/
inline bool lwl_ptrHeapIsNodeInHeap(
	const lwl_PtrHeapNode * pNode
) {
	return pNode->pParent != NULL;
}

/** ****************************************************************************
 *
 * \brief		Moves node position in heap.
 *
 * \param[in]	pHeap		Pointer to the heap.
 * \param[in]	pNode		Pointer to node that should be moved.
 * \param[in]	pCmp		Pointer to comparison information used to keep the
 * 							heap sorted in the correct order.
 *
 * \details		This function should be called if a node has been modified in a
 * 				way that may cause the comparison function to return a different
 * 				value than before.
 *
 ******************************************************************************/
inline bool lwl_ptrHeapMoveNode(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
) {
	bool isInHeap = lwl_ptrHeapIsNodeInHeap(pNode);

	if (isInHeap) {
		lwl__ptrHeapMoveInternal(pHeap, pNode, pCmp);
	}

	return isInHeap;
}

/** ****************************************************************************
 *
 * \brief		Peek at the first node in the heap.
 *
 * \param[in]	pHeap		Pointer to a heap.
 *
 * \return		Pointer to the first node.
 *
 * \details
 *
 ******************************************************************************/
inline lwl_PtrHeapNode * lwl_ptrHeapPeekFirst(
	const lwl_PtrHeap * pHeap
) {
	return pHeap->pRoot;
}

/** ****************************************************************************
 *
 * \brief		Pop node with highest key.
 *
 * \param[in]	pHeap 		Pointer to a heap.
 * \param[in]	pCmp		Pointer to node comparison information.
 *
 * \return		Pointer to the popped node. NULL if the tree was empty.
 *
 * \details
 *
 ******************************************************************************/
inline lwl_PtrHeapNode * lwl_ptrHeapPopFirst(
	lwl_PtrHeap *		   pHeap,
	const lwl_PtrHeapCmp * pCmp
) {
	lwl_PtrHeapNode * pNode = lwl_ptrHeapPeekFirst(pHeap);
	if (pNode != NULL) {
		bool wasRemoved = lwl_ptrHeapRemove(pHeap, pNode, pCmp);
		if (!wasRemoved) {
			pNode = NULL;
		}
	}

	return pNode;
}

/** ****************************************************************************
 *
 * \brief		Initialize heap node.
 *
 * \param[in]	pNode	Pointer to the node.
 *
 * \details
 *
 ******************************************************************************/
inline void lwl_ptrHeapNodeInit(
	lwl_PtrHeapNode * pNode
) {
	pNode->pLeft = NULL;
	pNode->pRight = NULL;
	pNode->pParent = NULL;
}

/** ****************************************************************************
 *
 * \brief		Initialize heap.
 *
 * \param[in]	pHeap	Pointer to the heap.
 *
 * \details
 *
 ******************************************************************************/
inline void lwl_ptrHeapInit(
	lwl_PtrHeap * pHeap
) {
	pHeap->pRoot = NULL;
	pHeap->nodeCount = 0U;
}

/** ****************************************************************************
 *
 * \brief		Store a node pointer to the root of the heap.
 *
 * \param[in]	pHeap		Pointer to a heap.
 * \param[in]	pNode		Pointer to node that should be set as root node.
 *
 * \details
 *
 * \note		May only be called for an empty heap. NULL must be stored before
 * 				inserting nodes to the heap.
 *
 ******************************************************************************/
inline void lwl_ptrHeapSetRootPtr(
	lwl_PtrHeap *	  pHeap,
	lwl_PtrHeapNode * pNode
) {
	pHeap->pRoot = pNode;
}

#endif /* LWL_PTRHEAP_H_INCLUDED */
