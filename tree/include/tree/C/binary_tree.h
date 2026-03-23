#ifndef __BINARY_TREE_C_VERSION_DEFINITION_HEADER__
#define __BINARY_TREE_C_VERSION_DEFINITION_HEADER__
#include <tree/C/treelib_api.h>
#include <tree/C/treelib_extern.h>
#include <tree/C/compare_func.h>
#include <tree/C/op_result.h>
#include <stdint.h>


TREELIB_EXTERNC_DECL_BEGIN


typedef struct binaryTreeNode {
    struct binaryTreeNode* m_left;
    struct binaryTreeNode* m_right;
    struct binaryTreeNode* m_parent;
    void*           m_data;
    int8_t          m_height;
    int8_t          m_balance;
    uint8_t         m_reserved[6];
} binaryTreeNode;


binaryTreeResult_t TREELIB_API binaryTreeNodeCreate(
    binaryTreeNode* rootNode, 
    void*           value, 
    uint32_t        valueSizeBytes
);
binaryTreeResult_t TREELIB_API binaryTreeNodeCreateWithPointers(
    binaryTreeNode* rootNode,
    binaryTreeNode* leftNode,
    binaryTreeNode* rightNode,
    binaryTreeNode* parentNode,
    void*           value, 
    uint32_t        valueSizeBytes
);
void TREELIB_API binaryTreeNodeDestroy(binaryTreeNode* node);
void TREELIB_API binaryTreeDestroy(binaryTreeNode* rootNode, uint32_t binaryTreeSizeHint);
binaryTreeResult_t TREELIB_API binaryTreeDeepCopy(
    binaryTreeNode*  treeIn,
    uint32_t         binaryTreeSize,
    binaryTreeNode** treeOut
);
void TREELIB_API binaryTreeNodeShallowCopy(
    binaryTreeNode* nodeIn,
    binaryTreeNode* nodeOut
);
void TREELIB_API binaryTreeNodeMove(
    binaryTreeNode* toMoveFrom,
    binaryTreeNode* toMoveTo
);

binaryTreeBool_t TREELIB_API binaryTreeNodeIsLeaf(binaryTreeNode const* node);
binaryTreeBool_t TREELIB_API binaryTreeNodeIsSingleNodeParent(binaryTreeNode const* node);
binaryTreeBool_t TREELIB_API binaryTreeNodeIsFull(binaryTreeNode const* node);
binaryTreeNode*  TREELIB_API binaryTreeFindMax(binaryTreeNode* node);
binaryTreeNode*  TREELIB_API binaryTreeFindMin(binaryTreeNode* node);
binaryTreeBool_t TREELIB_API binaryTreeIsValidBST(
    binaryTreeNode const*    node, 
    uint32_t                 binaryTreeSizeHint,
    binaryTreeComparatorFunc cmp
);
binaryTreeBool_t TREELIB_API binaryTreeSearchValue(
    binaryTreeNode*          node,
    void*                    value,
    binaryTreeComparatorFunc cmp,
    binaryTreeNode**         outNodeIfFound
);


TREELIB_EXTERNC_DECL_END

#endif /* __BINARY_TREE_C_VERSION_DEFINITION_HEADER__ */
