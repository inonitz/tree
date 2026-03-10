#include "tree/C/op_result.h"
#include <tree/C/binary_tree.h>
#include <tree/C/queue.h>
#include <string.h>
#include <stdlib.h>


static const uint32_t gkGenericQueueArbitraryInitialSize = 8192;


binaryTreeResult_t binaryTreeNodeCreate(
    binaryTreeNode* rootNode, 
    void*           value, 
    uint32_t        valueSizeBytes
) {
    memset(rootNode, 0x00, sizeof(binaryTreeNode));


    rootNode->m_data = malloc(valueSizeBytes);
    if(rootNode->m_data == NULL) {
        return BINARY_TREE_OP_FAILURE;
    }


    memcpy(rootNode->m_data, value, valueSizeBytes);
    return BINARY_TREE_OP_SUCCESS;
}


binaryTreeResult_t binaryTreeNodeCreateWithPointers(
    binaryTreeNode* rootNode,
    binaryTreeNode* leftNode,
    binaryTreeNode* rightNode,
    binaryTreeNode* parentNode,
    void*           value, 
    uint32_t        valueSizeBytes
) {
    memset(rootNode, 0x00, sizeof(binaryTreeNode));
    
    rootNode->m_data = malloc(valueSizeBytes);
    if(rootNode->m_data == NULL) {
        return BINARY_TREE_OP_FAILURE;
    }

    memcpy(rootNode->m_data, value, valueSizeBytes);
    rootNode->m_left   = leftNode;
    rootNode->m_right  = rightNode;
    rootNode->m_parent = parentNode;
    return BINARY_TREE_OP_SUCCESS;
}


void binaryTreeNodeDestroy(binaryTreeNode* node)
{
    free(node->m_data);
    memset(node, 0x00, sizeof(binaryTreeNode));
    /* 
        The node object itself may have been allocated out of
        this scope, and thus its' memory shall be managed externally.
    */
    return;
}


void binaryTreeDestroy(binaryTreeNode* rootNode, uint32_t binaryTreeSizeHint) 
{
    uint32_t        currLevelSize = 0;
    binaryTreeNode* currNode      = rootNode;
    GenericQueue    currLevelNodes;


    binaryTreeSizeHint = (binaryTreeSizeHint == 0) ? gkGenericQueueArbitraryInitialSize : binaryTreeSizeHint;
    GenericQueueCreate(&currLevelNodes, sizeof(binaryTreeNode*), binaryTreeSizeHint);
    
    while(!GenericQueueEmpty(&currLevelNodes) ) 
    {
        currLevelSize = GenericQueueSize(&currLevelNodes);

        while(currLevelSize) {
            GenericQueueFront(&currLevelNodes, (void*)&currNode);
            if(currNode->m_left != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&currNode->m_left);
            }
            if(currNode->m_right != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&currNode->m_right);
            }

            binaryTreeNodeDestroy(currNode);
            free((void*)currNode);

            GenericQueuePop(&currLevelNodes);
            --currLevelSize;
        }
    }


    GenericQueueDestroy(&currLevelNodes);
    return;
}


binaryTreeResult_t TREELIB_API binaryTreeDeepCopy(
    binaryTreeNode*  treeIn,
    uint32_t         binaryTreeSize,
    binaryTreeNode** treeOut
) {
    binaryTreeNode const* currNode   = treeIn;
    binaryTreeNode const* leftNode   = treeIn->m_left;
    binaryTreeNode const* rightNode  = treeIn->m_right;
    binaryTreeNode*       nodeBuffer = NULL;
    uint32_t     currLevelSize = 0;
    uint32_t     nodeBufIdx    = 0;
    uint8_t      status        = 0;
    GenericQueue currLevelNodes;


    status = GenericQueueCreate(&currLevelNodes, sizeof(binaryTreeNode*), binaryTreeSize);
    nodeBuffer = (binaryTreeNode*)malloc(sizeof(binaryTreeNode) * binaryTreeSize);
    if(status || nodeBuffer == NULL) {
        /* Either case has happened, so we make sure to free whatever we (if we) allocated. */
        if(!status) {
            GenericQueueDestroy(&currLevelNodes);
        }
        if(nodeBuffer != NULL) {
            free(nodeBuffer);
        }
        return BINARY_TREE_OP_FAILURE;
    }
    
    
    GenericQueuePush(&currLevelNodes, (void*)&currNode);
    while( !GenericQueueEmpty(&currLevelNodes) ) 
    {
        currLevelSize = GenericQueueSize(&currLevelNodes);
        while(currLevelSize) {
            GenericQueueFront(&currLevelNodes, (void*)&currNode);
            leftNode  = currNode->m_left;
            rightNode = currNode->m_right;

            
            if(leftNode != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&leftNode);
            }
            if(rightNode != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&rightNode);
            }

            GenericQueuePop(&currLevelNodes);
            --currLevelSize;
        }
    }


    GenericQueueDestroy(&currLevelNodes);
    return BINARY_TREE_OP_SUCCESS;
}


void TREELIB_API binaryTreeNodeShallowCopy(
    binaryTreeNode* nodeIn,
    binaryTreeNode* nodeOut
) {
    memcpy(nodeOut, nodeIn, sizeof(binaryTreeNode));
    return;
}


void TREELIB_API binaryTreeNodeMove(
    binaryTreeNode* toMoveFrom,
    binaryTreeNode* toMoveTo
) {
    binaryTreeNodeShallowCopy(toMoveFrom, toMoveTo);
    memset(toMoveFrom, 0x00, sizeof(binaryTreeNode));
    return;
}




binaryTreeBool_t binaryTreeNodeIsLeaf(binaryTreeNode const* node) {
    if(node == NULL) {
        return 0;
    }
    return (node->m_left == NULL) && (node->m_right == NULL);
}


binaryTreeBool_t binaryTreeNodeIsSingleNodeParent(binaryTreeNode const* node) {
    if(node == NULL) {
        return 0;
    }
    uint8_t leftEmpty  = (node->m_left  == NULL);
    uint8_t rightEmpty = (node->m_right == NULL);
    return ( leftEmpty && !rightEmpty ) || ( !leftEmpty && rightEmpty );
}


binaryTreeNode* binaryTreeFindMax(binaryTreeNode* node)
{
    binaryTreeNode* search = node;
    while(search != NULL) {
        node = search;
        search = search->m_right;
    }


    return node;
}

binaryTreeNode* binaryTreeFindMin(binaryTreeNode* node)
{
    binaryTreeNode* search = node;
    while(search != NULL) {
        node = search;
        search = search->m_left;
    }


    return node;
}

binaryTreeBool_t binaryTreeIsValidBST(
    binaryTreeNode const*    node, 
    uint32_t                 binaryTreeSizeHint,
    binaryTreeComparatorFunc cmp
) {
    uint8_t         satisfiesCondition = BINARY_TREE_BOOL_TRUE;
    uint8_t         tmpCond            = BINARY_TREE_BOOL_TRUE;
    uint32_t        currLevelSize      = 0;
    binaryTreeNode const* currNode     = node;
    binaryTreeNode const* leftNode     = node->m_left;
    binaryTreeNode const* rightNode    = node->m_right;
    GenericQueue currLevelNodes;


    binaryTreeSizeHint = (binaryTreeSizeHint == 0) ? gkGenericQueueArbitraryInitialSize : binaryTreeSizeHint;
    GenericQueueCreate(&currLevelNodes, sizeof(binaryTreeNode*), binaryTreeSizeHint);
    
    /* 
        Extract the first check Outside the loop because 
        the tree might be very small / doesn't satisfy the condition already 
    */
    if(leftNode != NULL) {
        tmpCond = tmpCond && ( cmp(leftNode->m_data, currNode->m_data) < 0 );
        GenericQueuePush(&currLevelNodes, (void*)&leftNode);
    }
    if(rightNode != NULL) {
        tmpCond = tmpCond && ( cmp(rightNode->m_data, currNode->m_data) > 0 );
        GenericQueuePush(&currLevelNodes, (void*)&rightNode);
    }
    satisfiesCondition = satisfiesCondition && tmpCond;
    
    
    while(satisfiesCondition && !GenericQueueEmpty(&currLevelNodes) ) 
    {
        currLevelSize = GenericQueueSize(&currLevelNodes);

        while(currLevelSize) {
            GenericQueueFront(&currLevelNodes, (void*)&currNode);
            leftNode  = currNode->m_left;
            rightNode = currNode->m_right;

            if(leftNode != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&leftNode);
                tmpCond = tmpCond && ( cmp(leftNode->m_data, currNode->m_data) < 0 );
            }
            if(rightNode != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&rightNode);
                tmpCond = tmpCond && ( cmp(rightNode->m_data, currNode->m_data) > 0 );
            }

            GenericQueuePop(&currLevelNodes);
            --currLevelSize;
        }


        satisfiesCondition = satisfiesCondition && tmpCond;
    }


    GenericQueueDestroy(&currLevelNodes);
    return satisfiesCondition;
}


binaryTreeBool_t binaryTreeSearchValue(
    binaryTreeNode*          node,
    void*                    value,
    binaryTreeComparatorFunc cmp,
    binaryTreeNode**         outNodeIfFound
) {
    binaryTreeNode* search       = node;
    binaryTreeNode* searchParent = NULL;
    int8_t          cmpResult    = 0;


    for(; cmpResult != 0 && search != NULL ;) {
        cmpResult    = cmp(value, search->m_data);
        searchParent = search;
        search       = cmpResult < 0 ? search->m_left : search->m_right;
    }
    cmpResult = (cmpResult == 0) ? 1 : 0;
    
    
    *outNodeIfFound = cmpResult ? searchParent : NULL;
    return cmpResult;
}
