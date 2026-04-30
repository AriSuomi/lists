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

#include <stdio.h>
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

static lwl_PtrHeapNode * lwl__ptrHeapGetLastNode(
	const lwl_PtrHeap * pHeap
);

static void lwl__ptrHeapMoveUp(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
);

static void lwl__ptrHeapMoveDown(
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
 * \param[in]	pHeap		Pointer to heap.
 * \param[in]	pNew		Pointer to node that should be inserted.
 * \param[in]	pCmp		Pointer to node compare information.
 *
 * \retval		true		The node was inserted successfully.
 * \retval		false		Could not insert the node. It may already be in a
 *							heap.
 *
 ******************************************************************************/
bool lwl_ptrHeapInsert(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNewNode,
	const lwl_PtrHeapCmp * pCmp
) {
	bool wasInserted = false;
	bool isInHeap = lwl_ptrHeapIsNodeInHeap(pNewNode);

	if (!isInHeap) {
		/*
		 * The node is not yet in a heap. Continue with the insert.
		 */
		pHeap->nodeCount++;

		if (pHeap->pRoot == NULL) {
			/*
			 * The heap is empty. Add the new node as the root node.
			 */
			pNewNode->pParent = &lwl__ptrHeapRootParent;
			pHeap->pRoot = pNewNode;

		} else {
			/*
			 * Find the next free spot in the tree. The node count has already
			 * been incremented so the lwl__ptrHeapGetLastNode function will
			 * return the parent node of the next free spot in the heap.
			 */
			lwl_PtrHeapNode * pParent = lwl__ptrHeapGetLastNode(pHeap);

			/*
			 * Insert the new node to the next free spot.
			 */
			if (pParent->pLeft == NULL) {
				pParent->pLeft = pNewNode;
			} else {
				pParent->pRight = pNewNode;
			}

			pNewNode->pParent = pParent;

			/*
			 * Move the inserted node to the correct position.
			 */
			lwl__ptrHeapMoveDown(pHeap, pNewNode, pCmp);
		}

		wasInserted = true;
	}

	return wasInserted;
}

/** ****************************************************************************
 *
 * \brief		Remove a node from the heap.
 *
 * \param[in]	pHeap		Pointer to the heap.
 * \param[in]	pNode		Pointer to node that should be removed from the
 * 							heap.
 * \param[in]	pCmp		Pointer to node comparing information.
 *
 * \retval		true		The node was successfully removed.
 * \retval		false		Could not remove the node. Maybe it isn't in a tree.
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
		 * Cut the last node from the heap by grafting a NULL into the heap
		 * as a replacement.
		 */
		lwl_PtrHeapNode * pLastNode = lwl__ptrHeapGetLastNode(pHeap);
		lwl__ptrHeapGraftNode(pHeap, pLastNode, NULL);

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
 * \brief		Get the last node of the heap.
 *
 * \param[in]	pHeap		Pointer to the heap.
 *
 * \return		Pointer to the last node or NULL if the heap is empty.
 *
 ******************************************************************************/
static lwl_PtrHeapNode * lwl__ptrHeapGetLastNode(
	const lwl_PtrHeap * pHeap
) {
	/*
	 * First determine the path to the last node. The node count can be used
	 * as a path to the last node. Each bit from most significant to the least
	 * significant tells which direction to take when traversing the tree.
	 *
	 * The node count bits are left aligned here to make it easier to start
	 * checking bits from the most significant.
	 */
	uint16_t nodeCount = pHeap->nodeCount;
	uint8_t leadingZeroes = lwl__portClz16(nodeCount);
	uint_fast16_t path = (uint_fast16_t)(nodeCount << leadingZeroes);

	/*
	 * Traverse the path to the last node.
	 */
	lwl_PtrHeapNode * pNode = pHeap->pRoot;
	lwl_PtrHeapNode * pNextNode = pNode;

	while (pNextNode != NULL) {
		pNode = pNextNode;
		path <<= 1U;
		if ((path & LWL__PTRHEAP_U16_MSB) == 0U) {
			pNextNode = pNode->pLeft;
		} else {
			pNextNode = pNode->pRight;
		}
	}

	return pNode;
}

/** ****************************************************************************
 *
 * \brief		Graft a scion replacing an entire branch of the heap.
 *
 * \param[in]	pHeap		Pointer to the heap.
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
 * \param[in]	pHeap			Pointer to the heap.
 * \param[in]	pNode			Pointer to the node that should be replaced.
 * \param[in]	pReplacement	Pointer to the replacing node.
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
 * \param	pHeap		Pointer to the heap.
 * \param	pParent		Pointer to the parent node.
 * \param	pChild		Pointer to the child node.
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
 * \brief	Move a node down in the heap until a valid position is found.
 *
 * \param	pHeap			Pointer to the heap.
 * \param	pNode			Pointer to node that should be moved.
 * \param	pCmp			Pointer to comparison information.
 *
 ******************************************************************************/
static void lwl__ptrHeapMoveDown(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
) {
	/*
	 * Swap the position of the node and an adjacent node until we get a valid
	 * heap.
	 */
	while (
		pNode->pParent != &lwl__ptrHeapRootParent &&
		pCmp->pCmpFn(pNode, pNode->pParent, pCmp->pCmpParam)
	) {
		lwl__ptrHeapSwapAdjacentNodes(pHeap, pNode->pParent, pNode);
	}
}

/** ****************************************************************************
 *
 * \brief	Move a node up in the heap until a valid position is found.
 *
 * \param	pHeap			Pointer to the heap.
 * \param	pNode			Pointer to node that should be moved.
 * \param	pCmp			Pointer to comparison information.
 *
 ******************************************************************************/
static void lwl__ptrHeapMoveUp(
	lwl_PtrHeap *		   pHeap,
	lwl_PtrHeapNode *	   pNode,
	const lwl_PtrHeapCmp * pCmp
) {
	lwl_PtrHeapNode * pLeft = pNode->pLeft;

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
		if (pCmp->pCmpFn(pLeft, pFirst, pCmp->pCmpParam)) {
			pFirst = pLeft;
		}

		lwl_PtrHeapNode * pRight = pNode->pRight;
		if (
			pRight != NULL &&
			pCmp->pCmpFn(pRight, pFirst, pCmp->pCmpParam)
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
 * \param[in]	pHeap		Pointer to the heap.
 * \param[in]	pNode		Pointer to node that should be moved.
 * \param[in]	pCmp		Pointer to comparison information.
 *
 * \details		Moves the node up or down in the heap until the heap property
 *				is satisfied.
 *
 * \note		This function is intended for internal heap usage only. It
 * 				should not be called form other code.
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
		lwl__ptrHeapMoveUp(pHeap, pNode, pCmp);
	} else {
		lwl__ptrHeapMoveDown(pHeap, pNode, pCmp);
	}
}
