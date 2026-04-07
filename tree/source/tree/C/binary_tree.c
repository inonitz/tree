#include <tree/C/binary_tree.h>
#include <tree/C/queue.h>
#include <tree/C/stack.h>
#include <tree/C/alloc.h>
#include <string.h>


static const uint32_t gkGenericStackArbitraryInitialSize = 1024;
static const uint32_t gkGenericQueueArbitraryInitialSize = 4096;


binaryTreeResult_t binaryTreeNodeCreate(
    binaryTreeNode* rootNode,
    void*           value,
    uint32_t        valueSizeBytes,
    binaryTreeValCopyFunc valueCopyConstructFunc
) {
    memset(rootNode, 0x00, sizeof(binaryTreeNode));


    rootNode->m_data = (void*)treelibMallocBufferExplicit(uint8_t, valueSizeBytes);
    if(rootNode->m_data == NULL) {
        treelibFreeTypeExplicit(rootNode->m_data);
        return BINARY_TREE_OP_FAILURE;
    }

    if(valueCopyConstructFunc == NULL) {
        memcpy(rootNode->m_data, value, valueSizeBytes);
    } else {
        valueCopyConstructFunc(rootNode->m_data, value);
    }
    return BINARY_TREE_OP_SUCCESS;
}


binaryTreeResult_t binaryTreeNodeCreateWithPointers(
    binaryTreeNode* rootNode,
    binaryTreeNode* leftNode,
    binaryTreeNode* rightNode,
    binaryTreeNode* parentNode,
    void*           value,
    uint32_t        valueSizeBytes,
    binaryTreeValCopyFunc valueCopyConstructFunc
) {
    memset(rootNode, 0x00, sizeof(binaryTreeNode));

    rootNode->m_data = (void*)treelibMallocBytesExplicit(uint8_t, valueSizeBytes, 1);
    if(rootNode->m_data == NULL) {
        return BINARY_TREE_OP_FAILURE;
    }

    rootNode->m_left   = leftNode;
    rootNode->m_right  = rightNode;
    rootNode->m_parent = parentNode;

    if(valueCopyConstructFunc == NULL) {
        memcpy(rootNode->m_data, value, valueSizeBytes);
        return BINARY_TREE_OP_SUCCESS;
    }
    return ( 0 != valueCopyConstructFunc(rootNode->m_data, value) ) ? 
        BINARY_TREE_OP_SUCCESS 
        : 
        BINARY_TREE_OP_FAILURE;
}


void binaryTreeNodeDestroy(
    binaryTreeNode* node,
    binaryTreeValDeleteFunc valueDestructorFunctor
) {
    if(node == NULL) {
        return;
    }
    
    
    /* If value is a complex type a destructor must be called */
    if(valueDestructorFunctor != NULL) {
        valueDestructorFunctor(node->m_data);
    }
    /* Now we can free it safely */
    treelibFreeTypeExplicit(node->m_data);
    memset(node, 0x00, sizeof(binaryTreeNode));
    /*
        The node object itself may have been allocated out of
        this scope, and thus its' memory shall be managed externally.
    */
    return;
}


binaryTreeResult_t binaryTreeDestroy(
    binaryTreeNode*         rootNode, 
    binaryTreeValDeleteFunc valueDestructorFunctor,
    uint32_t                binaryTreeSizeHint
) {
    if(rootNode == NULL) {
        return BINARY_TREE_OP_SUCCESS;
    }
    /*
        Iterative Post-Order Traversal Using a single Stack
        We traverse the children first, de-allocate them, and then we come back to the parents.
        Thank you very much to:
        https://medium.com/@amanjain843/iterative-postorder-traversal-of-a-binary-tree-using-a-single-stack-time-o-n-space-o-h-bb037b9ef28
    */
    GenericStack nodeStack;
    binaryTreeNode*    currNode = rootNode;
    binaryTreeNode*    tmpNode  = NULL;
    binaryTreeResult_t status   = BINARY_TREE_OP_SUCCESS;


    binaryTreeSizeHint = (binaryTreeSizeHint == 0) ? 
        gkGenericStackArbitraryInitialSize 
        : 
        binaryTreeSizeHint;
    
    status = GenericStackCreate(&nodeStack, sizeof(binaryTreeNode*), binaryTreeSizeHint);
    if(status) {
        GenericStackDestroy(&nodeStack);
        return status;
    }


    while(currNode != NULL || !GenericStackEmpty(&nodeStack))
    {
        while(currNode != NULL) {
            GenericStackPush(&nodeStack, (void*)&currNode);
            currNode = currNode->m_left;
        }

        GenericStackTop(&nodeStack, (void*)&currNode);
        if(currNode == NULL) {
            GenericStackPop(&nodeStack);

            /* These 4-LOC could be replaced by anything - this is just a tree traversal */
            GenericStackTop(&nodeStack, (void*)&tmpNode);
            binaryTreeNodeDestroy(tmpNode, valueDestructorFunctor);
            treelibFreeTypeExplicit(tmpNode);
            tmpNode = NULL;

            GenericStackPop(&nodeStack);
        }
        else {
            tmpNode = NULL;
            GenericStackPush(&nodeStack, (void*)&tmpNode);
            currNode = currNode->m_right;
        }
    }


    GenericStackDestroy(&nodeStack);
    return status;
}


binaryTreeResult_t binaryTreeDeepCopy(
    binaryTreeNode const* treeIn,
    uint32_t              binaryTreeSize,
    uint32_t              valueSizeBytes,
    binaryTreeValCopyFunc valueCopyConstructorFunctor,
    binaryTreeNode**      treeOut,
    binaryTreeNode**      treeNodeBufferOut
) {
    binaryTreeNode const* currNode  = treeIn;
    binaryTreeNode const* leftNode  = treeIn->m_left;
    binaryTreeNode const* rightNode = treeIn->m_right;
    binaryTreeNode* currCopiedNode  = NULL;
    binaryTreeNode* leftCopiedNode  = NULL;
    binaryTreeNode* rightCopiedNode = NULL;
    binaryTreeNode* nodeBuffer      = NULL;

    uint32_t           currLevelSize = 0;
    uint32_t           nodeBufTopIdx = 0;
    binaryTreeResult_t failStatus[3] = { 
        BINARY_TREE_OP_SUCCESS, 
        BINARY_TREE_OP_SUCCESS, 
        BINARY_TREE_OP_SUCCESS
    };    GenericQueue       currLevelNodes;
    GenericQueue       copiedLevelNodes;


    failStatus[0] = GenericQueueCreate(&currLevelNodes, sizeof(binaryTreeNode*), binaryTreeSize);
    failStatus[1] = GenericQueueCreate(&copiedLevelNodes, sizeof(binaryTreeNode*), binaryTreeSize);
    nodeBuffer = treelibMallocBufferExplicit(binaryTreeNode, binaryTreeSize);
    failStatus[2] = (nodeBuffer == NULL);
    if(failStatus[0] || failStatus[1] || failStatus[2]) {
        GenericQueueDestroy(&currLevelNodes);
        GenericQueueDestroy(&copiedLevelNodes);
        treelibFreeTypeExplicit(nodeBuffer);

        *treeOut = NULL;
        *treeNodeBufferOut = NULL;
        return BINARY_TREE_OP_FAILURE;
    }


    /* Copy the root node */
    currCopiedNode = &nodeBuffer[nodeBufTopIdx];
    ++nodeBufTopIdx;
    failStatus[0] = binaryTreeNodeCreateWithPointers(
        currCopiedNode, 
        NULL, NULL, NULL, 
        currNode->m_data, 
        valueSizeBytes,
        valueCopyConstructorFunctor
    );

    GenericQueuePush(&currLevelNodes,   (void*)&currNode);
    GenericQueuePush(&copiedLevelNodes, (void*)&currCopiedNode);
    while(!failStatus[0] && !GenericQueueEmpty(&currLevelNodes) )
    {
        currLevelSize = GenericQueueSize(&currLevelNodes);
        while(!failStatus[0] && currLevelSize) {
            GenericQueueFront(&currLevelNodes,   (void*)&currNode);
            GenericQueueFront(&copiedLevelNodes, (void*)&currCopiedNode);

            leftNode  = currNode->m_left;
            rightNode = currNode->m_right;

            /* 
                for each child:
                    Allocate a node from nodeBuffer
                    initialize it
                    add it to the copied tree
                    add it to the queue (so we can continue iterating)
            */
            if(leftNode != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&leftNode);

                leftCopiedNode = &nodeBuffer[nodeBufTopIdx];
                ++nodeBufTopIdx;
                failStatus[1] = binaryTreeNodeCreateWithPointers(leftCopiedNode, 
                    NULL, NULL, currCopiedNode, 
                    leftNode->m_data, 
                    valueSizeBytes,
                    valueCopyConstructorFunctor
                );
                currCopiedNode->m_left = leftCopiedNode;

                GenericQueuePush(&copiedLevelNodes, (void*)&leftCopiedNode);
                failStatus[0] = failStatus[0] && failStatus[1];
            }
            if(rightNode != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&rightNode);
                
                rightCopiedNode = &nodeBuffer[nodeBufTopIdx];
                ++nodeBufTopIdx;
                failStatus[1] = binaryTreeNodeCreateWithPointers(rightCopiedNode, 
                    NULL, NULL, currCopiedNode, 
                    rightNode->m_data, 
                    valueSizeBytes,
                    valueCopyConstructorFunctor
                );
                currCopiedNode->m_right = rightCopiedNode;

                GenericQueuePush(&copiedLevelNodes, (void*)&rightCopiedNode);
                failStatus[0] = failStatus[0] && failStatus[1];
            }

            GenericQueuePop(&currLevelNodes);
            GenericQueuePop(&copiedLevelNodes);
            --currLevelSize;
        }
    }


    *treeOut           = &nodeBuffer[0];
    *treeNodeBufferOut = nodeBuffer;
    GenericQueueDestroy(&currLevelNodes);
    GenericQueueDestroy(&copiedLevelNodes);
    return BINARY_TREE_OP_SUCCESS;
}


binaryTreeResult_t binaryTreeDeepCopyNoBuf(
    binaryTreeNode const* treeIn,
    uint32_t              binaryTreeSize,
    uint32_t              valueSizeBytes,
    binaryTreeValCopyFunc valueCopyConstructorFunctor,
    binaryTreeNode**      treeOut
) {
    binaryTreeNode const* currNode  = treeIn;
    binaryTreeNode const* leftNode  = treeIn->m_left;
    binaryTreeNode const* rightNode = treeIn->m_right;
    binaryTreeNode* currCopiedNode  = NULL;
    binaryTreeNode* leftCopiedNode  = NULL;
    binaryTreeNode* rightCopiedNode = NULL;
    binaryTreeNode** allocNodeBuf   = NULL;

    uint32_t           currLevelSize = 0;
    uint32_t           allocNodeBufTopIdx = 0;
    binaryTreeResult_t failStatus[3] = { 
        BINARY_TREE_OP_SUCCESS, 
        BINARY_TREE_OP_SUCCESS, 
        BINARY_TREE_OP_SUCCESS
    };
    GenericQueue       currLevelNodes;
    GenericQueue       copiedLevelNodes;


    failStatus[0] = GenericQueueCreate(&currLevelNodes, sizeof(binaryTreeNode*), binaryTreeSize);
    failStatus[1] = GenericQueueCreate(&copiedLevelNodes, sizeof(binaryTreeNode*), binaryTreeSize);
    allocNodeBuf = treelibMallocBufferExplicit(binaryTreeNode*, binaryTreeSize);
    failStatus[2] = (allocNodeBuf == NULL);
    if(failStatus[0] || failStatus[1] || failStatus[2]) {
        GenericQueueDestroy(&currLevelNodes);
        GenericQueueDestroy(&copiedLevelNodes);
        treelibFreeTypeExplicit(allocNodeBuf);

        *treeOut = NULL;
        return BINARY_TREE_OP_FAILURE;
    }


    /* Copy the root node */
    currCopiedNode = treelibMallocTypeExplicit(binaryTreeNode);
    allocNodeBuf[allocNodeBufTopIdx] = currCopiedNode;
    ++allocNodeBufTopIdx;
    failStatus[0] = binaryTreeNodeCreateWithPointers(
        currCopiedNode, 
        NULL, NULL, NULL, 
        currNode->m_data, 
        valueSizeBytes,
        valueCopyConstructorFunctor
    );

    GenericQueuePush(&currLevelNodes,   (void*)&currNode);
    GenericQueuePush(&copiedLevelNodes, (void*)&currCopiedNode);
    while(!failStatus[0] && !GenericQueueEmpty(&currLevelNodes) )
    {
        currLevelSize = GenericQueueSize(&currLevelNodes);
        while(!failStatus[0] && currLevelSize) {
            GenericQueueFront(&currLevelNodes,   (void*)&currNode);
            GenericQueueFront(&copiedLevelNodes, (void*)&currCopiedNode);

            leftNode  = currNode->m_left;
            rightNode = currNode->m_right;

            /* 
                for each child:
                    Allocate a node from nodeBuffer
                    initialize it
                    add it to the copied tree
                    add it to the queue (so we can continue iterating)
            */
            if(leftNode != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&leftNode);

                leftCopiedNode = treelibMallocTypeExplicit(binaryTreeNode);
                failStatus[0] = (leftCopiedNode == NULL); 
                if(!failStatus[0]) {
                    allocNodeBuf[allocNodeBufTopIdx] = leftCopiedNode;
                    ++allocNodeBufTopIdx;
                    failStatus[1] = binaryTreeNodeCreateWithPointers(
                        leftCopiedNode, 
                        NULL, NULL, currCopiedNode, 
                        leftNode->m_data, 
                        valueSizeBytes,
                        valueCopyConstructorFunctor
                    );

                    currCopiedNode->m_left = leftCopiedNode;
                    GenericQueuePush(&copiedLevelNodes, (void*)&leftCopiedNode);
                    failStatus[0] = failStatus[0] && failStatus[1];
                }
            }
            if(rightNode != NULL) {
                GenericQueuePush(&currLevelNodes, (void*)&rightNode);
                
                rightCopiedNode = treelibMallocTypeExplicit(binaryTreeNode);
                failStatus[0] = (rightCopiedNode == NULL); 
                if(!failStatus[0]) {
                    allocNodeBuf[allocNodeBufTopIdx] = rightCopiedNode;
                    ++allocNodeBufTopIdx;
                    failStatus[1] = binaryTreeNodeCreateWithPointers(
                        rightCopiedNode, 
                        NULL, NULL, currCopiedNode, 
                        rightNode->m_data, 
                        valueSizeBytes,
                        valueCopyConstructorFunctor
                    );

                    currCopiedNode->m_right = rightCopiedNode;
                    GenericQueuePush(&copiedLevelNodes, (void*)&rightCopiedNode);
                    failStatus[0] = failStatus[0] && failStatus[1];
                }
            }


            GenericQueuePop(&currLevelNodes);
            GenericQueuePop(&copiedLevelNodes);
            --currLevelSize;
        }
    }


    if(failStatus[0]) {
        for(uint32_t i = 0; i < allocNodeBufTopIdx; ++i) {
            treelibFreeTypeExplicit(allocNodeBuf[i]);
        }
    }


    *treeOut = failStatus[0] ? NULL : allocNodeBuf[0];
    treelibFreeTypeExplicit(allocNodeBuf);
    GenericQueueDestroy(&currLevelNodes);
    GenericQueueDestroy(&copiedLevelNodes);
    return BINARY_TREE_OP_SUCCESS;
}


void binaryTreeNodeShallowCopy(
    binaryTreeNode* nodeIn,
    binaryTreeNode* nodeOut
) {
    memcpy(nodeOut, nodeIn, sizeof(binaryTreeNode));
    return;
}


void binaryTreeNodeMove(
    binaryTreeNode* toMoveFrom,
    binaryTreeNode* toMoveTo
) {
    binaryTreeNodeShallowCopy(toMoveFrom, toMoveTo);
    memset(toMoveFrom, 0x00, sizeof(binaryTreeNode));
    return;
}




binaryTreeBool_t binaryTreeNodeIsLeaf(binaryTreeNode const* node) {
    if(node == NULL) {
        return BINARY_TREE_BOOL_FALSE;
    }
    return (node->m_left == NULL) && (node->m_right == NULL);
}


binaryTreeBool_t binaryTreeNodeIsSingleNodeParent(binaryTreeNode const* node) {
    if(node == NULL) {
        return BINARY_TREE_BOOL_FALSE;
    }
    uint8_t leftEmpty  = (node->m_left  == NULL);
    uint8_t rightEmpty = (node->m_right == NULL);
    return ( leftEmpty && !rightEmpty ) || ( !leftEmpty && rightEmpty );
}


binaryTreeBool_t binaryTreeNodeIsFull(binaryTreeNode const* node) {
    if(node == NULL) {
        return BINARY_TREE_BOOL_FALSE;
    }
    return (node->m_left != NULL) && (node->m_right != NULL);
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


binaryTreeStatusPair_t binaryTreeCompare(
    binaryTreeNode const*    nodeA,
    binaryTreeNode const*    nodeB,
    uint32_t                 binaryTreeSizeHint,
    binaryTreeComparatorFunc cmp
) {
    binaryTreeStatusPair_t status = (binaryTreeStatusPair_t){
        BINARY_TREE_OP_SUCCESS,
        BINARY_TREE_BOOL_TRUE
    };
    struct TreeContext {
        binaryTreeNode const* currNode; 
        binaryTreeNode const* currLeft; 
        binaryTreeNode const* currRight;
        uint32_t              lvlSize;
        binaryTreeResult_t    qStatus;
        GenericQueue          lvlNodes;
    };
    
    struct TreeContext A = {
        nodeA, nodeA->m_left, nodeA->m_right, 0, BINARY_TREE_OP_SUCCESS, (GenericQueue){ .m_buffer = NULL }
    };
    struct TreeContext B = {
        nodeB, nodeB->m_left, nodeB->m_right, 0, BINARY_TREE_OP_SUCCESS, (GenericQueue){ .m_buffer = NULL }
    };


    binaryTreeSizeHint = (binaryTreeSizeHint == 0) ? 
        gkGenericQueueArbitraryInitialSize 
        : 
        binaryTreeSizeHint;

    A.qStatus = GenericQueueCreate(&A.lvlNodes, sizeof(binaryTreeNode*), binaryTreeSizeHint);
    B.qStatus = GenericQueueCreate(&B.lvlNodes, sizeof(binaryTreeNode*), binaryTreeSizeHint);
    status.m_op = A.qStatus || B.qStatus;
    if(status.m_op == BINARY_TREE_OP_FAILURE) {
        GenericQueueDestroy(&A.lvlNodes);
        GenericQueueDestroy(&B.lvlNodes);
        status.m_bool = BINARY_TREE_BOOL_FALSE;
        return status;
    }


    A.qStatus = GenericQueuePush(&A.lvlNodes, (void*)&A.currNode);
    B.qStatus = GenericQueuePush(&B.lvlNodes, (void*)&B.currNode);
    status.m_op = A.qStatus || B.qStatus;
    while(status.m_op != BINARY_TREE_OP_FAILURE
        && status.m_bool == BINARY_TREE_BOOL_TRUE
        && !GenericQueueEmpty(&A.lvlNodes) 
        && !GenericQueueEmpty(&B.lvlNodes)
    ) {
        A.lvlSize = GenericQueueSize(&A.lvlNodes);
        B.lvlSize = GenericQueueSize(&B.lvlNodes);
        status.m_bool = (A.lvlSize == B.lvlSize);

        while(status.m_op != BINARY_TREE_OP_FAILURE 
            && status.m_bool == BINARY_TREE_BOOL_TRUE 
            && A.lvlSize 
            && B.lvlSize
        ) {
            GenericQueueFront(&A.lvlNodes, (void*)&A.currNode);
            GenericQueueFront(&B.lvlNodes, (void*)&B.currNode);

            /* Iterating Over Tree A */
            A.currLeft  = A.currNode->m_left;
            A.currRight = A.currNode->m_right;
            if(A.currLeft  != NULL) {
                status.m_op = status.m_op || GenericQueuePush(&A.lvlNodes, (void*)&A.currLeft);
            }
            if(A.currRight != NULL) {
                status.m_op = status.m_op || GenericQueuePush(&A.lvlNodes, (void*)&A.currRight);
            }
            GenericQueuePop(&A.lvlNodes);
            --A.lvlSize;


            /* Iterating Over Tree B */
            B.currLeft  = B.currNode->m_left;
            B.currRight = B.currNode->m_right;
            if(B.currLeft  != NULL) {
                status.m_op = status.m_op || GenericQueuePush(&B.lvlNodes, (void*)&B.currLeft);
            }
            if(B.currRight != NULL) {
                status.m_op = status.m_op || GenericQueuePush(&B.lvlNodes, (void*)&B.currRight);
            }
            GenericQueuePop(&B.lvlNodes);
            --B.lvlSize;


            status.m_bool = status.m_bool && ( cmp(A.currNode->m_data, B.currNode->m_data) == 0 );
        }
    }


    GenericQueueDestroy(&A.lvlNodes);
    GenericQueueDestroy(&B.lvlNodes);
    return status;
}


binaryTreeStatusPair_t binaryTreeIsValidBST(
    binaryTreeNode const*    node,
    uint32_t                 binaryTreeSizeHint,
    binaryTreeComparatorFunc cmp
) {
    binaryTreeStatusPair_t status = (binaryTreeStatusPair_t){
        BINARY_TREE_OP_SUCCESS,
        BINARY_TREE_BOOL_TRUE
    };
    GenericStack          nodeStack;
    binaryTreeNode const* currNode = node;
    void*                 prevData = NULL;
    if(node == NULL) {
        return status;
    }

    binaryTreeSizeHint = (binaryTreeSizeHint == 0) ? 
        gkGenericStackArbitraryInitialSize 
        : 
        binaryTreeSizeHint;

    status.m_op = GenericStackCreate(&nodeStack, sizeof(binaryTreeNode const*), binaryTreeSizeHint);
    if(status.m_op == BINARY_TREE_OP_FAILURE) {
        goto __cleanup;
    }

    /* Iterative In-Order Traversal */
    while (currNode != NULL || !GenericStackEmpty(&nodeStack)) 
    {
        while (currNode != NULL) {
            status.m_op = GenericStackPush(&nodeStack, (void*)&currNode);
            if(status.m_op == BINARY_TREE_OP_FAILURE) {
                goto __cleanup;
            }
            currNode = currNode->m_left;
        }

        GenericStackTop(&nodeStack, (void*)&currNode);
        GenericStackPop(&nodeStack);

        /* Elements must be strictly ascending */
        if (prevData != NULL) {
            if (cmp(prevData, currNode->m_data) >= 0) {
                status.m_bool = BINARY_TREE_BOOL_FALSE;
                break; /* Exit early, the tree is invalid */
            }
        }
        
        prevData = currNode->m_data;
        currNode = currNode->m_right;
    }


__cleanup:
    GenericStackDestroy(&nodeStack);
    return status;
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
    return (uint8_t)cmpResult;
}
