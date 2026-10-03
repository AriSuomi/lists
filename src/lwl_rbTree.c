/** ****************************************************************************
 *
 * \file	lwl_rbTree.c
 *
 * \brief	Source file for the red black tree implementation.
 *
 * \details
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

#include "lwl/lwl_port.h"
#include "lwl/lwl_rbTree.h"

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

static inline bool lwl__rbNodeIsRed(
	const lwl_RbTreeNode * pNode
);

static lwl__RbNodeDir lwl__rbTreeGetNodeDir(
	const lwl_RbTreeNode * pNode,
	const lwl_RbTreeNode * pParent
);

static void lwl__rbTreeRotate(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode,
	lwl__RbNodeDir	 direction
);

static void lwl__rbTreeInsertFixup(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
);

static void lwl__rbTreeGraftNode(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pOldBranch,
	lwl_RbTreeNode * pScion
);

static void lwl__rbTreeRemoveFixup(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode,
	lwl_RbTreeNode * pParent
);

/*******************************************************************************
;
;	I N L I N E   F U N C T I O N S
;
;-----------------------------------------------------------------------------*/

extern inline void lwl_rbTreeInit(
	lwl_RbTree * pTree
);

extern inline void lwl_rbNodeInit(
	lwl_RbTreeNode * pNode
);

extern inline bool lwl_rbNodeIsInTree(
	const lwl_RbTreeNode * pNode
);

extern inline lwl_RbTreeNode * lwl_rbTreeGetLeftmostChild(
	lwl_RbTreeNode * pNode
);

extern inline lwl_RbTreeNode * lwl_rbTreePeekFirst(
	const lwl_RbTree * pTree
);

extern inline lwl_RbTreeNode * lwl_rbTreePopFirst(
	lwl_RbTree * pTree
);

extern inline void lwl_rbTreeSetRootPtr(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
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
 * \brief		Insert the supplied node to the tree.
 *
 * \param[in]	pTree		Pointer to tree.
 * \param[in]	pNewNode	Pointer to node that should be inserted.
 * \param[in]	pCmp		Pointer to node compare information.
 *
 ******************************************************************************/
void lwl_rbInsertNode(
	lwl_RbTree *		  pTree,
	lwl_RbTreeNode *	  pNewNode,
	const lwl_RbTreeCmp * pCmp
) {
	lwl__portAssert(pTree != NULL);
	lwl__portAssert(pNewNode != NULL);
	lwl__portAssert(lwl_rbNodeIsInTree(pNewNode) == false);
	lwl__portAssert(pCmp != NULL);

	lwl_RbTreeNode * pParent = NULL;
	lwl__RbNodeDir	 dir = LWL__RBDIR_LEFT;

	/*
	 * Copied to locals since the compiler must otherwise reload them after
	 * every call, as the compare function could modify *pCmp.
	 */
	lwl_RbTreeCmpFn * pCmpFn = pCmp->pCmpFn;
	const void *	  pCmpParam = pCmp->pCmpParam;

	lwl_RbTreeNode * pTreeNode = pTree->root;
	while (pTreeNode != NULL) {
		pParent = pTreeNode;

		dir = pCmpFn(pTreeNode, pNewNode, pCmpParam)
			? LWL__RBDIR_RIGHT
			: LWL__RBDIR_LEFT;

		pTreeNode = pTreeNode->children[dir];
	}

	pNewNode->color = LWL__RBCOLOR_RED;
	pNewNode->children[LWL__RBDIR_LEFT] = NULL;
	pNewNode->children[LWL__RBDIR_RIGHT] = NULL;
	pNewNode->pParent = pParent;

	if (pParent == NULL) {
		pTree->root = pNewNode;
	} else {
		pParent->children[dir] = pNewNode;
	}

	/*
	 * The new node is the first node only if it was added as the left child of
	 * the previous first node. This also covers an empty tree, where both the
	 * parent and the previous first node are NULL.
	 */
	if ((pParent == pTree->pLeftmost) && (dir == LWL__RBDIR_LEFT)) {
		pTree->pLeftmost = pNewNode;
	}

	lwl__rbTreeInsertFixup(pTree, pNewNode);
}

/** ****************************************************************************
 *
 * \brief		Remove a node from the tree.
 *
 * \param[in]	pTree		Pointer to the tree.
 * \param[in]	pNode		Pointer to node that should be removed from the
 * 							tree.
 * \param[in]	pCmp		Pointer to node comparing information.
 *
 ******************************************************************************/
void lwl_rbTreeRemoveNode(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
) {
	lwl__portAssert(pTree != NULL);
	lwl__portAssert(pNode != NULL);
	lwl__portAssert(lwl_rbNodeIsInTree(pNode) == true);

	/*
	 * The fixup node may be NULL, so its parent is tracked separately.
	 */
	lwl_RbTreeNode * pFixupNode = NULL;
	lwl_RbTreeNode * pFixupParent = pNode->pParent;
	lwl__RbNodeColor originalColor = pNode->color;

	if (pNode == pTree->pLeftmost) {
		/*
		 * The first node has no left child, so the next node is its right
		 * child if it has one. In a red black tree that right child is a red
		 * leaf. Otherwise the next node is the parent.
		 */
		lwl_RbTreeNode * pNextNode = pNode->children[LWL__RBDIR_RIGHT];

		lwl__portAssert(pNode->children[LWL__RBDIR_LEFT] == NULL);
		lwl__portAssert(
			(pNextNode == NULL) ||
			(pNextNode->children[LWL__RBDIR_LEFT] == NULL)
		);

		pTree->pLeftmost = (pNextNode != NULL) ? pNextNode : pFixupParent;
	}

	if (pNode->children[LWL__RBDIR_LEFT] == NULL) {
		pFixupNode = pNode->children[LWL__RBDIR_RIGHT];
		lwl__rbTreeGraftNode(pTree, pNode, pNode->children[LWL__RBDIR_RIGHT]);

	} else if (pNode->children[LWL__RBDIR_RIGHT] == NULL) {
		pFixupNode = pNode->children[LWL__RBDIR_LEFT];
		lwl__rbTreeGraftNode(pTree, pNode, pNode->children[LWL__RBDIR_LEFT]);

	} else {
		lwl_RbTreeNode * pNextNode = lwl_rbTreeGetLeftmostChild(
			pNode->children[LWL__RBDIR_RIGHT]
		);

		originalColor = pNextNode->color;
		pFixupNode = pNextNode->children[LWL__RBDIR_RIGHT];

		if (pNextNode->pParent == pNode) {
			pFixupParent = pNextNode;

		} else {
			pFixupParent = pNextNode->pParent;

			lwl__rbTreeGraftNode(
				pTree,
				pNextNode,
				pNextNode->children[LWL__RBDIR_RIGHT]
			);

			pNextNode->children[LWL__RBDIR_RIGHT] =
				pNode->children[LWL__RBDIR_RIGHT];

			pNextNode->children[LWL__RBDIR_RIGHT]->pParent = pNextNode;
		}

		lwl__rbTreeGraftNode(pTree, pNode, pNextNode);
		pNextNode->children[LWL__RBDIR_LEFT] =
			pNode->children[LWL__RBDIR_LEFT];
		pNextNode->children[LWL__RBDIR_LEFT]->pParent = pNextNode;
		pNextNode->color = pNode->color;
	}

	if (originalColor == LWL__RBCOLOR_BLACK) {
		lwl__rbTreeRemoveFixup(pTree, pFixupNode, pFixupParent);
	}

	// Mark the node as not being in a tree
	pNode->pParent = pNode;
}

/** ****************************************************************************
 *
 * \brief		Check if the supplied node is red.
 *
 * \param[in]	pNode		Pointer to the node. May be NULL.
 *
 * \retval		true		The node is red.
 * \retval		false		The node is black or NULL.
 *
 ******************************************************************************/
static inline bool lwl__rbNodeIsRed(
	const lwl_RbTreeNode * pNode
) {
	return (pNode != NULL) && (pNode->color == LWL__RBCOLOR_RED);
}

/** ****************************************************************************
 *
 * \brief		Perform a rotation around the specified node.
 *
 * \param[in]	pTree		Pointer to the tree.
 * \param[in]	pNode		The rotation should be done around this node.
 * \param[in]	direction	The rotation direction.
 *
 * \details
 *
 * \note
 *
 ******************************************************************************/
static void lwl__rbTreeRotate(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode,
	lwl__RbNodeDir	 direction
) {
	lwl_RbTreeNode * pChild = pNode->children[LWL__RBDIR_RIGHT - direction];
	lwl_RbTreeNode * pParent = pNode->pParent;

	pNode->children[LWL__RBDIR_RIGHT - direction] = pChild->children[direction];

	if (pChild->children[direction] != NULL) {
		pChild->children[direction]->pParent = pNode;
	}

	pChild->pParent = pParent;

	if (pParent == NULL) {
		pTree->root = pChild;

	} else if (pNode == pParent->children[direction]) {
		pParent->children[direction] = pChild;

	} else {
		pParent->children[LWL__RBDIR_RIGHT - direction] = pChild;
	}

	pChild->children[direction] = pNode;
	pNode->pParent = pChild;
}

/** ****************************************************************************
 *
 * \brief		Check if the supplied node is the left or right child of its
 *				parent
 *
 * \param[in]	pNode		Pointer to the node. May be NULL.
 * \param[in]	pParent		Pointer to the parent node.
 *
 * \retval		LWL__RBDIR_RIGHT	The node is the right child of its parent.
 * \retval		LWL__RBDIR_LEFT		The node is the left child of its parent.
 *
 * \note		The node cannot be the root node!
 *
 ******************************************************************************/
static lwl__RbNodeDir lwl__rbTreeGetNodeDir(
	const lwl_RbTreeNode * pNode,
	const lwl_RbTreeNode * pParent
) {
	lwl__portAssert(pParent != NULL);

	return (pParent->children[LWL__RBDIR_RIGHT] == pNode)
		? LWL__RBDIR_RIGHT
		: LWL__RBDIR_LEFT;
}

/** ****************************************************************************
 *
 * \brief		Perform fixup after node insertion.
 *
 * \param[in]	pTree		Pointer to the tree.
 * \param[in]	pNode		The new inserted node.
 *
 * \details
 *
 * \note
 *
 ******************************************************************************/
static void lwl__rbTreeInsertFixup(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
) {
	lwl_RbTreeNode * pParent = pNode->pParent;

	/*
	 * A red parent is never the root node, so the grandparent always exists
	 * inside the loop.
	 */
	while (lwl__rbNodeIsRed(pParent)) {
		lwl__RbNodeDir parentDir = lwl__rbTreeGetNodeDir(
			pParent,
			pParent->pParent
		);

		lwl_RbTreeNode * pUncle =
			pParent->pParent->children[LWL__RBDIR_RIGHT - parentDir];

		if (lwl__rbNodeIsRed(pUncle)) {
			/*
			 * Both the uncle and the parent are red. They can both be
			 * made black if we make the grandparent red.
			 */
			pParent->color = LWL__RBCOLOR_BLACK;
			pUncle->color = LWL__RBCOLOR_BLACK;
			pParent->pParent->color = LWL__RBCOLOR_RED;

			pNode = pParent->pParent;
			pParent = pNode->pParent;

		} else {
			lwl__RbNodeDir nodeDir =
				lwl__rbTreeGetNodeDir(pNode, pParent);

			if (parentDir != nodeDir) {
				// Case 2: Triangle
				pNode = pParent;
				lwl__rbTreeRotate(pTree, pNode, parentDir);
				pParent = pNode->pParent;
			}

			// Case 3: Line
			pParent->color = LWL__RBCOLOR_BLACK;
			pParent->pParent->color = LWL__RBCOLOR_RED;
			lwl__rbTreeRotate(
				pTree,
				pParent->pParent,
				LWL__RBDIR_RIGHT - parentDir
			);

			pParent = pNode->pParent;
		}
	}

	pTree->root->color = LWL__RBCOLOR_BLACK;
}

/** ****************************************************************************
 *
 * \brief		Graft a scion replacing an entire branch of the tree.
 *
 * \param[in]	pTree		Pointer to the tree.
 * \param[in]	pOldBranch	Pointer to the branch that should be replaced.
 * \param[in]	pScion		Pointer to the replacement branch. May be NULL if
 * 							the old branch should just be removed from the tree.
 *
 * \details		The scion will replace the old branch of the tree. The scion may
 * 				also have child nodes.
 *
 ******************************************************************************/
static void lwl__rbTreeGraftNode(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pOldBranch,
	lwl_RbTreeNode * pScion
) {
	lwl_RbTreeNode * pRootstock = pOldBranch->pParent;

	if (pRootstock == NULL) {
		pTree->root = pScion;

	} else {
		lwl__RbNodeDir nodeDir = lwl__rbTreeGetNodeDir(pOldBranch, pRootstock);
		pRootstock->children[nodeDir] = pScion;
	}

	if (pScion != NULL) {
		pScion->pParent = pRootstock;
	}
}

/** ****************************************************************************
 *
 * \brief		Perform fixup after node removal.
 *
 * \param[in]	pTree		Pointer to the tree.
 * \param[in]	pNode		The node to start the fixup from. May be NULL.
 * \param[in]	pParent		Pointer to the parent of pNode. NULL if pNode is
 * 							the root node.
 *
 * \details		The parent is passed separately since pNode may be NULL, in
 * 				which case its parent cannot be read from the node.
 *
 ******************************************************************************/
static void lwl__rbTreeRemoveFixup(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode,
	lwl_RbTreeNode * pParent
) {
	while ((pNode != pTree->root) && !lwl__rbNodeIsRed(pNode)) {
		lwl__RbNodeDir nodeDir = lwl__rbTreeGetNodeDir(pNode, pParent);

		lwl_RbTreeNode * pSibling =
			pParent->children[LWL__RBDIR_RIGHT - nodeDir];

		if (lwl__rbNodeIsRed(pSibling)) {
			// Case 1: Sibling is LWL__RBCOLOR_RED
			pSibling->color = LWL__RBCOLOR_BLACK;
			pParent->color = LWL__RBCOLOR_RED;
			lwl__rbTreeRotate(pTree, pParent, nodeDir);
			pSibling = pParent->children[LWL__RBDIR_RIGHT - nodeDir];
		}

		if (
			!lwl__rbNodeIsRed(pSibling->children[nodeDir]) &&
			!lwl__rbNodeIsRed(pSibling->children[LWL__RBDIR_RIGHT - nodeDir])
		) {
			// Case 2
			pSibling->color = LWL__RBCOLOR_RED;
			pNode = pParent;
			pParent = pNode->pParent;

		} else {
			if (
				!lwl__rbNodeIsRed(
					pSibling->children[LWL__RBDIR_RIGHT - nodeDir]
				)
			) {
				// Case 3: Triangle
				pSibling->children[nodeDir]->color = LWL__RBCOLOR_BLACK;
				pSibling->color = LWL__RBCOLOR_RED;
				lwl__rbTreeRotate(pTree, pSibling, LWL__RBDIR_RIGHT - nodeDir);
				pSibling = pParent->children[LWL__RBDIR_RIGHT - nodeDir];
			}

			// Case 4: Line
			pSibling->color = pParent->color;
			pParent->color = LWL__RBCOLOR_BLACK;
			pSibling->children[LWL__RBDIR_RIGHT - nodeDir]->color =
				LWL__RBCOLOR_BLACK;
			lwl__rbTreeRotate(pTree, pParent, nodeDir);
			pNode = pTree->root;
		}
	}

	if (pNode != NULL) {
		pNode->color = LWL__RBCOLOR_BLACK;
	}
}
