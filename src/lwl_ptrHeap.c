/** ****************************************************************************
 *
 * \file	lwl_ptrHeap.c
 *
 * \brief	Source file for the heap implementation using pointers.
 *
 * \details	This heap implementation uses nodes linked with pointers instead
 *			of an array. It is slower than the regular array heap but it does
 *			not have a static size.
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

#include "lwl/lwl_port.h"
#include "lwl/lwl_ptrHeap.h"

/*******************************************************************************
;
;	D E F I N E S
;
;-----------------------------------------------------------------------------*/

/**
 * 16 bit unsigned integer with only the MSB bit set.
 */
#define LWL__PTRHEAP_U16_MSB 0x8000U

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

static void lwl__ptrHeapGraftNode(
	lwl_PtrHeap *			pHeap,
	const lwl_PtrHeapNode * pOldBranch,
	lwl_PtrHeapNode *		pScion
);

static void lwl__ptrHeapSwapAdjacentNodes(
	lwl_PtrHeap *	  pHeap,
	lwl_PtrHeapNode * pParent,
	lwl_PtrHeapNode * pChild
);

static void lwl__ptrHeapReplaceNode(
	lwl_PtrHeap *			pHeap,
	const lwl_PtrHeapNode * pNode,
	lwl_PtrHeapNode *		pReplacement
);

static inline lwl_PtrHeapNode ** lwl__ptrHeapGetLastSlot(
	lwl_PtrHeap *	   pHeap,
	lwl_PtrHeapNode ** ppParent
);

static void lwl__ptrHeapMoveTowardsLeaves(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
);

static void lwl__ptrHeapMoveTowardsRoot(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
);

/*******************************************************************************
;
;	I N L I N E   F U N C T I O N S
;
;-----------------------------------------------------------------------------*/

extern inline lwl_PtrHeapNode * lwl_ptrHeapPopFirst(
	lwl_PtrHeap *		   pHeap,
	const lwl_PtrHeapCmp * pCmp
);

extern inline lwl_PtrHeapNode * lwl_ptrHeapPeekFirst(
	const lwl_PtrHeap * pHeap
);

extern inline void lwl_ptrHeapNodeInit(
	lwl_PtrHeapNode * pNode
);

extern inline void lwl_ptrHeapInit(
	lwl_PtrHeap * pHeap
);

extern inline void lwl_ptrHeapSetRootPtr(
	lwl_PtrHeap *	  pHeap,
	lwl_PtrHeapNode * pNode
);

extern inline bool lwl_ptrHeapIsNodeInHeap(
	const lwl_PtrHeapNode * pNode
);

extern inline bool lwl_ptrHeapMoveNode(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
);

/*******************************************************************************
;
;	D A T A
;
;-----------------------------------------------------------------------------*/

/**
 * Sentinel parent node for the root node. This is a dummy node which makes it
 * possible to determine if a node is in a heap or not even when it is the
 * only node in the heap.
 */
static lwl_PtrHeapNode lwl__ptrHeapRootParent;

/*******************************************************************************
;
;	F U N C T I O N   D E F I N I T I O N S
;
;-----------------------------------------------------------------------------*/

/** ****************************************************************************
 *
 * \brief		Insert the supplied node to the heap.
 *
 * \param		pHeap		Pointer to heap.
 * \param		pNewNode	Pointer to node that should be inserted.
 * \param[in]	pCmp		Pointer to node compare information.
 *
 * \retval		true		The node was inserted successfully.
 * \retval		false		Could not insert the node. It may already be in a
 *							heap or the heap may be full.
 *
 ******************************************************************************/
bool lwl_ptrHeapInsert(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNewNode,
	const lwl_PtrHeapCmp * pCmp
) {
	bool wasInserted = false;
	bool isInHeap = lwl_ptrHeapIsNodeInHeap(pNewNode);

	if (!isInHeap && pHeap->nodeCount < 0xFFFFU) {
		/*
		 * The node is not yet in a heap. Continue with the insert.
		 */
		pHeap->nodeCount++;

		/*
		 * Find the next free spot in the tree. The node count has already
		 * been incremented so the lwl__ptrHeapGetLastSlot function will
		 * return the next free spot in the heap. In an empty heap it is the
		 * root pointer.
		 */
		lwl_PtrHeapNode *  pParent;
		lwl_PtrHeapNode ** ppSlot = lwl__ptrHeapGetLastSlot(pHeap, &pParent);

		/*
		 * Insert the new node to the next free spot.
		 */
		*ppSlot = pNewNode;
		pNewNode->pParent = pParent;

		/*
		 * Move the inserted node to the correct position.
		 */
		lwl__ptrHeapMoveTowardsRoot(pHeap, pNewNode, pCmp);

		wasInserted = true;
	}

	return wasInserted;
}

/** ****************************************************************************
 *
 * \brief		Remove a node from the heap.
 *
 * \param		pHeap		Pointer to the heap.
 * \param		pNode		Pointer to node that should be removed from the
 * 							heap.
 * \param[in]	pCmp		Pointer to node comparing information.
 *
 * \retval		true		The node was successfully removed.
 * \retval		false		Could not remove the node. The supplied node is not
 * 							in any heap or the supplied heap is empty.
 *
 * \note		The supplied node must be in the supplied heap. The heap will
 * 				be corrupted if the node is in another heap.
 *
 ******************************************************************************/
bool lwl_ptrHeapRemove(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
) {
	bool wasRemoved = false;

	bool isInHeap = lwl_ptrHeapIsNodeInHeap(pNode);

	if (pHeap->nodeCount == 0U) {
		/* The node cannot be in this heap. */
		isInHeap = false;
	}

	if (isInHeap) {
		/*
		 * Cut the last node from the heap by clearing the pointer to it.
		 */
		lwl_PtrHeapNode *  pLastParent;
		lwl_PtrHeapNode ** ppLastSlot = lwl__ptrHeapGetLastSlot(
			pHeap,
			&pLastParent
		);
		lwl_PtrHeapNode * pLastNode = *ppLastSlot;
		*ppLastSlot = NULL;

		if (pLastNode != pNode) {
			/*
			 * The last node is not the node that should be removed. Replace the
			 * node that should be removed with the last node.
			 */
			lwl__ptrHeapReplaceNode(pHeap, pNode, pLastNode);

			/*
			 * The node has now been removed from the heap but the replacement
			 * node might violate the heap property. Update the node position.
			 */
			lwl__ptrHeapMoveInternal(pHeap, pLastNode, pCmp);
		}

		/*
		 * Decrease the heap node counter to keep track of how many nodes the
		 * heap contains.
		 */
		pHeap->nodeCount--;

		/*
		 * Clean up all pointers of the removed node by reinitializing it.
		 */
		lwl_ptrHeapNodeInit(pNode);

		wasRemoved = true;
	}

	return wasRemoved;
}

/** ****************************************************************************
 *
 * \brief		Get the pointer that points to the last node of the heap.
 *
 * \param[in]	pHeap		Pointer to the heap.
 * \param[out]	ppParent	The parent of the last node is stored here. It is
 * 							the root parent sentinel if the last node is the
 * 							root.
 *
 * \return		Pointer to the root pointer or to the child pointer of the
 * 				parent node that points to the last node.
 *
 * \details		The node count decides which node is the last node. The
 * 				returned pointer does not have to point to a node yet, so
 * 				incrementing the node count first gives the next free spot.
 * 				It is inline because GCC does not inline it into its two
 * 				callers otherwise, which costs a call on every insert and remove.
 *
 ******************************************************************************/
static inline lwl_PtrHeapNode ** lwl__ptrHeapGetLastSlot(
	lwl_PtrHeap *	   pHeap,
	lwl_PtrHeapNode ** ppParent
) {
	uint16_t nodeCount = pHeap->nodeCount;
	lwl__portAssert(nodeCount != 0U);

	/*
	 * First determine the path to the last node. The node count can be used
	 * as a path to the last node. Each bit from most significant to the least
	 * significant tells which direction to take when traversing the tree.
	 * The most significant set bit is the root, so the number of bits after
	 * it is the depth of the last node.
	 *
	 * The node count bits are left aligned here to make it easier to start
	 * checking bits from the most significant.
	 */
	uint8_t		  leadingZeroes = lwl__portClz16(nodeCount);
	uint_fast8_t  depth = (uint_fast8_t)(15U - leadingZeroes);
	uint_fast16_t path = (uint_fast16_t)(nodeCount << leadingZeroes);

	lwl_PtrHeapNode ** ppSlot = &pHeap->pRoot;
	lwl_PtrHeapNode *  pParent = &lwl__ptrHeapRootParent;

	if (depth > 0U) {
		/*
		 * Traverse the path to the parent of the last node. The loop count is
		 * known in advance so the child pointer of the last node is never
		 * read. Each step loads a child node instead of selecting a slot
		 * pointer. This lets compilers use a branch, which is faster than a
		 * conditional move when the path is predictable.
		 */
		pParent = pHeap->pRoot;

		while (depth > 1U) {
			path <<= 1U;
			if ((path & LWL__PTRHEAP_U16_MSB) == 0U) {
				pParent = pParent->pLeft;
			} else {
				pParent = pParent->pRight;
			}
			depth--;
		}

		path <<= 1U;
		if ((path & LWL__PTRHEAP_U16_MSB) == 0U) {
			ppSlot = &pParent->pLeft;
		} else {
			ppSlot = &pParent->pRight;
		}
	}

	*ppParent = pParent;

	return ppSlot;
}

/** ****************************************************************************
 *
 * \brief		Graft a scion replacing an entire branch of the heap.
 *
 * \param		pHeap		Pointer to the heap.
 * \param[in]	pOldBranch	Pointer to the branch that should be replaced.
 * \param[in]	pScion		Pointer to the replacement branch. May be NULL if
 * 							the old branch should just be removed from the heap.
 *
 * \details		The scion will replace the old branch of the heap. The scion may
 * 				also have child nodes.
 *
 * \note		This function does not update the parent pointer of the scion!
 * 				That must be done separately.
 *
 ******************************************************************************/
static void lwl__ptrHeapGraftNode(
	lwl_PtrHeap *			pHeap,
	const lwl_PtrHeapNode * pOldBranch,
	lwl_PtrHeapNode *		pScion
) {
	lwl_PtrHeapNode * pRootstock = pOldBranch->pParent;

	if (pRootstock == &lwl__ptrHeapRootParent) {
		pHeap->pRoot = pScion;

	} else {
		if (pRootstock->pLeft == pOldBranch) {
			pRootstock->pLeft = pScion;

		} else {
			pRootstock->pRight = pScion;
		}
	}
}

/** ****************************************************************************
 *
 * \brief		Replace a node with another node.
 *
 * \param		pHeap			Pointer to the heap.
 * \param[in]	pNode			Pointer to the node that should be replaced.
 * \param		pReplacement	Pointer to the replacing node.
 *
 * \details		This function replaces just one node in the heap. The
 * 				replacement node must be a lone node with no children.
 *
 ******************************************************************************/
static void lwl__ptrHeapReplaceNode(
	lwl_PtrHeap *			pHeap,
	const lwl_PtrHeapNode * pNode,
	lwl_PtrHeapNode *		pReplacement
) {
	lwl__ptrHeapGraftNode(pHeap, pNode, pReplacement);

	pReplacement->pParent = pNode->pParent;
	pReplacement->pRight = pNode->pRight;
	pReplacement->pLeft = pNode->pLeft;

	if (pNode->pLeft != NULL) {
		pNode->pLeft->pParent = pReplacement;

		if (pNode->pRight != NULL) {
			pNode->pRight->pParent = pReplacement;
		}
	}
}

/** ****************************************************************************
 *
 * \brief	Swap the place of a child and a parent node.
 *
 * \param	pHeap			Pointer to the heap.
 * \param	pParent			Pointer to the parent node.
 * \param	pChild			Pointer to the child node.
 *
 ******************************************************************************/
static void lwl__ptrHeapSwapAdjacentNodes(
	lwl_PtrHeap *	  pHeap,
	lwl_PtrHeapNode * pParent,
	lwl_PtrHeapNode * pChild
) {
	/*
	 * Update the pointers between the new parent node and the parent of that
	 * node.
	 */
	lwl__ptrHeapGraftNode(pHeap, pParent, pChild);
	pChild->pParent = pParent->pParent;
	pParent->pParent = pChild;

	/*
	 * Store the left and right pointers of the parent before overwriting them.
	 */
	lwl_PtrHeapNode * pParentLeft = pParent->pLeft;
	lwl_PtrHeapNode * pParentRight = pParent->pRight;

	/*
	 * Update links to and from the child nodes of the new child node.
	 */
	pParent->pRight = NULL;
	pParent->pLeft = pChild->pLeft;
	if (pParent->pLeft != NULL) {
		pParent->pLeft->pParent = pParent;

		/*
		 * Update links to and from the right child of the new child node.
		 * There is never a right child node without a left child node.
		 */
		pParent->pRight = pChild->pRight;
		if (pParent->pRight != NULL) {
			pParent->pRight->pParent = pParent;
		}
	}

	/*
	 * Update the pointer to and from child nodes of the new child node.
	 */
	if (pParentLeft == pChild) {
		/*
		 * The child node was the left child of the parent.
		 */
		pChild->pLeft = pParent;
		pChild->pRight = pParentRight;
		if (pParentRight != NULL) {
			pParentRight->pParent = pChild;
		}

	} else {
		/*
		 * The child node was the right child of the parent. The parent must
		 * have had a left child also because it had a right child.
		 */
		pChild->pRight = pParent;
		pChild->pLeft = pParentLeft;
		pParentLeft->pParent = pChild;
	}
}

/** ****************************************************************************
 *
 * \brief	Move a node towards the root until a valid position is found.
 *
 * \param		pHeap		Pointer to the heap.
 * \param		pNode		Pointer to node that should be moved.
 * \param[in]	pCmp		Pointer to comparison information.
 *
 ******************************************************************************/
static void lwl__ptrHeapMoveTowardsRoot(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
) {
	lwl_PtrHeapCmpFn * pCmpFn = pCmp->pCmpFn;
	const void *	   pCmpParam = pCmp->pCmpParam;

	/*
	 * Swap the position of the node and an adjacent node until we get a valid
	 * heap.
	 */
	while (
		pNode->pParent != &lwl__ptrHeapRootParent &&
		pCmpFn(pNode, pNode->pParent, pCmpParam)) {
		lwl__ptrHeapSwapAdjacentNodes(pHeap, pNode->pParent, pNode);
	}
}

/** ****************************************************************************
 *
 * \brief	Move a node towards the leaves until a valid position is found.
 *
 * \param		pHeap		Pointer to the heap.
 * \param		pNode		Pointer to node that should be moved.
 * \param[in]	pCmp		Pointer to comparison information.
 *
 ******************************************************************************/
static void lwl__ptrHeapMoveTowardsLeaves(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
) {
	lwl_PtrHeapCmpFn * pCmpFn = pCmp->pCmpFn;
	const void *	   pCmpParam = pCmp->pCmpParam;
	lwl_PtrHeapNode *  pLeft = pNode->pLeft;

	/*
	 * Swap the position of the node and one of its child nodes until the
	 * position is valid for a heap.
	 */
	while (pLeft != NULL) {
		/*
		 * The node has a left child. Check if the left child
		 * should be before the node.
		 */
		lwl_PtrHeapNode * pFirst = pNode;
		if (pCmpFn(pLeft, pFirst, pCmpParam)) {
			pFirst = pLeft;
		}

		lwl_PtrHeapNode * pRight = pNode->pRight;
		if (
			pRight != NULL &&
			pCmpFn(pRight, pFirst, pCmpParam)
		) {
			/*
			 * The node has a right child that should be before the node
			 * and its left child.
			 */
			pFirst = pRight;
		}

		if (pFirst == pNode) {
			/*
			 * A valid position has been found for the node.
			 */
			break;
		}

		lwl__ptrHeapSwapAdjacentNodes(pHeap, pNode, pFirst);
		pLeft = pNode->pLeft;
	}
}

/** ****************************************************************************
 *
 * \brief		Moves node position in heap.
 *
 * \param		pHeap		Pointer to the heap.
 * \param		pNode		Pointer to node that should be moved.
 * \param[in]	pCmp		Pointer to comparison information.
 *
 * \details		Moves the node up or down in the heap until the heap property
 *				is satisfied.
 *
 * \note		This function is intended for internal heap usage only. It
 * 				should not be called from other code.
 *
 ******************************************************************************/
void lwl__ptrHeapMoveInternal(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
) {
	lwl_PtrHeapNode * pParent = pNode->pParent;

	/*
	 * Check if the node needs to be moved up or down in the heap.
	 */
	if (
		pParent == &lwl__ptrHeapRootParent ||
		pCmp->pCmpFn(pParent, pNode, pCmp->pCmpParam)
	) {
		lwl__ptrHeapMoveTowardsLeaves(pHeap, pNode, pCmp);
	} else {
		lwl__ptrHeapMoveTowardsRoot(pHeap, pNode, pCmp);
	}
}
