#ifndef __BINARY_TREE_GENERIC_IMPLEMENTATION_HEADER__
#   define __BINARY_TREE_GENERIC_IMPLEMENTATION_HEADER__
#   ifndef __BINARY_TREE_GENERIC_DEFINITION_HEADER__
#       include <tree/internal/binaryTreeGeneric.hpp>
#   endif /* __BINARY_TREE_GENERIC_DEFINITION_HEADER__ */
#include <tree/internal/AVLTreeRotateState.hpp>
#include <cstdio>
#include <cstring>
#include <queue>


template<typename T>
int8_t binaryTree<T>::computeHeight(binaryTree<T>* node) noexcept
{
    if(node == nullptr) {
        return -1;
    }
    auto rh = node->m_right ? node->m_right->m_height : -1;
    auto lh = node->m_left  ? node->m_left->m_height  : -1;
    return 1 + std::max(rh, lh);
}

template<typename T>
int8_t binaryTree<T>::computeBalanceFactor(binaryTree<T>* node) noexcept
{
    if(node == nullptr) {
        return 0;
    }
    int64_t rh = node->m_right ? node->m_right->m_height : -1;
    int64_t lh = node->m_left  ? node->m_left->m_height  : -1;
    return rh - lh;
}

template<typename T>
binaryTree<T>* binaryTree<T>::findMaxAndPushParents(binaryTree<T>* node, std::deque<binaryTree<T>*>& parentQueue)
{
    auto* search = node;
    while(search != nullptr) {
        node = search;
        parentQueue.push_back(node);
        search = search->m_right;
    }
    return node;
}

template<typename T>
binaryTree<T>* binaryTree<T>::findMinAndPushParents(binaryTree<T>* node, std::deque<binaryTree<T>*>& parentQueue)
{
    auto* search = node;
    while(search != nullptr) {
        node = search;
        parentQueue.push_back(node);
        search = search->m_left;
    }
    return node;
}

template<typename T>
bool binaryTree<T>::isValidAVL_InternalRecursive(binaryTree<T>* node)
{
    if(node == nullptr) {
        return true;
    }
    bool balanced = (node->m_bf == -1) || (node->m_bf == +1) ||  (node->m_bf == 0); 
    return balanced && isValidAVL_InternalRecursive(node->m_left) && isValidAVL_InternalRecursive(node->m_right);
}


template<typename T>
uint32_t binaryTree<T>::writeTreeToBufferRecursive(
    char*       outputBuf, 
    uint32_t    outputBufIdx, 
    binaryTree<T>* root, 
    uint32_t    space
) {
    constexpr auto kCOUNT = 5;
    if (root == NULL) {
        return 0;
    }
    space += kCOUNT;
    
    
    outputBufIdx = binaryTree<T>::writeTreeToBufferRecursive(outputBuf, outputBufIdx, root->m_right, space);
    outputBufIdx += sprintf(&outputBuf[outputBufIdx], "\n\n\n%*s%d (%u, %d)\n", space - kCOUNT, "", root->m_data, root->m_height, root->m_bf);
    outputBufIdx = binaryTree<T>::writeTreeToBufferRecursive(outputBuf, outputBufIdx, root->m_left, space);
    return outputBufIdx;
}




template<typename T>
binaryTree<T>::binaryTree(T const& value)
{
    m_data.set(value);
    m_left   = nullptr;
    m_right  = nullptr;
    m_parent = nullptr;
    m_height = 0;
    m_bf     = 0;
    return;
}

template<typename T>
binaryTree<T>::binaryTree(
    binaryTree<T>* left, 
    binaryTree<T>* right, 
    binaryTree<T>* parent, 
    T const&       value
) {
    m_data.set(value);
    m_left   = left;
    m_right  = right;
    m_parent = parent;
    m_height = 0;
    m_bf     = 0;
    return;
}

template<typename T>
void binaryTree<T>::destroy(binaryTree<T>* node) noexcept
{
    if(node == nullptr) {
        return;
    }


    uint32_t currentLevelSize = 0;
    std::queue<binaryTree<T>*> currentHeightNodes;
    /* 
        Iterative Level Order Traversal 
        top node popped has already pushed its children to the queue
    */
    currentHeightNodes.push(node);
    while(!currentHeightNodes.empty()) {
        currentLevelSize = currentHeightNodes.size();

        while(currentLevelSize) {
            binaryTree<T>* currNode = currentHeightNodes.front();

            if(currNode->m_left) {
                currentHeightNodes.push(currNode->m_left);
            }
            if(currNode->m_right) {
                currentHeightNodes.push(currNode->m_right);
            }

            currNode->m_data.release();
            delete currNode; /* calling delete on nullptr is safe */


            currentHeightNodes.pop();
            --currentLevelSize;
        }
    }
    return;
}

template<typename T>
void binaryTree<T>::deepCopy(
    binaryTree<T>* nodeIn, 
    binaryTree<T>* nodeOut
) {

}

template<typename T>
void binaryTree<T>::shallowCopy(
    binaryTree<T>* nodeIn, 
    binaryTree<T>* nodeOut
) {
    nodeOut->m_left   = nodeIn->m_left;
    nodeOut->m_right  = nodeIn->m_right;
    nodeOut->m_parent = nodeIn->m_parent;
    nodeOut->m_data.set(nodeIn->m_data.get());
    nodeOut->m_height   = nodeIn->m_height;
    nodeOut->m_bf       = nodeIn->m_bf;
    for(int i = 0; i < sizeof(nodeOut->m_reserved) / sizeof(nodeOut->m_reserved[0]); ++i) {
        nodeOut->m_reserved[i] = nodeIn->m_reserved[i];
    }
    return;
}

template<typename T>
void binaryTree<T>::move(
    binaryTree<T>* toMoveFrom,
    binaryTree<T>* toMoveTo
)
{
    shallowCopy(toMoveFrom, toMoveTo);
    memset(toMoveFrom, 0x00, toMoveFrom->nodeSize());
    return;
}




template<typename T>
bool binaryTree<T>::isLeaf(binaryTree<T>* node)  noexcept
{
    if(node == nullptr) {
        return false;
    }
    return node->m_left == nullptr && node->m_right == nullptr;
}

template<typename T>
bool binaryTree<T>::isSingleChildParent(binaryTree<T>* node) noexcept
{
    if(node == nullptr) {
        return false;
    }
    bool leftState  = (node->m_left  == nullptr);
    bool rightState = (node->m_right == nullptr);
    return ( leftState && !rightState ) || ( !leftState && rightState );
}

template<typename T>
binaryTree<T>* binaryTree<T>::getLeft(binaryTree<T>* node) noexcept
{
    return node->m_left;
}

template<typename T>
binaryTree<T>* binaryTree<T>::getRight(binaryTree<T>* node) noexcept
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
template<typename T>
binaryTree<T>* binaryTree<T>::rotateLeft(binaryTree<T>* node)
{
    auto* root_parent = node->m_parent;
    auto* x = node;
    auto* y = x->m_right;
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
template<typename T>
binaryTree<T>* binaryTree<T>::rotateRight(binaryTree<T>* node)
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

template<typename T>
binaryTree<T>* binaryTree<T>::findMax(binaryTree<T>* node)
{
    auto* search = node;
    while(search != nullptr) {
        node = search;
        search = search->m_right;
    }
    return node;
}

template<typename T>
binaryTree<T>* binaryTree<T>::findMin(binaryTree<T>* node)
{
    auto* search = node;
    while(search != nullptr) {
        node = search;
        search = search->m_left;
    }
    return node;
}

template<typename T>
void binaryTree<T>::maybeRebalance(
    binaryTree<T>*  node, 
    binaryTree<T>** maybeNewRoot
) {
    node->m_height = binaryTree<T>::computeHeight(node);
    node->m_bf     = binaryTree<T>::computeBalanceFactor(node);
    auto bfright = computeBalanceFactor(node->m_right);
    auto bfleft  = computeBalanceFactor(node->m_left);

    AVLTreeRotationState state = AVLTreeRotationState::NONE;
    state = (node->m_bf == -2 && bfleft  <= 0) ? AVLTreeRotationState::LEFTLEFT   : state;
    state = (node->m_bf == -2 && bfleft  >  0) ? AVLTreeRotationState::LEFTRIGHT  : state;
    state = (node->m_bf == +2 && bfright >= 0) ? AVLTreeRotationState::RIGHTRIGHT : state;
    state = (node->m_bf == +2 && bfright <  0) ? AVLTreeRotationState::RIGHTLEFT  : state;
    
    
    switch(state) {
        case AVLTreeRotationState::LEFTLEFT:
        *maybeNewRoot = rotateRight(node);
        break;

        case AVLTreeRotationState::LEFTRIGHT:
        node->m_left = rotateLeft(node->m_left);
        *maybeNewRoot = rotateRight(node);
        break;

        case AVLTreeRotationState::RIGHTRIGHT:
        *maybeNewRoot = rotateLeft(node);
        break;

        case AVLTreeRotationState::RIGHTLEFT:
        node->m_right = rotateRight(node->m_right);
        *maybeNewRoot = rotateLeft(node);
        break;

        case AVLTreeRotationState::NONE:
        default:
        break;
    }


    return;
}





template<typename T>
bool binaryTree<T>::isValidBSTRecursive(binaryTree<T>* node)
{
    if(node == nullptr) {
        return true;
    }
    if(binaryTree<T>::isLeaf(node)) {
        return true;
    }
    bool satisfiesCondition = true;
    if(node->m_left) {
        satisfiesCondition = satisfiesCondition && (node->m_data.get() > node->m_left->m_data.get());
    }
    if(node->m_right) {
        satisfiesCondition = satisfiesCondition && (node->m_data.get() < node->m_right->m_data.get());
    }
    return satisfiesCondition && isValidBSTRecursive(node->m_left) && isValidBSTRecursive(node->m_right);
}

template<typename T>
bool binaryTree<T>::isValidAVL(binaryTree<T>* node)
{
    if(!binaryTree<T>::isValidBSTRecursive(node)) {
        return false;
    }
    return binaryTree<T>::isValidAVL_InternalRecursive(node);
}

template<typename T>
void binaryTree<T>::writeBufferRecursive(
    char*       outputBuf, 
    uint32_t    outputBufIdx, 
    binaryTree<T>* root, 
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




template<typename T>
binaryTree<T>* binaryTree<T>::searchIterative(binaryTree<T>* node, T const& value) {
    bool           found = false;
    binaryTree<T>* searchptr = nullptr;

    for(searchptr = node; (searchptr != nullptr) && !found; ) {
        found = (searchptr->m_data.get() == value);
        // printf("searchIterative cmp: %u ==? %u\n", searchptr->m_data, value);
        if(!found) {
            searchptr = (value < searchptr->m_data.get()) ? 
                searchptr->m_left : 
                searchptr->m_right;
        }
    }

    // printf("found %u -> %s\n", value, foundcond ? "SUCCESS" : "FAILURE");
    return found ? searchptr : nullptr;
}


template<typename T>
bool binaryTree<T>::searchRecursive(binaryTree* node, T const& value)
{
    if(node == nullptr) {
        return false;
    }

    auto const& nodeData = node->m_data.get();
    if(value < nodeData) {
        return binaryTree<T>::searchRecursive(node->m_left, value);
    } 
    else if(value > nodeData) {
        return binaryTree<T>::searchRecursive(node->m_right, value);
    }


    return true;
}




template<typename T>
bool binaryTree<T>::AVLInsertIterative(binaryTree<T>* node, T const& value, binaryTree<T>** out)
{
    int8_t      bfright, bfleft;
    bool        inserted = false;
    bool        found  = false;
    binaryTree<T>* search = node;
    binaryTree<T>* allocNode    = nullptr;
    binaryTree<T>* maybeNewRoot = nullptr;
    std::deque<binaryTree<T>*> nodesTouched;


    if(node == nullptr) {
        *out = new binaryTree<T>{value};
        return true;
    }

    // binaryTree<T>::searchIterative(val, search);
    // if(search != nullptr) {
    //     *out = node;
    //     return false;
    // }


    // search    = node;
    // allocNode = new binaryTree<T>{val};
    // for(; search != nullptr ;) {
    //     nodesTouched.push_back(search);
    //     search = (val < search->m_data) ? search->m_left : search->m_right;
    // }
    for(search = node; !found && search != nullptr; ) {
        nodesTouched.push_back(search);
        found  = (value == search->m_data.get());
        search = (value < search->m_data.get()) ? search->m_left : search->m_right;
    }
    
    
    if(found) {
        *out = node;
        return false;
    }
    
    
    allocNode = new binaryTree<T>{value};
    if(allocNode == nullptr) {
        *out = nullptr;
        return false;
    }


    bfright = bfleft = 0;
    for(; !nodesTouched.empty() ;) {
        auto& currentParent = nodesTouched.back();
        /* 
            If there was a rebalance, maybeNewRoot would change to the correct root
            Otherwise, maybeNewRoot is just the old root, and thus can be returned as normal.
        */
        maybeNewRoot = currentParent;

        /* I don't want to unroll the first-iteration contents of the loop just for insertion. */
        /* In Hindsight, this was a mistake. See the C Implementation for Insertion/Deletion */
        if(!inserted) {
            // currentParent->m_nodes[ (val < currentParent->m_data) ? 1 : 0] = allocNode;
            if(value < currentParent->m_data.get()) {
                currentParent->m_left = allocNode;
            } else {
                currentParent->m_right = allocNode;
            }
            allocNode->m_parent = currentParent;
            inserted = true;
        }


        binaryTree<T>::maybeRebalance(currentParent, &maybeNewRoot);
        nodesTouched.pop_back();
    }


    *out = maybeNewRoot;
    return true;
}


template<typename T>
bool binaryTree<T>::AVLDeleteIterative(binaryTree<T>* node, T const& value, binaryTree<T>** out)
{
    bool           found     = false;
    binaryTree<T>* search    = nullptr;
    binaryTree<T>* allocNode = nullptr;
    std::deque<binaryTree<T>*> nodesTouched;

    if(node == nullptr) {
        *out = nullptr;
        return false;
    }
    if(binaryTree<T>::isLeaf(node) && value == node->m_data.get()) {
        node->m_data.release();
        delete node;
        
        *out = nullptr;
        return true;
    }

    auto* childIfSingleParent = node->m_left ? node->m_left : node->m_right;
    if(binaryTree<T>::isSingleChildParent(node) && value == node->m_data.get()) {
        node->m_data.release();
        delete node;
        
        childIfSingleParent->m_parent = nullptr;
        *out = childIfSingleParent;
        return true;
    }
    if(binaryTree<T>::isSingleChildParent(node) && value == childIfSingleParent->m_data.get()) {
        childIfSingleParent->m_data.release();
        delete childIfSingleParent;
        
        node->m_left  = (childIfSingleParent == node->m_left ) ? nullptr : node->m_left;
        node->m_right = (childIfSingleParent == node->m_right) ? nullptr : node->m_right;
        *out = node;
        return true;
    }


    found  = false;
    search = node;
    for(; search != nullptr && !found; ) {
        nodesTouched.push_back(search);
        found = (value == search->m_data.get());
        search = (value < search->m_data.get()) ? search->m_left : search->m_right;
    }
    if(!found) {
        *out = node;
        return false;
    }


    bool           alreadyDeleted = false;
    binaryTree<T>* maybeNewRoot   = nullptr;


    /* nodesTouched.back() will return the element to be deleted, if found == true. */
    for(; !nodesTouched.empty() ;) {
        /* 
            If there was a rebalance, maybeNewRoot would change to the correct root
            Otherwise, maybeNewRoot is just the old root, and thus can be returned as normal.
        */
        auto* currentParent = nodesTouched.back();
        maybeNewRoot = currentParent;


        if(!alreadyDeleted) { /* currentParent is the node to be deleted ; may enter twice if the node has 2 children. */
            auto&       parent   = currentParent->m_parent;
            binaryTree<T>* getChild = nullptr;

            if(binaryTree<T>::isLeaf(currentParent)) {
                parent->m_left  = (parent->m_left  == currentParent) ? getChild : parent->m_left;
                parent->m_right = (parent->m_right == currentParent) ? getChild : parent->m_right;
                
                currentParent->m_data.release();
                delete currentParent;
                alreadyDeleted = true;
            } 
            else if(binaryTree<T>::isSingleChildParent(currentParent)) {
                getChild = currentParent->m_left ? currentParent->m_left : currentParent->m_right;
                getChild->m_parent = parent;
                parent->m_left  = (parent->m_left  == currentParent) ? getChild : parent->m_left;
                parent->m_right = (parent->m_right == currentParent) ? getChild : parent->m_right;
                
                currentParent->m_data.release();
                delete currentParent;
                alreadyDeleted = true;
            }
            else { /* Need to choose between either node paths */
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


                // /* 0. Pop the currentParent from the top of the queue.
                //     Because we're deleting successorNode, we don't want to delete currentParent anymore.
                // */
                // nodesTouched.pop_back();


                /* 1. Find Successor */
                binaryTree<T>* successorNode = nullptr;
                if(currentParent->m_left->m_height < currentParent->m_right->m_height) { /* search in the smaller subtree */
                    successorNode = findMaxAndPushParents(currentParent->m_left, nodesTouched);
                } else {
                    successorNode = findMinAndPushParents(currentParent->m_right, nodesTouched);
                }
                assert(successorNode != nullptr && "There should always be a successor node available\n");

                /* 2. Swap Values */
                std::swap(currentParent->m_data, successorNode->m_data);

                /* 
                    3. delete(successorNode) 
                    This will happen in the next iteration.
                    Because nodesTouched.back() is the successorNode currently, we don't want to pop it just yet
                    Therefore, we'll continue to the next iteration.
                */
                continue;
            }

            /* if a node was deleted we do not want to update any of its values. */
            nodesTouched.pop_back();
            continue;
        }
        

        binaryTree<T>::maybeRebalance(currentParent, &maybeNewRoot);
        nodesTouched.pop_back();
    }


    *out = maybeNewRoot;
    return true;
}


template<typename T>
binaryTree<T>* binaryTree<T>::AVLInsertRecursive(binaryTree* node, binaryTree* parent, T const& value)
{
    /* First Iteration node=root; If we rebalanced on First iteration maybeNewRoot will contain the updated root */
    binaryTree* allocNode    = nullptr;
    binaryTree* maybeNewRoot = node;

    if(node == nullptr) { /* We finished searching */
        /* 
            We should notify if this allocation fails, 
            In which case we don't increment the nodeCount in the original AVL tree 
        */
        allocNode = new binaryTree<T>{ nullptr, nullptr, parent, value };
        return allocNode;
    }

    if(value < node->m_data.get()) { 
        /* Search Left-Subtree */
        node->m_left = binaryTree<T>::AVLInsertRecursive(node->m_left, node, value);
    } 
    else {
        /* Search Right-Subtree */
        node->m_right = binaryTree<T>::AVLInsertRecursive(node->m_right, node, value);
    }


    binaryTree<T>::maybeRebalance(node, &maybeNewRoot);
    return maybeNewRoot;
}



template<typename T>
binaryTree<T>* binaryTree<T>::AVLDeleteRecursive(binaryTree* node, binaryTree* parent, T const& value)
{
    binaryTree* allocNode    = nullptr;
    binaryTree* maybeNewRoot = node;


    if(value < node->m_data.get()) { 
        /* Search Left-Subtree */
        node->m_left = binaryTree<T>::AVLDeleteRecursive(node->m_left, node, value);
    
    } else if(value > node->m_data.get()) {
        /* Search Right-Subtree */
        node->m_right = binaryTree<T>::AVLDeleteRecursive(node->m_right, node, value);

    } else {
        /* Node was found */
        if(binaryTree<T>::isLeaf(node)) {
            node->m_data.release();
            delete node;
            return nullptr;
        }
        else if(binaryTree<T>::isSingleChildParent(node)) {
            auto* childNode = node->m_left ? node->m_left : node->m_right;
            childNode->m_parent = parent;

            node->m_data.release();
            delete node;
            return childNode;
        }
        else {
            /* Node is full. need to find a successor */
            bool leftSubTreeSmaller = node->m_left->m_height < node->m_right->m_height;
            binaryTree<T>* successor = nullptr;

            if(leftSubTreeSmaller) {
                successor = findMax(node->m_left);
                T const& successorData = successor->m_data.get();

                node->m_data.set(successorData);
                node->m_left = binaryTree<T>::AVLDeleteRecursive(node->m_left, node, successorData);
            } else {
                successor = findMin(node->m_right);
                T const& successorData = successor->m_data.get();

                node->m_data.set(successorData);
                node->m_right = binaryTree<T>::AVLDeleteRecursive(node->m_right, node, successorData);
            }
        }
    }


    binaryTree<T>::maybeRebalance(node, &maybeNewRoot);
    return maybeNewRoot;
}


#endif /* __BINARY_TREE_GENERIC_IMPLEMENTATION_HEADER__ */