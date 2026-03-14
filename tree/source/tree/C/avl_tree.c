#include <tree/C/avl_tree.h>
#include <tree/C/binary_tree.h>
#include <tree/C/stack.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <util2/C/debugbreak.h>


#ifndef mallocTypeExplicit
#   define mallocTypeExplicit(Type) \
        (  (Type*)(malloc(sizeof(Type)))  )
#endif /* mallocTypeExplicit */

#ifndef freeTypeExplicit
#   define freeTypeExplicit(Pointer) \
        (  free( (void*)Pointer )  )
#endif /* freeTypeExplicit */


enum AVLTreeRotation {
    AVL_TREE_ROTATION_NONE       = 0,
    AVL_TREE_ROTATION_LEFTLEFT   = 1,
    AVL_TREE_ROTATION_RIGHTRIGHT = 2,
    AVL_TREE_ROTATION_LEFTRIGHT  = 3,
    AVL_TREE_ROTATION_RIGHTLEFT  = 4
};


int8_t AVLTreeComputeHeight(binaryTreeNode* node);
int8_t AVLTreeComputeBalance(binaryTreeNode* node);
binaryTreeNode* rotateLeft (binaryTreeNode* node);
binaryTreeNode* rotateRight(binaryTreeNode* node);
void AVLTreeMaybeRebalance(
    binaryTreeNode* node, 
    binaryTreeNode** maybeNewRootAddr
);
void removeNodeAndLinkParentWithChild(binaryTreeNode* nodeToModify);
binaryTreeNode* AVLTreeFindMaxAndPushParents(
    binaryTreeNode* node, 
    GenericStack*   parentQueue
);
binaryTreeNode* AVLTreeFindMinAndPushParents(
    binaryTreeNode* node, 
    GenericStack*   parentQueue
);
void AVLTreeContextPrint(binaryTreePrintCtx* ctx, const char* format, ...);




void AVLTreeCreate(
    AVLTree*                 root,
    binaryTreeComparatorFunc valueCompare,
    uint32_t                 valueSizeInBytes
) {
    root->m_root          = NULL;
    root->m_cmp           = valueCompare;
    root->m_nodeCount     = 0;
    root->m_dataSizeBytes = valueSizeInBytes;
    return;
}


void AVLTreeDestroy(AVLTree* root)
{
    binaryTreeDestroy(root->m_root, root->m_nodeCount);
    root->m_root          = NULL;
    root->m_cmp           = NULL;
    root->m_nodeCount     = 0;
    root->m_dataSizeBytes = 0;
    return;
}


binaryTreeResult_t AVLTreeInsert(AVLTree* root, void* value)
{
    binaryTreeBool_t inserted    = BINARY_TREE_BOOL_FALSE;
    binaryTreeBool_t foundNode   = BINARY_TREE_BOOL_FALSE;
    int8_t          cmpResult    = 0;
    int8_t          bfLeft       = 0;
    int8_t          bfRight      = 0;
    binaryTreeNode* search       = NULL;
    binaryTreeNode* maybeNewRoot = NULL;
    binaryTreeNode* allocNode    = NULL;
    binaryTreeNode* currParent   = NULL;
    GenericStack    nodesTouched;


    /* Initial Tree Empty case */
    if(root->m_root == NULL) {
        
        root->m_root = mallocTypeExplicit(binaryTreeNode);
        binaryTreeNodeCreate(root->m_root, value, root->m_dataSizeBytes);
        ++root->m_nodeCount;

        return BINARY_TREE_OP_SUCCESS;
    }


    /* Tree isn't empty, we may find the node in the tree */
    GenericStackCreate(&nodesTouched, sizeof(binaryTreeNode*), 2 * AVLTreeHeight(root));
    for(search = root->m_root; !foundNode && search != NULL; ) {
        GenericStackPush(&nodesTouched, (void*)&search);
        
        cmpResult = root->m_cmp(value, search->m_data);
        foundNode = (cmpResult == 0);
        search    = cmpResult < 0 ? search->m_left : search->m_right;
    }


    if(foundNode) {
        GenericStackDestroy(&nodesTouched);
        return BINARY_TREE_OP_FAILURE;
    }


    allocNode = mallocTypeExplicit(binaryTreeNode);
    binaryTreeNodeCreate(allocNode, value, root->m_dataSizeBytes);

    for(; !GenericStackEmpty(&nodesTouched) ;) {
        
        GenericStackTop(&nodesTouched, (void*)&currParent);
        /* 
            If there was a rebalance, maybeNewRoot would change to the correct root
            Otherwise, maybeNewRoot is just the old root, and thus can be returned as normal.
        */
        maybeNewRoot = currParent;


        if(!inserted) {
            cmpResult = root->m_cmp(value, currParent->m_data);

            if(cmpResult < 0)
                currParent->m_left = allocNode;
            else
                currParent->m_right = allocNode;
            
            allocNode->m_parent = currParent;
            inserted = BINARY_TREE_BOOL_TRUE;
            ++root->m_nodeCount;
        }

        AVLTreeMaybeRebalance(currParent, &maybeNewRoot);
        GenericStackPop(&nodesTouched);
    }


    root->m_root = maybeNewRoot;
    return BINARY_TREE_OP_SUCCESS;
}


binaryTreeResult_t AVLTreeRemove(AVLTree* root, void* value)
{
    uint8_t         foundNode = BINARY_TREE_BOOL_FALSE;
    int8_t          cmpResult = 0;
    binaryTreeNode* currNode  = root->m_root;
    binaryTreeNode* childNodeIfSingleParent = NULL;
    GenericStack    nodesTouched;
    
    uint8_t         deletedNode  = BINARY_TREE_BOOL_FALSE;
    binaryTreeNode* maybeNewRoot = NULL;
    binaryTreeNode* currParent   = NULL;
    binaryTreeNode  deletedNodeCopy;


    /* Step 1. Check for Edge-Cases that can be easily handled */
    if(currNode == NULL) {
        return BINARY_TREE_OP_FAILURE;
    }


    if(binaryTreeNodeIsLeaf(currNode)
        && 
        root->m_cmp(value, currNode->m_data) == 0
    ) {
        binaryTreeNodeDestroy(currNode);
        free(currNode);
        root->m_root      = NULL;
        root->m_nodeCount = 0;
        return BINARY_TREE_OP_SUCCESS;
    }


    childNodeIfSingleParent = currNode->m_left ? currNode->m_left : currNode->m_right;
    if(binaryTreeNodeIsSingleNodeParent(currNode) 
        && root->m_cmp(value, currNode->m_data) == 0
    ) { 
        binaryTreeNodeDestroy(currNode);
        free(currNode);
        
        childNodeIfSingleParent->m_parent = NULL;
        root->m_root = childNodeIfSingleParent;
        --root->m_nodeCount;
        return BINARY_TREE_OP_SUCCESS;
    }


    if(binaryTreeNodeIsSingleNodeParent(currNode) 
        && root->m_cmp(value, childNodeIfSingleParent->m_data) == 0
    ) { 
        binaryTreeNodeDestroy(childNodeIfSingleParent);
        free(childNodeIfSingleParent);
        
        currNode->m_left  = (childNodeIfSingleParent == currNode->m_left ) ? 
            NULL : currNode->m_left;
        currNode->m_right = (childNodeIfSingleParent == currNode->m_right) ? 
            NULL : currNode->m_right;
        --root->m_nodeCount;
        return BINARY_TREE_OP_SUCCESS;
    }


    GenericStackCreate(&nodesTouched, sizeof(binaryTreeNode*), 2 * AVLTreeHeight(root));
    for(; !foundNode && currNode != NULL; ) {
        GenericStackPush(&nodesTouched, (void*)&currNode);
        
        cmpResult = root->m_cmp(value, currNode->m_data);
        foundNode = (cmpResult == 0);
        currNode  = cmpResult < 0 ? currNode->m_left : currNode->m_right;
    }

    if(!foundNode) { /* we didn't find value in the tree, so we fail and exit. */
        GenericStackDestroy(&nodesTouched);
        return BINARY_TREE_OP_FAILURE;
    }


    /* GenericStackTop(&nodesTouched) will return the element to be deleted, if found == true */
    GenericStackTop(&nodesTouched, &childNodeIfSingleParent);
    binaryTreeNodeShallowCopy(childNodeIfSingleParent, &deletedNodeCopy);
    for(; !GenericStackEmpty(&nodesTouched) ;) 
    {
        /* 
            On Rebalance, the last element of the stack (maybeNewRoot) will be the new root.
            Otherwise, the last element is just the old root, and nothing will change.
        */
        GenericStackTop(&nodesTouched, &currParent);
        maybeNewRoot = currParent;


        /* currentParent is the node to be deleted ; may enter twice if the node has 2 children. */
        if(!deletedNode) {
            binaryTreeNode* parent   = currParent->m_parent;
            binaryTreeNode* getChild = NULL;

            if(binaryTreeNodeIsLeaf(currParent)) {
                parent->m_left  = (parent->m_left  == currParent) ? getChild : parent->m_left;
                parent->m_right = (parent->m_right == currParent) ? getChild : parent->m_right;
                
                binaryTreeNodeDestroy(currParent);
                free(currParent);
                --root->m_nodeCount;
                deletedNode = BINARY_TREE_BOOL_TRUE;
            } 
            else if(binaryTreeNodeIsSingleNodeParent(currParent)) {
                getChild = currParent->m_left ? currParent->m_left : currParent->m_right;
                getChild->m_parent = parent;
                parent->m_left  = (parent->m_left  == currParent) ? getChild : parent->m_left;
                parent->m_right = (parent->m_right == currParent) ? getChild : parent->m_right;
                
                binaryTreeNodeDestroy(currParent);
                free(currParent);
                --root->m_nodeCount;
                deletedNode = BINARY_TREE_BOOL_TRUE;
            }
            else {
                /* 
                    Instead of doing the hard work
                        1. Find the successorNode in the currentParent subtree
                        2. delete(currentParent)
                        3. dealing with all of the pointer shenanigans required for a 2 child tree
                            (Which may also have subtrees, not to mention the parent nodes...)
                    
                    I'll instead do the following:
                        1. Find the successorNode in the currentParent subtree.
                        2. swap_values(successorNode, currentParent)
                        3. delete(successorNode)
                            -> deleting the successor will (by design) not get into this else-statement
                                ever again, because it'll be either: singleChildParent OR leafNode
                            
                            -> This works by just pushing the parents of the successorNode to the queue,
                                and making sure to update all of their heights/balances' accordingly.
                */
                /* 1. Find Successor */
                binaryTreeNode* successorNode = NULL;
                if(currParent->m_left->m_height < currParent->m_right->m_height) { /* search in the smaller subtree */
                    successorNode = AVLTreeFindMaxAndPushParents(currParent->m_left, &nodesTouched);
                } else {
                    successorNode = AVLTreeFindMinAndPushParents(currParent->m_right, &nodesTouched);
                }

                /* 2. Swap Values */
                currParent->m_data = successorNode->m_data;
                successorNode->m_data = deletedNodeCopy.m_data;
                /* 
                    3. delete(successorNode) 
                    This will happen in the next iteration.
                    Because GenericStackTop(&nodesTouched) is the successorNode currently, we don't want to pop it just yet
                    Therefore, we'll continue to the next iteration.
                */
                continue;
            }


            /* if a node was deleted we do not want to update any of its values. */
            /* i.e We don't want to call AVLTreeMaybeRebalance */
            GenericStackPop(&nodesTouched);
            continue;
        }


        AVLTreeMaybeRebalance(currParent, &maybeNewRoot);
        GenericStackPop(&nodesTouched);
    }


    GenericStackDestroy(&nodesTouched);
    return BINARY_TREE_OP_SUCCESS;
}


binaryTreeResult_t AVLTreeRemove2(AVLTree* root, void* value)
{
    uint8_t         foundNode    = BINARY_TREE_BOOL_FALSE;
    int8_t          cmpResult    = 0;
    binaryTreeNode* currNode     = root->m_root;
    binaryTreeNode* toDelete     = NULL;
    binaryTreeNode* maybeNewRoot = NULL;
    GenericStack    nodesTouched;


    GenericStackCreate(&nodesTouched, sizeof(binaryTreeNode*), 2 * AVLTreeHeight(root));
    for(; !foundNode && currNode != NULL; ) 
    {
        GenericStackPush(&nodesTouched, (void*)&currNode);
        
        cmpResult = root->m_cmp(value, currNode->m_data);
        foundNode = (cmpResult == 0);
        currNode  = cmpResult < 0 ? currNode->m_left : currNode->m_right;
    }
    if(!foundNode) {
        GenericStackDestroy(&nodesTouched);
        return BINARY_TREE_OP_FAILURE;
    }


    /* GenericStackTop() is the node to be deleted.  */
    GenericStackTop(&nodesTouched, &currNode);
    uint8_t fullNode = binaryTreeNodeIsFull(currNode);
    toDelete = currNode;
    if(fullNode)
    {
        binaryTreeNode* successorNode = NULL;
        void*           tmp           = NULL;
        /* 
            search in the smaller subtree 
            the successerNode along with its parents will be pushed to the stack
            GenericStackTop() -> successorNode
        */
        if(currNode->m_left->m_height < currNode->m_right->m_height) {
            successorNode = AVLTreeFindMaxAndPushParents(currNode->m_left, &nodesTouched);
        } else {
            successorNode = AVLTreeFindMinAndPushParents(currNode->m_right, &nodesTouched);
        }
        
        tmp                   = currNode->m_data;
        currNode->m_data      = successorNode->m_data;
        successorNode->m_data = tmp;
        /* Now successorNode contains the value to be deleted, we unlink it in the next step */
        // currNode = successorNode;
        toDelete = successorNode;
    }
    util2_debugbreakif(currNode->m_data == (binaryTreeNode*)0xfeeefeeefeeefeee);
    removeNodeAndLinkParentWithChild(toDelete); /* Needs to happen whether the node is full/not */
    GenericStackPop(&nodesTouched);             /* Pop the value we just deleted from the tree. */


    /* Check for rebalance/root changes */
    for(; !GenericStackEmpty(&nodesTouched) ;) 
    {
        GenericStackTop(&nodesTouched, &currNode);
        util2_debugbreakif(currNode->m_data == (binaryTreeNode*)0xfeeefeeefeeefeee);
        maybeNewRoot = currNode;

        
        AVLTreeMaybeRebalance(currNode, &maybeNewRoot);
        GenericStackPop(&nodesTouched);
    }
    

    root->m_root = maybeNewRoot;
    --root->m_nodeCount;
    GenericStackDestroy(&nodesTouched);
    return BINARY_TREE_OP_SUCCESS;
}


binaryTreeBool_t AVLTreeSearch(AVLTree const* root, void* value)
{
    if(AVLTreeEmpty(root)) {
        return BINARY_TREE_BOOL_FALSE;
    }


    binaryTreeNode* search       = NULL;
    int8_t          cmpResult    = -1;
    for(search = root->m_root; search != NULL && (cmpResult != 0); ) {
        cmpResult = root->m_cmp(value, search->m_data);
        search    = cmpResult < 0 ? search->m_left : search->m_right;
    }
    
    return cmpResult == 0 ? BINARY_TREE_BOOL_TRUE : BINARY_TREE_BOOL_FALSE;
}


binaryTreeBool_t AVLTreeIsValidBST(AVLTree const* root)
{
    uint8_t               satisfiesCondition = 1;
    uint8_t               tmpCond = 0;
    GenericStack          nodeStack;
    const binaryTreeNode* currNode = root->m_root;


    /* Iterative Reverse-In-Order Tree Traversal (Visit right, then root, then left) */
    GenericStackCreate(&nodeStack, sizeof(binaryTreeNode*), root->m_nodeCount);
    while (  satisfiesCondition && ( currNode != NULL || !GenericStackEmpty(&nodeStack) )  ) 
    {
        while (currNode != NULL) {
            GenericStackPush(&nodeStack, (void*)&currNode);
            currNode = currNode->m_right;
        }

        GenericStackTop(&nodeStack, (void*)&currNode);
        GenericStackPop(&nodeStack);

        tmpCond = 
            (currNode->m_balance ==  0) || 
            (currNode->m_balance == -1) || 
            (currNode->m_balance == +1);
        satisfiesCondition = satisfiesCondition && tmpCond;

        currNode = currNode->m_left;
    }


    return satisfiesCondition ? BINARY_TREE_BOOL_TRUE : BINARY_TREE_BOOL_FALSE;
}


binaryTreeBool_t AVLTreeIsBalanced(AVLTree const* root)
{
    if(!binaryTreeIsValidBST(root->m_root, root->m_nodeCount, root->m_cmp)) {
        return BINARY_TREE_BOOL_FALSE;
    }


    uint8_t               satisfiesCondition = 1;
    uint8_t               tmpCond = 0;
    GenericStack          nodeStack;
    const binaryTreeNode* currNode = root->m_root;


    /* Iterative Reverse-In-Order Tree Traversal */
    GenericStackCreate(&nodeStack, sizeof(binaryTreeNode*), root->m_nodeCount);
    while (  satisfiesCondition && ( currNode != NULL || !GenericStackEmpty(&nodeStack) )  ) 
    {
        while (currNode != NULL) {
            GenericStackPush(&nodeStack, (void*)&currNode);
            currNode = currNode->m_right;
        }

        GenericStackTop(&nodeStack, (void*)&currNode);
        GenericStackPop(&nodeStack);

        tmpCond = 
            (currNode->m_balance ==  0) || 
            (currNode->m_balance == -1) || 
            (currNode->m_balance == +1);
        satisfiesCondition = satisfiesCondition && tmpCond;

        currNode = currNode->m_left;
    }


    GenericStackDestroy(&nodeStack);
    return satisfiesCondition ? BINARY_TREE_BOOL_TRUE : BINARY_TREE_BOOL_FALSE;
}


binaryTreeBool_t AVLTreeEmpty(AVLTree const* root)
{
    return root->m_nodeCount == 0 ? BINARY_TREE_BOOL_TRUE : BINARY_TREE_BOOL_FALSE;
}


uint32_t AVLTreeSize(AVLTree const* root)
{
    return root->m_nodeCount;
}


int8_t AVLTreeHeight(AVLTree const* root)
{
    int8_t result = -1;
    
    result = !AVLTreeEmpty(root) ? (int8_t)root->m_root->m_height : result;
    return result;
}


// void AVLTreePrint(AVLTree const* root, void* filePointer)
// {
//     typedef struct PointerSpacePair {
//         binaryTreeNode const* m_ptr;
//         uint32_t              m_space;
//     } NodeSpacing_t;

//     static const uint32_t kSpaceCount  = 8;
//     NodeSpacing_t         tmpNode      = {};
//     uint32_t              currentSpace = 0;
//     GenericStack          nodeStack;
//     const binaryTreeNode* currNode = root->m_root;


//     (void)fprintf( (FILE*)filePointer, "\n-------------------------------- AVLTreePrintBegin() --------------------------------\n");
//     if(AVLTreeEmpty(root)) {
//         (void)fprintf( (FILE*)filePointer, "\nNULL (0, 0)\n");
//         return;
//     }

//     /* Iterative Reverse-In-Order Tree Traversal */
//     GenericStackCreate(&nodeStack, sizeof(NodeSpacing_t), root->m_nodeCount);
//     while (currNode != NULL || !GenericStackEmpty(&nodeStack)) 
//     {
//         while (currNode != NULL) {
//             currentSpace += kSpaceCount;
//             tmpNode = (NodeSpacing_t){ currNode, currentSpace };

//             GenericStackPush(&nodeStack, (void*)&tmpNode);
//             currNode = currNode->m_right;
//         }

//         GenericStackTop(&nodeStack, &tmpNode);
//         currentSpace = tmpNode.m_space;
//         GenericStackPop(&nodeStack);

//         (void)fprintf( (FILE*)filePointer, "\n%*s 0x%" PRIx64  " (%u, %d)\n", 
//             tmpNode.m_space - kSpaceCount, "",
//             (uintptr_t)tmpNode.m_ptr->m_data,
//             (uint32_t)tmpNode.m_ptr->m_height,
//             (int32_t)tmpNode.m_ptr->m_balance
//         );

//         currNode = tmpNode.m_ptr->m_left;
//     }


//     (void)fprintf( (FILE*)filePointer, "\n--------------------------------  AVLTreePrintEnd()  --------------------------------\n");
//     GenericStackDestroy(&nodeStack);
//     return;
// }


void AVLTreePrint(
    AVLTree const* root, 
    void*          outputTargetPtr, 
    uint8_t        isBuffer, 
    uint64_t       bufferSize,
    binaryTreeNodeDataPrinterFunc dataPrinter
) {
    typedef struct PointerSpacePair {
        binaryTreeNode const* m_ptr;
        uint32_t              m_space;
    } NodeSpacing_t;


    static const uint32_t kSpaceCount  = 8;
    NodeSpacing_t         tmpNode      = {};
    uint32_t              currentSpace = 0;
    GenericStack          nodeStack;
    const binaryTreeNode* currNode = root->m_root;
    binaryTreePrintCtx    ctx = { outputTargetPtr, bufferSize, 0 };


    AVLTreeContextPrint(&ctx, "\n--- AVLTreePrintBegin() ---\n");
    if(AVLTreeEmpty(root)) {
        AVLTreeContextPrint(&ctx, "NULL (0, 0)\n---  AVLTreePrintEnd()  ---\n");
        return;
    }


    GenericStackCreate(&nodeStack, sizeof(NodeSpacing_t), root->m_nodeCount);
    while (currNode != NULL || !GenericStackEmpty(&nodeStack)) 
    {
        while (currNode != NULL) {
            currentSpace += kSpaceCount;
            tmpNode = (NodeSpacing_t){ currNode, currentSpace };
            GenericStackPush(&nodeStack, (void*)&tmpNode);
            currNode = currNode->m_right;
        }

        GenericStackTop(&nodeStack, &tmpNode);
        currentSpace = tmpNode.m_space;
        GenericStackPop(&nodeStack);


        /* Actual Printing Begin */
        AVLTreeContextPrint(&ctx, "\n%*s ", tmpNode.m_space - kSpaceCount, "");

        if(dataPrinter != NULL) {
            dataPrinter(&ctx, tmpNode.m_ptr->m_data);
        } else {
            AVLTreeContextPrint(&ctx, "0x%" PRIx64, (uintptr_t)tmpNode.m_ptr->m_data);
        }
        
        AVLTreeContextPrint(&ctx, " (%u, %d)\n", 
            (uint32_t)tmpNode.m_ptr->m_height,
            (int32_t)tmpNode.m_ptr->m_balance
        );
        /* Actual Printing End */
        
        currNode = tmpNode.m_ptr->m_left;
    }


    AVLTreeContextPrint(&ctx, "\n---  AVLTreePrintEnd()  ---\n");
    GenericStackDestroy(&nodeStack);
    return;
}




int8_t AVLTreeComputeHeight(binaryTreeNode* node) {
    if(node == NULL) {
        return -1;
    }
    int8_t rh  = node->m_right ? node->m_right->m_height : -1;
    int8_t lh  = node->m_left  ? node->m_left->m_height  : -1;
    int8_t res = lh < rh ? rh : lh;
    return 1 + res;
}

int8_t AVLTreeComputeBalance(binaryTreeNode* node) {
    if(node == NULL) {
        return 0;
    }
    int16_t rh = node->m_right ? node->m_right->m_height : -1;
    int16_t lh = node->m_left  ? node->m_left->m_height  : -1;
    return rh - lh;
}


/* 
    Before the Rotation:
        X (Root)
        / \
    Z   Y
        / \
        B   C
    After The Rotation:
        Y (New Root)
        / \
        X   C
        / \
    Z   B
*/
binaryTreeNode* rotateLeft(binaryTreeNode* node)
{
    binaryTreeNode* root_parent = node->m_parent;
    binaryTreeNode* x = node;
    binaryTreeNode* y = node->m_right;
    binaryTreeNode* b = y->m_left;

    x->m_right = b;
    y->m_left = x;

    y->m_parent = root_parent;
    x->m_parent = y;
    if(b) {
        b->m_parent = x;
    }
    if(root_parent) {
        /* swap X for Y on the upper-parent level */
        root_parent->m_left  = (root_parent->m_left  == x) ? y : root_parent->m_left;
        root_parent->m_right = (root_parent->m_right == x) ? y : root_parent->m_right;
    }

    x->m_height  = AVLTreeComputeHeight(x);
    x->m_balance = AVLTreeComputeBalance(x);
    y->m_height  = AVLTreeComputeHeight(y);
    y->m_balance = AVLTreeComputeBalance(y);
    return y;
}


/* 
    Before The Rotation:
        Y (Old Root)
        / \
        X   C
        / \
    Z   B

    After the Rotation:
        X (Root)
        / \
    Z   Y
        / \
        B   C
*/
binaryTreeNode* rotateRight(binaryTreeNode* node)
{
    binaryTreeNode* root_parent = node->m_parent;
    binaryTreeNode* y = node;
    binaryTreeNode* x = y->m_left;
    binaryTreeNode* b = x->m_right;

    y->m_left = b;
    x->m_right = y;

    x->m_parent = root_parent;
    y->m_parent = x;
    if(b) {
        b->m_parent = y;
    }
    if(root_parent) {
        /* swap Y for X on the upper-parent level */
        root_parent->m_left  = (root_parent->m_left  == y) ? x : root_parent->m_left;
        root_parent->m_right = (root_parent->m_right == y) ? x : root_parent->m_right;
    }

    y->m_height  = AVLTreeComputeHeight(y);
    y->m_balance = AVLTreeComputeBalance(y);
    x->m_height  = AVLTreeComputeHeight(x);
    x->m_balance = AVLTreeComputeBalance(x);
    return x;
}


void AVLTreeMaybeRebalance(
    binaryTreeNode*  node, 
    binaryTreeNode** maybeNewRootAddr
) {
    int8_t bfRight, bfLeft;
    enum AVLTreeRotation state = AVL_TREE_ROTATION_NONE;


    node->m_height  = AVLTreeComputeHeight(node);
    node->m_balance = AVLTreeComputeBalance(node);
    bfRight = AVLTreeComputeBalance(node->m_right);
    bfLeft  = AVLTreeComputeBalance(node->m_left);
    state = (node->m_balance == -2 && bfLeft  <= 0) ? AVL_TREE_ROTATION_LEFTLEFT   : state;
    state = (node->m_balance == -2 && bfLeft  >  0) ? AVL_TREE_ROTATION_LEFTRIGHT  : state;
    state = (node->m_balance == +2 && bfRight >= 0) ? AVL_TREE_ROTATION_RIGHTRIGHT : state;
    state = (node->m_balance == +2 && bfRight <  0) ? AVL_TREE_ROTATION_RIGHTLEFT  : state;
    switch(state) {
        case AVL_TREE_ROTATION_LEFTLEFT:
        *maybeNewRootAddr = rotateRight(node);
        break;
        
        case AVL_TREE_ROTATION_LEFTRIGHT:
        node->m_left = rotateLeft(node->m_left);
        *maybeNewRootAddr = rotateRight(node);
        break;

        case AVL_TREE_ROTATION_RIGHTRIGHT:
        *maybeNewRootAddr = rotateLeft(node);
        break;

        case AVL_TREE_ROTATION_RIGHTLEFT:
        node->m_right = rotateRight(node->m_right);
        *maybeNewRootAddr = rotateLeft(node);
        break;

        case AVL_TREE_ROTATION_NONE:
        default:
        break;
    }


    return;
}


void removeNodeAndLinkParentWithChild(binaryTreeNode* nodeToModify) 
{
    binaryTreeNode* parentNode = nodeToModify->m_parent;
    binaryTreeNode* childNode  = nodeToModify->m_left ? 
        nodeToModify->m_left 
        : 
        nodeToModify->m_right ? nodeToModify->m_right : NULL;
    

    if(childNode != NULL) {
        childNode->m_parent = parentNode;
    }
    if(parentNode != NULL) {
        parentNode->m_left  = (parentNode->m_left  == nodeToModify) ? childNode : parentNode->m_left;
        parentNode->m_right = (parentNode->m_right == nodeToModify) ? childNode : parentNode->m_right;
    }
    binaryTreeNodeDestroy(nodeToModify);
    free(nodeToModify);
    return;
}


binaryTreeNode* AVLTreeFindMaxAndPushParents(
    binaryTreeNode* node, 
    GenericStack*   parentQueue
) {
    binaryTreeNode* search = node;
    while(search != NULL) {
        node = search;
        GenericStackPush(parentQueue, node);
        search = search->m_right;
    }
    return node;
}


binaryTreeNode* AVLTreeFindMinAndPushParents(
    binaryTreeNode* node, 
    GenericStack*   parentQueue
) {
    binaryTreeNode* search = node;
    while(search != NULL) {
        node = search;
        GenericStackPush(parentQueue, node);
        search = search->m_left;
    }
    return node;
}


void AVLTreeContextPrint(binaryTreePrintCtx* ctx, const char* format, ...) 
{
    va_list args;
    va_start(args, format);


    if(ctx->m_buf == NULL || (ctx->m_bufSize != 0 && ctx->m_bufOffset >= ctx->m_bufSize)) {
        return;
    }
    if(ctx->m_bufSize == 0) {
        vfprintf((FILE*)ctx->m_buf, format, args);
        return;
    }


    int written = vsnprintf(
        (char*)ctx->m_buf + ctx->m_bufOffset, 
        ctx->m_bufSize - ctx->m_bufOffset, 
        format, 
        args
    );
    ctx->m_bufOffset += (written > 0) ? written : 0;
    
    va_end(args);
    return;
}