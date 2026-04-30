/** ****************************************************************************
 *
 * \file	lwl_heap.h
 *
 * \brief	Header file for a heap implementation.
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

#ifndef LWL_HEAP_H_INCLUDED
#define LWL_HEAP_H_INCLUDED

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

/**
 * Invalid index. Stored to nodes that are not in the heap.
 */
#define LWL__HEAP_INV_IDX 0xFFFFU

/*******************************************************************************
;
;	T Y P E S
;
;-----------------------------------------------------------------------------*/

/**
 * Type used to store the index of heap nodes.
 */
typedef uint16_t lwl_HeapIdx;

/**
 * Type used by each heap node.
 */
typedef struct {
	lwl_HeapIdx heapIdx;
} lwl_HeapNode;

/**
 * Type for heap.
 */
typedef struct {
	lwl_HeapNode ** pElements; /**< Pointer to the heap array. */
	lwl_HeapIdx		nodeCount; /**< Number of nodes in the heap. */
	lwl_HeapIdx		arraySize; /**< Number of elements in the array. */
} lwl_Heap;

/** ****************************************************************************
 *
 * \brief		Function type used for node compare functions.
 *
 * \param[in]	pNode1	Pointer to the first node.
 * \param[in]	pNode2 	Pointer to the second node.
 * \param[in]	pParam	Pointer to comparison parameter.
 *
 * \retval		true	The first node should be before the second node.
 * \retval		false	The second node should be before the first node.
 *
 * \details		Used for functions that determine the order of the heap.
 *
 ******************************************************************************/
typedef bool lwl_HeapCmpFn(
	const lwl_HeapNode * pNode1,
	const lwl_HeapNode * pNode2,
	const void *		 pParam
);

/**
 * Type used to store node comparing function and parameter.
 */
typedef struct {
	lwl_HeapCmpFn * pCmpFn;	   /**< Pointer to node comparing function. */
	const void *	pCmpParam; /**< Node comparing function parameter. */
} lwl_HeapCmp;

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

void lwl__heapMoveInternal(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
);

bool lwl_heapInsert(
	lwl_Heap *			pHeap,
	lwl_HeapNode *		pNew,
	const lwl_HeapCmp * pCmp
);

bool lwl_heapRemove(
	lwl_Heap *			pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
);

void lwl__heapMoveUp(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pNode,
	const lwl_HeapCmp * pCmp
);

/** ****************************************************************************
 *
 * \brief		Initialize a heap node.
 *
 * \param[in]	pNode		Pointer to the node.
 *
 * \details		Nodes must be initialized with this function before they are
 * 				inserted into a heap.
 *
 ******************************************************************************/
inline void lwl_heapNodeInit(
	lwl_HeapNode * pNode
) {
	pNode->heapIdx = LWL__HEAP_INV_IDX;
}

/** ****************************************************************************
 *
 * \brief		Initialize a heap.
 *
 * \param[in]	pHeap			Pointer to the heap.
 * \param[in]	pHeapArray		Pointer to array with pointers to nodes.
 * \param[in]	arraySize		Number of elements in the array.
 *
 * \details		A heap must be initialized with this function before nodes
 * 				can be inserted.
 *
 ******************************************************************************/
inline void lwl_heapInit(
	lwl_Heap *	   pHeap,
	lwl_HeapNode * pHeapArray[],
	lwl_HeapIdx	   arraySize
) {
	pHeap->nodeCount = 0U;
	pHeap->pElements = pHeapArray;
	pHeap->arraySize = arraySize;
}

/** ****************************************************************************
 *
 * \brief		Check if the supplied node is in a heap.
 *
 * \param[in]	pNode		Pointer to the node to check.
 *
 * \retval		true		The node is in a heap.
 * \retval		false		The node is not in a heap.
 *
 ******************************************************************************/
inline bool lwl_heapIsNodeInHeap(
	const lwl_HeapNode * pNode
) {
	return pNode->heapIdx != LWL__HEAP_INV_IDX;
}

/** ****************************************************************************
 *
 * \brief		Move a node in the heap.
 *
 * \param[in]	pHeap		Pointer to the heap containing the node.
 * \param[in]	pHeapNode	Pointer to the node that should be moved.
 * \param[in]	pCmp		Pointer to the node comparing information.
 *
 * \retval		true		The node was successfully moved.
 * \retval		false		Could not move the node. Maybe it isn't in a heap.
 *
 * \details		This function should be called if the node has changed in a way
 * 				that modifies the return value of the compare function.
 *
 ******************************************************************************/
inline bool lwl_heapMoveNode(
	const lwl_Heap *	pHeap,
	lwl_HeapNode *		pHeapNode,
	const lwl_HeapCmp * pCmp
) {
	bool okToMove = lwl_heapIsNodeInHeap(pHeapNode);

	if (pHeap->nodeCount == 0U) {
		okToMove = false;
	}

	if (okToMove) {
		lwl__heapMoveInternal(pHeap, pHeapNode, pCmp);
	}

	return okToMove;
}

/** ****************************************************************************
 *
 * \brief		Peek at the first node.
 *
 * \param[in]	pHeap		Pointer to the heap.
 *
 * \return		Pointer to the first node.
 *
 * \details		The first node is the root node. The order of the heap depends
 * 				on the comparing information supplied when inserting nodes.
 *
 ******************************************************************************/
inline lwl_HeapNode * lwl_heapPeekFirst(
	const lwl_Heap * pHeap
) {
	lwl__portAssert(pHeap != NULL);

	return pHeap->pElements[0];
}

/** ****************************************************************************
 *
 * \brief		Pop the first node.
 *
 * \param[in]	pHeap 		Pointer to the heap.
 * \param[in]	pCmp		Pointer to node comparison information.
 *
 * \return		Pointer to the popped node. NULL if the heap was empty.
 *
 * \details		This will pop the root node from the heap.
 *
 ******************************************************************************/
inline lwl_HeapNode * lwl_heapPopFirst(
	lwl_Heap *			pHeap,
	const lwl_HeapCmp * pCmp
) {
	lwl__portAssert(pHeap != NULL);

	if (pHeap->nodeCount == 0U) {
		return NULL;
	}

	/*
	 * First remove the last node from the heap.
	 */
	lwl_HeapNode ** pElements = pHeap->pElements;
	pHeap->nodeCount--;
	lwl_HeapNode * pLastNode = pElements[pHeap->nodeCount];

	lwl_HeapNode * pRoot = pElements[0];
	pRoot->heapIdx = LWL__HEAP_INV_IDX;

	if (pRoot != pLastNode) {
		/*
		 * The root node was not the last node of the heap.
		 * Replace the root node with the last node.
		 */
		pElements[0] = pLastNode;
		pLastNode->heapIdx = 0;

		/*
		 * The node may now be in the wrong place. Move it up until the heap
		 * property is satisfied.
		 */
		lwl__heapMoveUp(pHeap, pLastNode, pCmp);
	}

	return pRoot;
}

/** ****************************************************************************
 *
 * \brief		Set the root node.
 *
 * \param[in]	pHeap		Pointer to a heap.
 * \param[in]	pNode		Pointer to node that should be set as root node.
 *
 * \details		This function can be used to quickly store a node pointer
 * 				directly to the root of the heap. This is faster than
 * 				lwl_heapInsert().
 *
 * 				The purpose of this function is to temporarily store a node to
 * 				the root of the heap while the heap is empty. A NULL pointer
 * 				must be stored before nodes are inserted into the heap again.
 *
 ******************************************************************************/
inline void lwl_heapSetRootNode(
	lwl_Heap *	   pHeap,
	lwl_HeapNode * pNode
) {
	pHeap->pElements[0] = pNode;
}

#endif /* LWL_HEAP_H_INCLUDED */
