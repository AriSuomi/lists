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
	lwl_RbTreeNode * pNode
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

/**
 * Sentinel parent node for the root node. This is a dummy node which makes it
 * possible to determine if a node is in a tree or not even when it is the
 * only node in the tree.
 */
lwl_RbTreeNode lwl__rbTreeNull = {
	LWL__RBCOLOR_BLACK,
	&lwl__rbTreeNull,
	{&lwl__rbTreeNull, &lwl__rbTreeNull}
};

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
 * \retval		true		The node was inserted successfully.
 * \retval		false		Could not insert the node. It may already be in a
 *							tree.
 *
 ******************************************************************************/
bool lwl_rbInsertNode(
	lwl_RbTree *		  pTree,
	lwl_RbTreeNode *	  pNewNode,
	const lwl_RbTreeCmp * pCmp
) {
	lwl__portAssert(pTree != NULL);
	lwl__portAssert(pNewNode != NULL);
	lwl__portAssert(pCmp != NULL);

	lwl_RbTreeNode * pParent = &lwl__rbTreeNull;
	lwl__RbNodeDir	 dir = LWL__RBDIR_LEFT;

	lwl_RbTreeNode * pTreeNode = pTree->root;
	while (pTreeNode != &lwl__rbTreeNull) {
		pParent = pTreeNode;

		dir = pCmp->pCmpFn(pTreeNode, pNewNode, pCmp->pCmpParam)
			? LWL__RBDIR_RIGHT
			: LWL__RBDIR_LEFT;

		pTreeNode = pTreeNode->children[dir];
	}

	pNewNode->color = LWL__RBCOLOR_RED;
	pNewNode->children[LWL__RBDIR_LEFT] = &lwl__rbTreeNull;
	pNewNode->children[LWL__RBDIR_RIGHT] = &lwl__rbTreeNull;
	pNewNode->pParent = pParent;

	if (pParent == &lwl__rbTreeNull) {
		pTree->root = pNewNode;
	} else {
		pParent->children[dir] = pNewNode;
	}

	lwl__rbTreeInsertFixup(pTree, pNewNode);

	return true;
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
 * \retval		true		The node was successfully removed.
 * \retval		false		Could not remove the node. Maybe it isn't in a tree.
 *
 ******************************************************************************/
void lwl_rbTreeRemoveNode(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
) {
	lwl__portAssert(pTree != NULL);
	lwl__portAssert(pNode != NULL);
	lwl__portAssert(lwl_rbNodeIsInTree(pNode) == true);

	lwl_RbTreeNode * pFixupNode = NULL;
	lwl__RbNodeColor originalColor = pNode->color;

	if (pNode->children[LWL__RBDIR_LEFT] == &lwl__rbTreeNull) {
		pFixupNode = pNode->children[LWL__RBDIR_RIGHT];
		lwl__rbTreeGraftNode(pTree, pNode, pNode->children[LWL__RBDIR_RIGHT]);

	} else if (pNode->children[LWL__RBDIR_RIGHT] == &lwl__rbTreeNull) {
		pFixupNode = pNode->children[LWL__RBDIR_LEFT];
		lwl__rbTreeGraftNode(pTree, pNode, pNode->children[LWL__RBDIR_LEFT]);

	} else {
		lwl_RbTreeNode * pNextNode = lwl_rbTreeGetLeftmostChild(
			pNode->children[LWL__RBDIR_RIGHT]
		);

		originalColor = pNextNode->color;
		pFixupNode = pNextNode->children[LWL__RBDIR_RIGHT];

		if (pNextNode->pParent == pNode) {
			// Important for when pFixupNode is NIL
			pFixupNode->pParent = pNextNode;

		} else {
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
		lwl__rbTreeRemoveFixup(pTree, pFixupNode);
	}
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

	if (pChild->children[direction] != &lwl__rbTreeNull) {
		pChild->children[direction]->pParent = pNode;
	}

	pChild->pParent = pParent;

	if (pParent == &lwl__rbTreeNull) {
		pTree->root = pChild;

	} else if (pNode == pNode->pParent->children[direction]) {
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
 * \param[in]	pNode		Pointer to the node.
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
	lwl__portAssert(pParent != &lwl__rbTreeNull);

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

	while (pParent->color == LWL__RBCOLOR_RED) {
		lwl__RbNodeDir parentDir = lwl__rbTreeGetNodeDir(
			pParent,
			pParent->pParent
		);

		lwl_RbTreeNode * pUncle =
			pParent->pParent->children[LWL__RBDIR_RIGHT - parentDir];

		if (pUncle->color == LWL__RBCOLOR_RED) {
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

	if (pRootstock == &lwl__rbTreeNull) {
		pTree->root = pScion;

	} else {
		lwl__RbNodeDir nodeDir = lwl__rbTreeGetNodeDir(pOldBranch, pRootstock);
		pRootstock->children[nodeDir] = pScion;
	}

	pScion->pParent = pRootstock;
}

/** ****************************************************************************
 *
 * \brief		Perform fixup after node removal.
 *
 * \param[in]	pTree		Pointer to the tree.
 * \param[in]	pNode		The node to start the fixup from.
 *
 * \details
 *
 * \note
 *
 ******************************************************************************/
static void lwl__rbTreeRemoveFixup(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
) {
	while ((pNode != pTree->root) && (pNode->color == LWL__RBCOLOR_BLACK)) {
		lwl__RbNodeDir nodeDir = lwl__rbTreeGetNodeDir(pNode, pNode->pParent);

		lwl_RbTreeNode * pSibling =
			pNode->pParent->children[LWL__RBDIR_RIGHT - nodeDir];

		if (pSibling->color == LWL__RBCOLOR_RED) {
			// Case 1: Sibling is LWL__RBCOLOR_RED
			pSibling->color = LWL__RBCOLOR_BLACK;
			pNode->pParent->color = LWL__RBCOLOR_RED;
			lwl__rbTreeRotate(pTree, pNode->pParent, nodeDir);
			pSibling = pNode->pParent->children[LWL__RBDIR_RIGHT - nodeDir];
		}

		if (
			(pSibling->children[nodeDir]->color == LWL__RBCOLOR_BLACK) &&
			(pSibling->children[LWL__RBDIR_RIGHT - nodeDir]->color ==
			 LWL__RBCOLOR_BLACK)
		) {
			// Case 2
			pSibling->color = LWL__RBCOLOR_RED;
			pNode = pNode->pParent;

		} else {
			if (
				pSibling->children[LWL__RBDIR_RIGHT - nodeDir]->color ==
				LWL__RBCOLOR_BLACK
			) {
				// Case 3: Triangle
				pSibling->children[nodeDir]->color = LWL__RBCOLOR_BLACK;
				pSibling->color = LWL__RBCOLOR_RED;
				lwl__rbTreeRotate(pTree, pSibling, LWL__RBDIR_RIGHT - nodeDir);
				pSibling = pNode->pParent->children[LWL__RBDIR_RIGHT - nodeDir];
			}

			// Case 4: Line
			pSibling->color = pNode->pParent->color;
			pNode->pParent->color = LWL__RBCOLOR_BLACK;
			pSibling->children[LWL__RBDIR_RIGHT - nodeDir]->color =
				LWL__RBCOLOR_BLACK;
			lwl__rbTreeRotate(pTree, pNode->pParent, nodeDir);
			pNode = pTree->root;
		}
	}

	pNode->color = LWL__RBCOLOR_BLACK;
}
