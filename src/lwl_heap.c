
/** ****************************************************************************
 *
 * \file	lwl_heap.c
 *
 * \brief	Source file for the heap implementation.
 *
 * \details	This file implements a heap data structure using an array.
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

#include "lwl/lwl_heap.h"

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

static void lwl__heapMoveDown(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
);

static void lwl__heapSwapNodes(
	lwl_HeapNode * pElements[],
	lwl_HeapNode * pNode1,
	lwl_HeapNode * pNode2
);

/*******************************************************************************
;
;	I N L I N E   F U N C T I O N S
;
;-----------------------------------------------------------------------------*/

extern inline void lwl_heapNodeInit(
	lwl_HeapNode * pNode
);

extern inline void lwl_heapInit(
	lwl_Heap *	   pHeap,
	lwl_HeapNode * pHeapArray[],
	lwl_HeapIdx	   arraySize
);

extern inline lwl_HeapNode * lwl_heapPeekFirst(
	const lwl_Heap * pHeap
);

extern inline lwl_HeapNode * lwl_heapPopFirst(
	lwl_Heap *			pHeap,
	const lwl_HeapCmp * pCmp
);

extern inline bool lwl_heapIsNodeInHeap(
	const lwl_HeapNode * pNode
);

extern inline bool lwl_heapMoveNode(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pHeapNode,
	const lwl_HeapCmp * pCmp
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
 * \brief		Insert the supplied node into the heap.
 *
 * \param[in]	pHeap		Pointer to heap.
 * \param[in]	pNew		Pointer to node that should be inserted.
 * \param[in]	pCmp		Pointer to node comparing information.
 *
 * \retval		true		The node was inserted successfully.
 * \retval		false		Could not insert the node. It may already be in a
 * 							heap.
 *
 * \details		The node and the heap must be initialized before calling this
 * 				function.
 *
 ******************************************************************************/
bool lwl_heapInsert(
	lwl_Heap *			pHeap,
	lwl_HeapNode *		pNew,
	const lwl_HeapCmp * pCmp
) {
	bool okToInsert = !lwl_heapIsNodeInHeap(pNew);

	if (pHeap->nodeCount == pHeap->arraySize) {
		/*
		 * The heap is full. Cannot insert any more nodes.
		 */
		okToInsert = false;
	}

	if (okToInsert) {
		uint_fast16_t nodeIdx = pHeap->nodeCount;

		/*
		 * Insert the node to the next free spot in the heap.
		 */
		pHeap->pElements[nodeIdx] = pNew;
		pNew->heapIdx = (lwl_HeapIdx)nodeIdx;
		pHeap->nodeCount = (lwl_HeapIdx)(nodeIdx + 1U);

		/*
		 * Move node towards the root until the heap property is satisfied.
		 */
		lwl__heapMoveDown(pHeap, pNew, pCmp);
	}

	return okToInsert;
}

/** ****************************************************************************
 *
 * \brief		Remove a node from the heap.
 *
 * \param[in]	pHeap		Pointer to the heap that contains the node.
 * \param[in]	pNode		Pointer to the node that should be removed.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \retval		true		The node was successfully removed.
 * \retval		false		Could not remove the node. Maybe it isn't in a heap.
 *
 * \details
 *
 ******************************************************************************/
bool lwl_heapRemove(
	lwl_Heap *			pHeap,
	lwl_HeapNode *		pHeapNode,
	const lwl_HeapCmp * pCmp
) {
	lwl__portAssert(pHeap != NULL);

	if (pHeap->nodeCount == 0U) {
		return false;
	}

	bool isInHeap = lwl_heapIsNodeInHeap(pHeapNode);
	if (!isInHeap) {
		return false;
	}

	/*
	 * First remove the last node from the heap.
	 */
	lwl_HeapNode ** pElements = pHeap->pElements;
	pHeap->nodeCount--;
	lwl_HeapNode * pLastNode = pElements[pHeap->nodeCount];
	pElements[pHeap->nodeCount] = NULL;

	uint_fast16_t heapIdx = pHeapNode->heapIdx;
	pHeapNode->heapIdx = LWL__HEAP_INV_IDX;

	if (pLastNode != pHeapNode) {
		/*
		 * The node that should be removed is not the last node of the heap.
		 * Replace the node that should removed with the last node.
		 */
		pElements[heapIdx] = pLastNode;
		pLastNode->heapIdx = (lwl_HeapIdx)heapIdx;

		/*
		 * The node may now be in the wrong place. Move it up or down
		 * until the heap property is satisfied.
		 */
		lwl__heapMoveInternal(pHeap, pLastNode, pCmp);
	}

	return true;
}

/** ****************************************************************************
 *
 * \brief		Move a node down in the heap.
 *
 * \param[in]	pHeap		Pointer to the heap containing the node.
 * \param[in]	pNode		Pointer to the node that should be moved.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \details		Move a node towards the root node until the heap property is
 * 				satisfied.
 *
 ******************************************************************************/
static void lwl__heapMoveDown(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
) {
	uint_fast16_t	index = pNode->heapIdx;
	lwl_HeapNode ** pElements = pHeap->pElements;

	while (index != 0U) {
		uint_fast16_t  parentIndex = (index - 1U) >> 1U;
		lwl_HeapNode * pParent = pElements[parentIndex];

		/*
		 * Compare nodes to see if we need to swap them.
		 */
		bool doSwap = pCmp->pCmpFn(pNode, pParent, pCmp->pCmpParam);
		if (!doSwap) {
			break;
		}

		/*
		 * Swap nodes and continue down.
		 */
		lwl__heapSwapNodes(pElements, pParent, pNode);

		index = parentIndex;
	}
}

/** ****************************************************************************
 *
 * \brief		Move a node up in the heap.
 *
 * \param[in]	pHeap		Pointer to the heap containing the node.
 * \param[in]	pNode		Pointer to the node that should be moved.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \details		Move a node away from the root node until the heap property is
 * 				satisfied.
 *
 ******************************************************************************/
void lwl__heapMoveUp(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
) {
	uint_fast16_t leftIdx = 1U;
	if (pNode->heapIdx != 0U) {
		/*
		 * Get index of the left child node.
		 */
		leftIdx = (uint_fast16_t)(pNode->heapIdx << 1U) + 1U;
	}

	/*
	 * Swap the position of the node and one of its child nodes until the
	 * position is valid for a heap.
	 */
	lwl_HeapNode ** pElements = pHeap->pElements;

	while (leftIdx < pHeap->nodeCount) {
		/*
		 * The node has a left child. Check if the left child
		 * should be before the node.
		 */
		lwl_HeapNode * pFirst = pNode;
		lwl_HeapNode * pLeft = pElements[leftIdx];
		if (pCmp->pCmpFn(pLeft, pNode, pCmp->pCmpParam)) {
			pFirst = pLeft;
		}

		uint_fast16_t rightIdx = leftIdx + 1U;
		if (rightIdx < pHeap->nodeCount) {
			lwl_HeapNode * pRight = pElements[rightIdx];
			if (pCmp->pCmpFn(pRight, pFirst, pCmp->pCmpParam)) {
				/*
				 * The node has a right child that should be before the
				 * node and its left child.
				 */
				pFirst = pRight;
			}
		}

		if (pFirst == pNode) {
			/*
			 * A valid position has been found for the node.
			 */
			break;
		}

		lwl__heapSwapNodes(pElements, pNode, pFirst);

		/*
		 * Get index of the left child node.
		 */
		leftIdx = ((uint_fast16_t)(pNode->heapIdx << 1U)) + 1U;
	}
}

/** ****************************************************************************
 *
 * \brief		Move a node in a heap.
 *
 * \param[in]	pHeap		Pointer to the heap containing the node.
 * \param[in]	pNode		Pointer to the node that should be moved.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \details		This function is used internally in the heap. It should not
 * 				be called from other code.
 *
 ******************************************************************************/
void lwl__heapMoveInternal(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
) {
	/*
	 * Check if it should be moved up or down.
	 */
	uint_fast16_t nodeIdx = pNode->heapIdx;
	if (nodeIdx == 0U) {
		/*
		 * It is the root node.
		 */
		lwl__heapMoveUp(pHeap, pNode, pCmp);

	} else {
		uint_fast16_t  parentIdx = (nodeIdx - 1U) >> 1U;
		lwl_HeapNode * pParent = pHeap->pElements[parentIdx];

		if (pCmp->pCmpFn(pParent, pNode, pCmp->pCmpParam)) {
			lwl__heapMoveUp(pHeap, pNode, pCmp);
		} else {
			lwl__heapMoveDown(pHeap, pNode, pCmp);
		}
	}
}

/** ****************************************************************************
 *
 * \brief		Swap the location of two nodes in the heap.
 *
 * \param[in]	pElements	Pointer to array of heap elements.
 * \param[in]	pNode1		Pointer to node 1.
 * \param[in]	pNode2		Pointer to node 2.
 *
 ******************************************************************************/
static void lwl__heapSwapNodes(
	lwl_HeapNode * pElements[],
	lwl_HeapNode * pNode1,
	lwl_HeapNode * pNode2
) {
	lwl_HeapIdx node1Idx = pNode1->heapIdx;
	lwl_HeapIdx node2Idx = pNode2->heapIdx;
	pElements[node1Idx] = pNode2;
	pNode2->heapIdx = node1Idx;
	pElements[node2Idx] = pNode1;
	pNode1->heapIdx = node2Idx;
}
