#include "binaryTree.hpp"
#include <queue>


uint8_t binaryTree::computeHeight(binaryTree* node) noexcept
{
    if(node == nullptr) {
        return -1;
    }
    auto rh = node->m_right ? node->m_right->m_height : -1;
    auto lh = node->m_left  ? node->m_left->m_height  : -1;
    return 1 + std::max(rh, lh);
}

int8_t binaryTree::computeBalanceFactor(binaryTree* node) noexcept
{
    if(node == nullptr) {
        return 0;
    }
    int64_t rh = node->m_right ? node->m_right->m_height : -1;
    int64_t lh = node->m_left  ? node->m_left->m_height  : -1;
    return rh - lh;
}

binaryTree* binaryTree::findMaxAndPushParents(binaryTree* node, std::deque<binaryTree*>& parentQueue)
{
    auto* search = node;
    while(search != nullptr) {
        node = search;
        parentQueue.push_back(node);
        search = search->m_right;
    }
    return node;
}

binaryTree* binaryTree::findMinAndPushParents(binaryTree* node, std::deque<binaryTree*>& parentQueue)
{
    auto* search = node;
    while(search != nullptr) {
        node = search;
        parentQueue.push_back(node);
        search = search->m_left;
    }
    return node;
}

bool binaryTree::isValidAVL_InternalRecursive(binaryTree* node)
{
    if(node == nullptr) {
        return true;
    }
    bool balanced = (node->m_bf == -1) || (node->m_bf == +1) ||  (node->m_bf == 0); 
    return balanced && isValidAVL_InternalRecursive(node->m_left) && isValidAVL_InternalRecursive(node->m_right);
}


uint32_t binaryTree::writeTreeToBufferRecursive(
    char*       outputBuf, 
    uint32_t    outputBufIdx, 
    binaryTree* root, 
    uint32_t    space
) {
    constexpr auto kCOUNT = 5;
    if (root == NULL) {
        return 0;
    }
    space += kCOUNT;
    
    
    outputBufIdx = binaryTree::writeTreeToBufferRecursive(outputBuf, outputBufIdx, root->m_right, space);
    outputBufIdx += sprintf(&outputBuf[outputBufIdx], "\n\n\n%*s%d (%u, %d)\n", space - kCOUNT, "", root->m_data, root->m_height, root->m_bf);
    outputBufIdx = binaryTree::writeTreeToBufferRecursive(outputBuf, outputBufIdx, root->m_left, space);
    return outputBufIdx;
}




binaryTree::binaryTree(uint32_t value)
{
    m_data   = value;
    m_left   = nullptr;
    m_right  = nullptr;
    m_parent = nullptr;
    m_height = 0;
    m_bf     = 0;
    return;
}

binaryTree::binaryTree(
    binaryTree* left, 
    binaryTree* right, 
    binaryTree* parent, 
    uint32_t    value
) {
    m_data   = value;
    m_left   = left;
    m_right  = right;
    m_parent = parent;
    m_height = 0;
    m_bf     = 0;
    return;
}

void binaryTree::destroy(binaryTree* node) noexcept
{
    if(node == nullptr) {
        return;
    }


    uint32_t currentLevelSize = 0;
    std::queue<binaryTree*> currentHeightNodes;
    /* 
        Iterative Level Order Traversal 
        top node popped has already pushed its children to the queue
    */
    currentHeightNodes.push(node);
    while(!currentHeightNodes.empty()) {
        currentLevelSize = currentHeightNodes.size();

        while(currentLevelSize) {
            auto* currNode = currentHeightNodes.front();
            
            // while(currNode == nullptr && currentLevelSize) {
            //     currentHeightNodes.pop();
            //     currNode = currentHeightNodes.front();
            //     --currentLevelSize;
            // }

            if(currNode->m_left) {
                currentHeightNodes.push(currNode->m_left);
            }
            if(currNode->m_right) {
                currentHeightNodes.push(currNode->m_right);
            }
            currentHeightNodes.pop();
            --currentLevelSize;
            delete currNode; /* calling delete on nullptr is safe */
        }
    }
    return;
}

void binaryTree::deepCopy(
    binaryTree* nodeIn, 
    binaryTree* nodeOut
) {

}

void binaryTree::shallowCopy(
    binaryTree* nodeIn, 
    binaryTree* nodeOut
)
{
    memcpy(nodeOut, nodeIn, sizeof(binaryTree));
    return;
}

void binaryTree::move(
    binaryTree* toMoveFrom,
    binaryTree* toMoveTo
)
{
    shallowCopy(toMoveFrom, toMoveTo);
    memset(toMoveFrom, 0x00, sizeof(binaryTree));
    return;
}




bool binaryTree::isLeaf(binaryTree* node)  noexcept
{
    if(node == nullptr) {
        return false;
    }
    return node->m_left == nullptr && node->m_right == nullptr;
}

bool binaryTree::isSingleChildParent(binaryTree* node) noexcept
{
    if(node == nullptr) {
        return false;
    }
    bool leftState  = (node->m_left  == nullptr);
    bool rightState = (node->m_right == nullptr);
    return ( leftState && !rightState ) || ( !leftState && rightState );
}

binaryTree* binaryTree::getLeft(binaryTree* node) noexcept
{
    return node->m_left;
}

binaryTree* binaryTree::getRight(binaryTree* node) noexcept
{
    return node->m_right;
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
binaryTree* binaryTree::rotateLeft(binaryTree* node)
{
    auto* root_parent = node->m_parent;
    auto* x = node;
    auto* y = node->m_right;
    auto* b = y->m_left;

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

    x->m_height = computeHeight(x);
    x->m_bf     = computeBalanceFactor(x);
    y->m_height = computeHeight(y);
    y->m_bf     = computeBalanceFactor(y);
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
binaryTree* binaryTree::rotateRight(binaryTree* node)
{
    auto* root_parent = node->m_parent;
    auto* y = node;
    auto* x = y->m_left;
    auto* b = x->m_right;

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

    y->m_height = computeHeight(y);
    y->m_bf     = computeBalanceFactor(y);
    x->m_height = computeHeight(x);
    x->m_bf     = computeBalanceFactor(x);
    return x;
}

binaryTree* binaryTree::findMax(binaryTree* node)
{
    auto* search = node;
    while(search != nullptr) {
        node = search;
        search = search->m_right;
    }
    return node;
}

binaryTree* binaryTree::findMin(binaryTree* node)
{
    auto* search = node;
    while(search != nullptr) {
        node = search;
        search = search->m_left;
    }
    return node;
}




bool binaryTree::isValidBSTRecursive(binaryTree* node)
{
    if(node == nullptr) {
        return true;
    }
    if(binaryTree::isLeaf(node)) {
        return true;
    }
    bool satisfiesCondition = true;
    if(node->m_left) {
        satisfiesCondition = satisfiesCondition && (node->m_data > node->m_left->m_data);
    }
    if(node->m_right) {
        satisfiesCondition = satisfiesCondition && (node->m_data < node->m_right->m_data);
    }
    return satisfiesCondition && isValidBSTRecursive(node->m_left) && isValidBSTRecursive(node->m_right);
}

bool binaryTree::isValidAVL(binaryTree* node)
{
    if(!binaryTree::isValidBSTRecursive(node)) {
        return false;
    }
    return binaryTree::isValidAVL_InternalRecursive(node);
}

void binaryTree::writeBufferRecursive(
    char*       outputBuf, 
    uint32_t    outputBufIdx, 
    binaryTree* root, 
    uint32_t    space
) {
    writeTreeToBufferRecursive(
        outputBuf,
        outputBufIdx,
        root,
        space
    );
    return;
}




void binaryTree::searchval(uint32_t value, binaryTree*& foundptr) {
    bool        foundcond = false;
    binaryTree* searchptr = nullptr;

    for(searchptr = foundptr; (searchptr != nullptr) && !foundcond; ) {
        foundcond = (searchptr->m_data == value);
        // printf("searchval cmp: %u ==? %u\n", searchptr->m_data, value);
        if(!foundcond) {
            searchptr = (value < searchptr->m_data) ? 
                searchptr->m_left : 
                searchptr->m_right;
        }
    }

    // printf("found %u -> %s\n", value, foundcond ? "SUCCESS" : "FAILURE");
    foundptr = searchptr;
    return;
}


/* The rest of the definitions are in AVLTree2.cpp */