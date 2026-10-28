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

static void lwl__heapMoveTowardsRoot(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	uint_fast16_t		nodeIdx,
	const lwl_HeapCmp * pCmp
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

extern inline bool lwl_heapIsNodeInHeap(
	const lwl_Heap *	 pHeap,
	const lwl_HeapNode * pNode
);

extern inline bool lwl_heapMoveNode(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pHeapNode,
	const lwl_HeapCmp * pCmp
);

extern inline lwl_HeapNode * lwl_heapPeekFirst(
	const lwl_Heap * pHeap
);

extern inline lwl_HeapNode * lwl__heapRemoveLastNode(
	lwl_Heap * pHeap
);

extern inline lwl_HeapNode * lwl_heapPopFirst(
	lwl_Heap *			pHeap,
	const lwl_HeapCmp * pCmp
);

extern inline void lwl_heapStorePtr(
	const lwl_Heap * pHeap,
	lwl_HeapNode *	 pNode
);

extern inline lwl_HeapNode * lwl_heapGetStoredPtr(
	const lwl_Heap * pHeap
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
 * \param		pHeap		Pointer to heap.
 * \param		pNew		Pointer to node that should be inserted.
 * \param[in]	pCmp		Pointer to node comparing information.
 *
 * \retval		true		The node was inserted successfully.
 * \retval		false		Could not insert the node. It may already be in a
 * 							heap or the heap may be full.
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
	/* A node in any heap, not only in this one, must not be inserted */
	bool okToInsert = (pNew->heapIdx == LWL__HEAP_INV_IDX);

	if (pHeap->nodeCount == pHeap->arraySize) {
		/*
		 * The heap is full. Cannot insert any more nodes.
		 */
		okToInsert = false;
	}

	if (okToInsert) {
		uint_fast16_t nodeIdx = pHeap->nodeCount;
		pHeap->nodeCount = (lwl_HeapIdx)(nodeIdx + 1U);

		/*
		 * Move node towards the root until the heap property is satisfied.
		 */
		lwl__heapMoveTowardsRoot(pHeap, pNew, nodeIdx, pCmp);
	}

	return okToInsert;
}

/** ****************************************************************************
 *
 * \brief		Remove a node from the heap.
 *
 * \param		pHeap		Pointer to the heap that contains the node.
 * \param		pNode		Pointer to the node that should be removed.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \retval		true		The node was successfully removed.
 * \retval		false		Could not remove the node. The heap is empty or
 * 							it doesn't contain the supplied node.
 *
 * \details
 *
 ******************************************************************************/
bool lwl_heapRemove(
	lwl_Heap *			pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
) {
	lwl__portAssert(pHeap != NULL);

	if (pHeap->nodeCount == 0U) {
		return false;
	}

	bool isInHeap = lwl_heapIsNodeInHeap(pHeap, pNode);
	if (!isInHeap) {
		return false;
	}

	lwl_HeapNode * pLastNode = lwl__heapRemoveLastNode(pHeap);
	uint_fast16_t  heapIdx = pNode->heapIdx;
	pNode->heapIdx = LWL__HEAP_INV_IDX;

	if (pLastNode != pNode) {
		/*
		 * The node may now be in the wrong place. Move it towards the leaves or
		 * root until the heap property is satisfied.
		 */
		lwl__heapMoveInternal(pHeap, pLastNode, heapIdx, pCmp);
	}

	return true;
}

/** ****************************************************************************
 *
 * \brief		Move a node towards the root in the heap.
 *
 * \param		pHeap		Pointer to the heap containing the node.
 * \param		pNode		Pointer to the node that should be moved.
 * \param		nodeIdx		Starting index for the move operation.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \details		Move a node towards the root node until the heap property is
 * 				satisfied.
 *
 ******************************************************************************/
static void lwl__heapMoveTowardsRoot(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	uint_fast16_t		nodeIdx,
	const lwl_HeapCmp * pCmp
) {
	/*
	 * Copying variables to local variables to avoid loads inside the loop.
	 */
	lwl_HeapCmpFn * pCmpFn = pCmp->pCmpFn;
	const void *	pCmpParam = pCmp->pCmpParam;
	lwl_HeapNode ** pElements = pHeap->pElements;

	while (nodeIdx != 0U) {
		uint_fast16_t  parentIndex = (uint_fast16_t)((nodeIdx - 1U) >> 1U);
		lwl_HeapNode * pParent = pElements[parentIndex];

		/*
		 * Compare nodes to see if we need to move further.
		 */
		bool moveTowardsRoot = pCmpFn(pNode, pParent, pCmpParam);
		if (!moveTowardsRoot) {
			break;
		}

		/*
		 * Move parent and continue towards root.
		 */
		pElements[nodeIdx] = pParent;
		pParent->heapIdx = (lwl_HeapIdx)nodeIdx;

		nodeIdx = parentIndex;
	}

	pElements[nodeIdx] = pNode;
	pNode->heapIdx = (lwl_HeapIdx)nodeIdx;
}

/** ****************************************************************************
 *
 * \brief		Move a node towards the leaves in the heap.
 *
 * \param		pHeap		Pointer to the heap containing the node.
 * \param		pNode		Pointer to the node that should be moved.
 * \param		nodeIdx		Starting index for the move operation.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \details		Move a node away from the root node until the heap property is
 * 				satisfied.
 *
 ******************************************************************************/
void lwl__heapMoveTowardsLeaves(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	uint_fast16_t		nodeIdx,
	const lwl_HeapCmp * pCmp
) {
	/*
	 * Copying variables to local variables to avoid loads inside the loop.
	 */
	uint_fast16_t	nodeCount = pHeap->nodeCount;
	lwl_HeapCmpFn * pCmpFn = pCmp->pCmpFn;
	const void *	pCmpParam = pCmp->pCmpParam;
	lwl_HeapNode ** pElements = pHeap->pElements;

	/*
	 * Nodes from this index on have no children. Checking against this instead
	 * of calculating the child index first avoids an overflow when
	 * uint_fast16_t is 16 bits and the heap has more than 32767 nodes.
	 */
	uint_fast16_t firstLeafIdx = (uint_fast16_t)(nodeCount >> 1U);

	/*
	 * Search for a suitable position for the node by moving it towards the
	 * leaves.
	 */
	while (nodeIdx < firstLeafIdx) {
		uint_fast16_t childIdx =
			(uint_fast16_t)(((unsigned int)nodeIdx << 1U) + 1U);

		lwl_HeapNode * pFirst = pElements[childIdx];

		uint_fast16_t rightIdx = (uint_fast16_t)(childIdx + 1U);
		if (rightIdx < nodeCount) {
			/*
			 * There is a left and right child. Check which one should be first
			 * in the heap.
			 */
			lwl_HeapNode * pRight = pElements[rightIdx];
			if (pCmpFn(pRight, pFirst, pCmpParam)) {
				/*
				 * The right child should be before the left child.
				 */
				pFirst = pRight;
				childIdx = rightIdx;
			}
		}

		if (!pCmpFn(pFirst, pNode, pCmpParam)) {
			/*
			 * A valid position has been found for the node.
			 */
			break;
		}

		pElements[nodeIdx] = pFirst;
		pFirst->heapIdx = (lwl_HeapIdx)nodeIdx;

		/*
		 * Move to the child and continue searching for a valid position.
		 */
		nodeIdx = childIdx;
	}

	pElements[nodeIdx] = pNode;
	pNode->heapIdx = (lwl_HeapIdx)nodeIdx;
}

/** ****************************************************************************
 *
 * \brief		Fill an empty root position and insert a node into the heap.
 *
 * \param[in]	pHeap		Pointer to the heap. Its root position is empty.
 * \param		pNode		Pointer to the node that should be inserted. It is
 * 							usually the node that was the last node.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \details		This is the bottom-up heap sift (Floyd). The empty position is
 * 				first moved all the way to a leaf by moving up the child that
 * 				should be first. This needs only one compare per level, while
 * 				lwl__heapMoveTowardsLeaves needs two. The node is then moved
 * 				towards the root from the leaf. The last node usually belongs
 * 				near the leaves, so it rarely moves far.
 *
 ******************************************************************************/
void lwl__heapMoveToEmptyRoot(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
) {
	/*
	 * Copying variables to local variables to avoid loads inside the loop.
	 */
	uint_fast16_t	nodeCount = pHeap->nodeCount;
	lwl_HeapCmpFn * pCmpFn = pCmp->pCmpFn;
	const void *	pCmpParam = pCmp->pCmpParam;
	lwl_HeapNode ** pElements = pHeap->pElements;

	/*
	 * Nodes from this index on have no children. See
	 * lwl__heapMoveTowardsLeaves.
	 */
	uint_fast16_t firstLeafIdx = (uint_fast16_t)(nodeCount >> 1U);
	uint_fast16_t emptyIdx = 0U;

	/*
	 * Move the empty position to a leaf.
	 */
	while (emptyIdx < firstLeafIdx) {
		uint_fast16_t childIdx =
			(uint_fast16_t)(((unsigned int)emptyIdx << 1U) + 1U);

		lwl_HeapNode * pFirst = pElements[childIdx];

		uint_fast16_t rightIdx = (uint_fast16_t)(childIdx + 1U);
		if (rightIdx < nodeCount) {
			/*
			 * There is a left and right child. Check which one should be first
			 * in the heap.
			 */
			lwl_HeapNode * pRight = pElements[rightIdx];
			if (pCmpFn(pRight, pFirst, pCmpParam)) {
				pFirst = pRight;
				childIdx = rightIdx;
			}
		}

		pElements[emptyIdx] = pFirst;
		pFirst->heapIdx = (lwl_HeapIdx)emptyIdx;

		emptyIdx = childIdx;
	}

	/*
	 * Put the node into the empty leaf position and move it towards the root
	 * until the heap property is satisfied.
	 */
	lwl__heapMoveTowardsRoot(pHeap, pNode, emptyIdx, pCmp);
}

/** ****************************************************************************
 *
 * \brief		Move a node in a heap.
 *
 * \param[in]	pHeap		Pointer to the heap containing the node.
 * \param		pNode		Pointer to the node that should be moved.
 * \param		nodeIdx		Starting index for the move operation.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \details		This function is used internally in the heap. It should not
 * 				be called from other code.
 *
 ******************************************************************************/
void lwl__heapMoveInternal(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	uint_fast16_t		nodeIdx,
	const lwl_HeapCmp * pCmp
) {
	/*
	 * Check if it should be moved up or down.
	 */
	if (nodeIdx == 0U) {
		/*
		 * It is the root node.
		 */
		lwl__heapMoveTowardsLeaves(pHeap, pNode, nodeIdx, pCmp);

	} else {
		uint_fast16_t  parentIdx = (uint_fast16_t)((nodeIdx - 1U) >> 1U);
		lwl_HeapNode * pParent = pHeap->pElements[parentIdx];

		if (pCmp->pCmpFn(pParent, pNode, pCmp->pCmpParam)) {
			lwl__heapMoveTowardsLeaves(pHeap, pNode, nodeIdx, pCmp);
		} else {
			lwl__heapMoveTowardsRoot(pHeap, pNode, nodeIdx, pCmp);
		}
	}
}
