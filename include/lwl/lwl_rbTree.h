/** ****************************************************************************
 *
 * \file	lwl_rbTree.h
 *
 * \brief	Header file for a red black tree implementation.
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

#ifndef LWL_RBTREE_H_INCLUDED
#define LWL_RBTREE_H_INCLUDED

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

/**
 * The node color is stored in the lowest bit of the parent pointer when nodes
 * are aligned so that the bit is always zero in node pointers.
 */
#define LWL__RB_COLOR_IN_PARENT (LWL_PORT_NODE_ALIGNMENT >= 2)

/*******************************************************************************
;
;	T Y P E S
;
;-----------------------------------------------------------------------------*/

typedef enum {
	LWL__RBDIR_LEFT,
	LWL__RBDIR_RIGHT,
	LWL__RBDIR_COUNT
} lwl__RbNodeDir;

/**
 * Represents the color of a Red-Black Tree node.
 */
typedef enum {
	LWL__RBCOLOR_RED,
	LWL__RBCOLOR_BLACK
} lwl__RbNodeColor;

/**
 * Type used by each tree node.
 */
typedef struct lwl__rbTreeNode lwl_RbTreeNode;

/**
 * A node in the Red-Black Tree.
 *
 * The parent and color are only accessed with the lwl__rbNode* functions, since
 * the color may be stored in the lowest bit of the parent pointer.
 */
#if LWL__RB_COLOR_IN_PARENT
struct lwl__rbTreeNode {
	uintptr_t		 parentColor; // Parent pointer, color in the lowest bit
	lwl_RbTreeNode * children[LWL__RBDIR_COUNT];
};
#else
struct lwl__rbTreeNode {
	lwl__RbNodeColor color;
	lwl_RbTreeNode * pParent;
	lwl_RbTreeNode * children[LWL__RBDIR_COUNT];
};
#endif

/**
 * The Red-Black Tree.
 */
typedef struct {
	lwl_RbTreeNode * root;		// Pointer to the root of the tree
	lwl_RbTreeNode * pLeftmost; // Pointer to the first node of the tree
} lwl_RbTree;

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
 * \details	Used for functions that determine the order of the tree. The second
 * 			node is always the new node when inserting nodes into the tree.
 *
 ******************************************************************************/
typedef bool lwl_RbTreeCmpFn(
	const lwl_RbTreeNode * pNode1,
	const lwl_RbTreeNode * pNode2,
	const void *		   pParam
);

/**
 * Type used to store node comparing function and parameter.
 */
typedef struct {
	lwl_RbTreeCmpFn * pCmpFn;	 /**< Pointer to node comparing function. */
	const void *	  pCmpParam; /**< Node comparing function parameter. */
} lwl_RbTreeCmp;

/*******************************************************************************
;
;	F U N C T I O N S
;
;-----------------------------------------------------------------------------*/

void lwl_rbTreeInsertNode(
	lwl_RbTree *		  pTree,
	lwl_RbTreeNode *	  pNewNode,
	const lwl_RbTreeCmp * pCmp
);

void lwl_rbTreeRemoveNode(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
);

/** ****************************************************************************
 *
 * \brief		Get the parent of the supplied node.
 *
 * \param[in]	pNode		Pointer to the node.
 *
 * \return		Pointer to the parent node. NULL for the root node, and the node
 * 				itself when the node is not in a tree.
 *
 ******************************************************************************/
inline lwl_RbTreeNode * lwl__rbNodeGetParent(
	const lwl_RbTreeNode * pNode
) {
#if LWL__RB_COLOR_IN_PARENT
	return (lwl_RbTreeNode *)(pNode->parentColor & ~(uintptr_t)1U);
#else
	return pNode->pParent;
#endif
}

/** ****************************************************************************
 *
 * \brief		Get the color of the supplied node.
 *
 * \param[in]	pNode		Pointer to the node.
 *
 * \return		The color of the node.
 *
 ******************************************************************************/
inline lwl__RbNodeColor lwl__rbNodeGetColor(
	const lwl_RbTreeNode * pNode
) {
#if LWL__RB_COLOR_IN_PARENT
	return (lwl__RbNodeColor)(pNode->parentColor & 1U);
#else
	return pNode->color;
#endif
}

/** ****************************************************************************
 *
 * \brief		Set the parent of the supplied node. The color is not changed.
 *
 * \param[in]	pNode		Pointer to the node.
 * \param[in]	pParent		Pointer to the new parent node. May be NULL.
 *
 ******************************************************************************/
inline void lwl__rbNodeSetParent(
	lwl_RbTreeNode * pNode,
	lwl_RbTreeNode * pParent
) {
#if LWL__RB_COLOR_IN_PARENT
	pNode->parentColor = (uintptr_t)pParent | (pNode->parentColor & 1U);
#else
	pNode->pParent = pParent;
#endif
}

/** ****************************************************************************
 *
 * \brief		Set the color of the supplied node. The parent is not changed.
 *
 * \param[in]	pNode		Pointer to the node.
 * \param[in]	color		The new color.
 *
 ******************************************************************************/
inline void lwl__rbNodeSetColor(
	lwl_RbTreeNode * pNode,
	lwl__RbNodeColor color
) {
#if LWL__RB_COLOR_IN_PARENT
	pNode->parentColor =
		(pNode->parentColor & ~(uintptr_t)1U) | (uintptr_t)color;
#else
	pNode->color = color;
#endif
}

/** ****************************************************************************
 *
 * \brief		Set both the parent and the color of the supplied node.
 *
 * \param[in]	pNode		Pointer to the node.
 * \param[in]	pParent		Pointer to the new parent node. May be NULL.
 * \param[in]	color		The new color.
 *
 ******************************************************************************/
inline void lwl__rbNodeSetParentColor(
	lwl_RbTreeNode * pNode,
	lwl_RbTreeNode * pParent,
	lwl__RbNodeColor color
) {
#if LWL__RB_COLOR_IN_PARENT
	pNode->parentColor = (uintptr_t)pParent | (uintptr_t)color;
#else
	pNode->pParent = pParent;
	pNode->color = color;
#endif
}

/** ****************************************************************************
 *
 * \brief		Initializes the Red-Black Tree.
 *
 * \param		pTree		Poiner to the tree that should be initialized.
 *
 ******************************************************************************/
inline void lwl_rbTreeInit(
	lwl_RbTree * pTree
) {
	lwl__portAssert(pTree != NULL);

	pTree->root = NULL;
	pTree->pLeftmost = NULL;
}

/** ****************************************************************************
 *
 * \brief		Initialize tree node.
 *
 * \param[in]	pNode	Pointer to the node.
 *
 * \details
 *
 ******************************************************************************/
inline void lwl_rbNodeInit(
	lwl_RbTreeNode * pNode
) {
	lwl__portAssert(pNode != NULL);
#if LWL__RB_COLOR_IN_PARENT
	// Catches nodes in packed structures, which may not be aligned
	lwl__portAssert(
		((uintptr_t)pNode & (LWL_PORT_NODE_ALIGNMENT - 1U)) == 0U
	);
#endif

	pNode->children[LWL__RBDIR_LEFT] = NULL;
	pNode->children[LWL__RBDIR_RIGHT] = NULL;

	// A node pointing to itself is not in a tree
	lwl__rbNodeSetParentColor(pNode, pNode, LWL__RBCOLOR_RED);
}

/** ****************************************************************************
 *
 * \brief		Check if the supplied node is in a tree.
 *
 * \param[in]	pNode		The node to check.
 *
 * \retval		true		The node is in a tree.
 * \retval		false		The node is not in a tree.
 *
 * \details
 *
 ******************************************************************************/
inline bool lwl_rbNodeIsInTree(
	const lwl_RbTreeNode * pNode
) {
	lwl__portAssert(pNode != NULL);

	/*
	 * The parent pointer points to the node itself when the node is not in a
	 * tree. The root node has a NULL parent.
	 */
	bool isInTree = false;
	isInTree = lwl__rbNodeGetParent(pNode) != pNode;

	return isInTree;
}

/** ****************************************************************************
 *
 * \brief		Get the leftmost child of the supplied node.
 *
 * \param[in]	pNode		Parent node.
 *
 * \return		Pointer to the leftmost child node.
 *
 * \details
 *
 ******************************************************************************/
inline lwl_RbTreeNode * lwl_rbTreeGetLeftmostChild(
	lwl_RbTreeNode * pNode
) {
	lwl__portAssert(pNode != NULL);

	while (pNode->children[LWL__RBDIR_LEFT] != NULL) {
		pNode = pNode->children[LWL__RBDIR_LEFT];
	}

	return pNode;
}

/** ****************************************************************************
 *
 * \brief		Peek at the first node in the tree.
 *
 * \param[in]	pTree		Pointer to a tree.
 *
 * \return		Pointer to the first node. NULL if the tree is empty.
 *
 * \details
 *
 ******************************************************************************/
inline lwl_RbTreeNode * lwl_rbTreePeekFirst(
	const lwl_RbTree * pTree
) {
	lwl__portAssert(pTree != NULL);

	return pTree->pLeftmost;
}

/** ****************************************************************************
 *
 * \brief		Pop the first (leftmost) node.
 *
 * \param[in]	pTree 		Pointer to a tree.
 *
 * \return		Pointer to the popped node. NULL if the tree was empty.
 *
 * \details
 *
 ******************************************************************************/
inline lwl_RbTreeNode * lwl_rbTreePopFirst(
	lwl_RbTree * pTree
) {
	lwl__portAssert(pTree != NULL);

	lwl_RbTreeNode * pNode = lwl_rbTreePeekFirst(pTree);
	if (pNode != NULL) {
		lwl_rbTreeRemoveNode(pTree, pNode);
	}

	return pNode;
}

/** ****************************************************************************
 *
 * \brief		Store a node pointer to the root of the tree.
 *
 * \param[in]	pTree		Pointer to a tree.
 * \param[in]	pNode		Pointer to node that should be set as root node.
 *
 * \details		This is only intended for temporarily storing a node pointer to
 * 				the tree as fast as possible. It does not update any other
 * 				pointers in the tree and additional nodes cannot be stored after
 * 				this function has been called.
 *
 * \note		May only be called for an empty tree. NULL must be stored before
 * 				inserting nodes to the tree.
 *
 ******************************************************************************/
inline void lwl_rbTreeSetRootPtr(
	lwl_RbTree *	 pTree,
	lwl_RbTreeNode * pNode
) {
	lwl__portAssert(pTree != NULL);
	lwl__portAssert(pTree->root == NULL);

	pTree->root = pNode;
}

#endif /* LWL_RBTREE_H_INCLUDED */
