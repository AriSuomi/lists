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

#if LWL__RB_COLOR_IN_PARENT
/*
 * Compile time check that tree nodes are aligned at least as much as the port
 * claims with LWL_PORT_NODE_ALIGNMENT, so the lowest bit of a node pointer is
 * free for the color. The offset of the node after a single char is its
 * alignment. Done with a negative array size since _Static_assert and _Alignof
 * are not available before C11.
 */
typedef struct {
	char		   c;
	lwl_RbTreeNode node;
} lwl__RbAlignCheck;

typedef char lwl__rbNodeAlignmentMatchesPort[(offsetof(lwl__RbAlignCheck, node) >= LWL_PORT_NODE_ALIGNMENT) ? 1 : -1];
#endif

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

static void lwl__rbTreeGraftNode(
	lwl_RbTree *		   pTree,
	const lwl_RbTreeNode * pOldBranch,
	lwl_RbTreeNode *	   pScion
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

extern inline lwl_RbTreeNode * lwl__rbNodeGetParent(
	const lwl_RbTreeNode * pNode
);

extern inline lwl__RbNodeColor lwl__rbNodeGetColor(
	const lwl_RbTreeNode * pNode
);

extern inline void lwl__rbNodeSetParent(
	lwl_RbTreeNode * pNode,
	lwl_RbTreeNode * pParent
);

extern inline void lwl__rbNodeSetColor(
	lwl_RbTreeNode * pNode,
	lwl__RbNodeColor color
);

extern inline void lwl__rbNodeSetParentColor(
	lwl_RbTreeNode * pNode,
	lwl_RbTreeNode * pParent,
	lwl__RbNodeColor color
);

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

extern inline void lwl_rbTreeInsertNode(
	lwl_RbTree *		  pTree,
	lwl_RbTreeNode *	  pNewNode,
	const lwl_RbTreeCmp * pCmp
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
 * \brief		Remove a node from the tree.
 *
 * \param		pTree		Pointer to the tree.
 * \param		pNode		Pointer to node that should be removed from the
 * 							tree.
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
	lwl_RbTreeNode * pFixupParent = lwl__rbNodeGetParent(pNode);
	lwl__RbNodeColor originalColor = lwl__rbNodeGetColor(pNode);

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

		originalColor = lwl__rbNodeGetColor(pNextNode);
		pFixupNode = pNextNode->children[LWL__RBDIR_RIGHT];

		lwl_RbTreeNode * pNextParent = lwl__rbNodeGetParent(pNextNode);

		if (pNextParent == pNode) {
			pFixupParent = pNextNode;

		} else {
			pFixupParent = pNextParent;

			lwl__rbTreeGraftNode(
				pTree,
				pNextNode,
				pNextNode->children[LWL__RBDIR_RIGHT]
			);

			pNextNode->children[LWL__RBDIR_RIGHT] =
				pNode->children[LWL__RBDIR_RIGHT];

			lwl__rbNodeSetParent(
				pNextNode->children[LWL__RBDIR_RIGHT],
				pNextNode
			);
		}

		lwl__rbTreeGraftNode(pTree, pNode, pNextNode);
		pNextNode->children[LWL__RBDIR_LEFT] =
			pNode->children[LWL__RBDIR_LEFT];
		lwl__rbNodeSetParent(pNextNode->children[LWL__RBDIR_LEFT], pNextNode);
		lwl__rbNodeSetColor(pNextNode, lwl__rbNodeGetColor(pNode));
	}

	if (originalColor == LWL__RBCOLOR_BLACK) {
		if (lwl__rbNodeIsRed(pFixupNode)) {
			/*
			 * The red node that took the place of the removed black node only
			 * needs to be painted black. This is the common case, so it is
			 * handled here without calling the fixup function.
			 */
			lwl__rbNodeSetColor(pFixupNode, LWL__RBCOLOR_BLACK);
		} else {
			lwl__rbTreeRemoveFixup(pTree, pFixupNode, pFixupParent);
		}
	}

	// Mark the node as not being in a tree
	lwl__rbNodeSetParentColor(pNode, pNode, LWL__RBCOLOR_RED);
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
	return (pNode != NULL) && (lwl__rbNodeGetColor(pNode) == LWL__RBCOLOR_RED);
}

/** ****************************************************************************
 *
 * \brief		Perform a rotation around the specified node.
 *
 * \param		pTree		Pointer to the tree.
 * \param		pNode		The rotation should be done around this node.
 * \param		direction	The rotation direction.
 *
 * \details
 *
 ******************************************************************************/
static void lwl__rbTreeRotate(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode,
	lwl__RbNodeDir	 direction
) {
	lwl_RbTreeNode * pChild = pNode->children[LWL__RBDIR_RIGHT - direction];
	lwl_RbTreeNode * pParent = lwl__rbNodeGetParent(pNode);

	/*
	 * The inner child of pChild moves over to pNode. Read once, since the
	 * compiler must otherwise reload it after the store to pNode->children.
	 */
	lwl_RbTreeNode * pInnerChild = pChild->children[direction];

	pNode->children[LWL__RBDIR_RIGHT - direction] = pInnerChild;

	if (pInnerChild != NULL) {
		lwl__rbNodeSetParent(pInnerChild, pNode);
	}

	lwl__rbNodeSetParent(pChild, pParent);

	if (pParent == NULL) {
		pTree->root = pChild;

	} else if (pNode == pParent->children[direction]) {
		pParent->children[direction] = pChild;

	} else {
		pParent->children[LWL__RBDIR_RIGHT - direction] = pChild;
	}

	pChild->children[direction] = pNode;
	lwl__rbNodeSetParent(pNode, pChild);
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
 * \param		pTree		Pointer to the tree.
 * \param		pNode		The new inserted node.
 *
 * \details		lwl_rbTreeInsertNode() calls this only when the parent of the
 * 				new node is red. The other cases need no fixup and are handled
 * 				there without a call.
 *
 ******************************************************************************/
void lwl__rbTreeInsertFixup(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
) {
	lwl_RbTreeNode * pParent = lwl__rbNodeGetParent(pNode);

	/*
	 * A red parent is never the root node, so the grandparent always exists
	 * inside the loop.
	 */
	while (lwl__rbNodeIsRed(pParent)) {
		lwl_RbTreeNode * pGrandparent = lwl__rbNodeGetParent(pParent);

		lwl__RbNodeDir parentDir = lwl__rbTreeGetNodeDir(pParent, pGrandparent);

		lwl_RbTreeNode * pUncle =
			pGrandparent->children[LWL__RBDIR_RIGHT - parentDir];

		if (lwl__rbNodeIsRed(pUncle)) {
			/*
			 * Both the uncle and the parent are red. They can both be
			 * made black if we make the grandparent red.
			 */
			lwl__rbNodeSetColor(pParent, LWL__RBCOLOR_BLACK);
			lwl__rbNodeSetColor(pUncle, LWL__RBCOLOR_BLACK);
			lwl__rbNodeSetColor(pGrandparent, LWL__RBCOLOR_RED);

			pNode = pGrandparent;
			pParent = lwl__rbNodeGetParent(pNode);

		} else {
			lwl__RbNodeDir nodeDir =
				lwl__rbTreeGetNodeDir(pNode, pParent);

			if (parentDir != nodeDir) {
				// Case 2: Triangle
				pNode = pParent;
				lwl__rbTreeRotate(pTree, pNode, parentDir);
				pParent = lwl__rbNodeGetParent(pNode);
			}

			// Case 3: Line
			lwl__rbNodeSetColor(pParent, LWL__RBCOLOR_BLACK);
			lwl__rbNodeSetColor(pGrandparent, LWL__RBCOLOR_RED);
			lwl__rbTreeRotate(
				pTree,
				pGrandparent,
				LWL__RBDIR_RIGHT - parentDir
			);

			/*
			 * The subtree now has a black root, so the tree is valid. The
			 * tree root is also black: either it is the old parent or it
			 * was not changed.
			 */
			return;
		}
	}

	lwl__rbNodeSetColor(pTree->root, LWL__RBCOLOR_BLACK);
}

/** ****************************************************************************
 *
 * \brief		Graft a scion replacing an entire branch of the tree.
 *
 * \param		pTree		Pointer to the tree.
 * \param[in]	pOldBranch	Pointer to the branch that should be replaced.
 * \param		pScion		Pointer to the replacement branch. May be NULL if
 * 							the old branch should just be removed from the tree.
 *
 * \details		The scion will replace the old branch of the tree. The scion may
 * 				also have child nodes.
 *
 ******************************************************************************/
static void lwl__rbTreeGraftNode(
	lwl_RbTree *		   pTree,
	const lwl_RbTreeNode * pOldBranch,
	lwl_RbTreeNode *	   pScion
) {
	lwl_RbTreeNode * pRootstock = lwl__rbNodeGetParent(pOldBranch);

	if (pRootstock == NULL) {
		pTree->root = pScion;

	} else {
		lwl__RbNodeDir nodeDir = lwl__rbTreeGetNodeDir(pOldBranch, pRootstock);
		pRootstock->children[nodeDir] = pScion;
	}

	if (pScion != NULL) {
		lwl__rbNodeSetParent(pScion, pRootstock);
	}
}

/** ****************************************************************************
 *
 * \brief		Perform fixup after node removal.
 *
 * \param		pTree		Pointer to the tree.
 * \param		pNode		The node to start the fixup from. May be NULL.
 * \param		pParent		Pointer to the parent of pNode. NULL if pNode is
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
	// pNode is the root when it has no parent
	while ((pParent != NULL) && !lwl__rbNodeIsRed(pNode)) {
		/*
		 * This also works when pNode is NULL. The NULL branch is missing one
		 * black, so the sibling branch has at least one black node and cannot
		 * be NULL. Only the branch of pNode can then be equal to NULL.
		 */
		lwl__RbNodeDir nodeDir = lwl__rbTreeGetNodeDir(pNode, pParent);

		lwl_RbTreeNode * pSibling =
			pParent->children[LWL__RBDIR_RIGHT - nodeDir];

		lwl__portAssert(pSibling != NULL);

		/*
		 * Not lwl__rbNodeIsRed(), since its NULL check would make the compiler
		 * assume that the sibling may be NULL (-Wnull-dereference).
		 */
		if (lwl__rbNodeGetColor(pSibling) == LWL__RBCOLOR_RED) {
			// Case 1: Sibling is LWL__RBCOLOR_RED
			lwl__rbNodeSetColor(pSibling, LWL__RBCOLOR_BLACK);
			lwl__rbNodeSetColor(pParent, LWL__RBCOLOR_RED);
			lwl__rbTreeRotate(pTree, pParent, nodeDir);
			pSibling = pParent->children[LWL__RBDIR_RIGHT - nodeDir];
		}

		if (
			!lwl__rbNodeIsRed(pSibling->children[nodeDir]) &&
			!lwl__rbNodeIsRed(pSibling->children[LWL__RBDIR_RIGHT - nodeDir])
		) {
			// Case 2
			lwl__rbNodeSetColor(pSibling, LWL__RBCOLOR_RED);
			pNode = pParent;
			pParent = lwl__rbNodeGetParent(pNode);

		} else {
			if (
				!lwl__rbNodeIsRed(
					pSibling->children[LWL__RBDIR_RIGHT - nodeDir]
				)
			) {
				// Case 3: Triangle
				lwl__rbNodeSetColor(
					pSibling->children[nodeDir],
					LWL__RBCOLOR_BLACK
				);
				lwl__rbNodeSetColor(pSibling, LWL__RBCOLOR_RED);
				lwl__rbTreeRotate(pTree, pSibling, LWL__RBDIR_RIGHT - nodeDir);
				pSibling = pParent->children[LWL__RBDIR_RIGHT - nodeDir];
			}

			// Case 4: Line
			lwl__rbNodeSetColor(pSibling, lwl__rbNodeGetColor(pParent));
			lwl__rbNodeSetColor(pParent, LWL__RBCOLOR_BLACK);
			lwl__rbNodeSetColor(
				pSibling->children[LWL__RBDIR_RIGHT - nodeDir],
				LWL__RBCOLOR_BLACK
			);
			lwl__rbTreeRotate(pTree, pParent, nodeDir);

			/*
			 * The missing black is restored, so the tree is valid. The tree
			 * root is also black: either it is the sibling, which took the
			 * color of the old black root, or it was not changed.
			 */
			return;
		}
	}

	if (pNode != NULL) {
		lwl__rbNodeSetColor(pNode, LWL__RBCOLOR_BLACK);
	}
}
