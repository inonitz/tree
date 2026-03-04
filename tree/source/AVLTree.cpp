#include <tree/AVLTree.hpp>
#include <tree/binaryTree.hpp>
#include <cstdio>
#include <cassert>
#include <stack>
#include <utility>


bool binaryTree::AVLinsertIterative(binaryTree* node, uint32_t val, binaryTree** out)
{
    int8_t      bfright, bfleft;
    bool        inserted = false;
    bool        found  = false;
    binaryTree* search = node;
    binaryTree* allocNode    = nullptr;
    binaryTree* maybeNewRoot = nullptr;
    std::deque<binaryTree*> nodesTouched;


    if(node == nullptr) {
        *out = new binaryTree{val};
        return true;
    }

    // binaryTree::searchval(val, search);
    // if(search != nullptr) {
    //     *out = node;
    //     return false;
    // }


    // search    = node;
    // allocNode = new binaryTree{val};
    // for(; search != nullptr ;) {
    //     nodesTouched.push_back(search);
    //     search = (val < search->m_data) ? search->m_left : search->m_right;
    // }
    for(search = node; !found && search != nullptr; ) {
        nodesTouched.push_back(search);
        found  = (val == search->m_data);
        search = (val < search->m_data) ? search->m_left : search->m_right;
    }
    
    
    if(found) {
        *out = node;
        return false;
    }
    
    
    allocNode = new binaryTree{val};
    bfright = bfleft = 0;
    for(; !nodesTouched.empty() ;) {
        auto& currentParent = nodesTouched.back();
        /* 
            If there was a rebalance, maybeNewRoot would change to the correct root
            Otherwise, maybeNewRoot is just the old root, and thus can be returned as normal.
        */
        maybeNewRoot = currentParent;

        /* I don't want to unroll the first-iteration contents of the loop just for insertion. */
        if(!inserted) {
            // currentParent->m_nodes[ (val < currentParent->m_data) ? 1 : 0] = allocNode;
            if(val < currentParent->m_data) {
                currentParent->m_left = allocNode;
            } else {
                currentParent->m_right = allocNode;
            }
            allocNode->m_parent = currentParent;
            inserted = true;
        }


        currentParent->m_height = binaryTree::computeHeight(currentParent);
        currentParent->m_bf     = binaryTree::computeBalanceFactor(currentParent);
        auto bfright = computeBalanceFactor(currentParent->m_right);
        auto bfleft  = computeBalanceFactor(currentParent->m_left);
        AVLTreeRotationState state = AVLTreeRotationState::NONE;
        state = (currentParent->m_bf == -2 && bfleft  <= 0) ? AVLTreeRotationState::LEFTLEFT   : state;
        state = (currentParent->m_bf == -2 && bfleft  >  0) ? AVLTreeRotationState::LEFTRIGHT  : state;
        state = (currentParent->m_bf == +2 && bfright >= 0) ? AVLTreeRotationState::RIGHTRIGHT : state;
        state = (currentParent->m_bf == +2 && bfright <  0) ? AVLTreeRotationState::RIGHTLEFT  : state;
        switch(state) {
            case AVLTreeRotationState::LEFTLEFT:
            maybeNewRoot = rotateRight(currentParent);
            break;

            case AVLTreeRotationState::LEFTRIGHT:
            currentParent->m_left = rotateLeft(currentParent->m_left);
            maybeNewRoot = rotateRight(currentParent);
            break;

            case AVLTreeRotationState::RIGHTRIGHT:
            maybeNewRoot = rotateLeft(currentParent);
            break;

            case AVLTreeRotationState::RIGHTLEFT:
            currentParent->m_right = rotateRight(currentParent->m_right);
            maybeNewRoot = rotateLeft(currentParent);
            break;

            case AVLTreeRotationState::NONE:
            default:
            break;
        }


        nodesTouched.pop_back();
    }


    *out = maybeNewRoot;
    return true;
}


bool binaryTree::AVLdeleteIterative(binaryTree* node, uint32_t value, binaryTree** out)
{
    bool        found     = false;
    binaryTree* search    = nullptr;
    binaryTree* allocNode = nullptr;
    std::deque<binaryTree*> nodesTouched;

    if(node == nullptr) {
        *out = nullptr;
        return false;
    }
    if(binaryTree::isLeaf(node) && value == node->m_data) {
        delete node;
        *out = nullptr;
        return true;
    }

    auto* childIfSingleParent = node->m_left ? node->m_left : node->m_right;
    if(binaryTree::isSingleChildParent(node) && value == node->m_data) {
        delete node;
        childIfSingleParent->m_parent = nullptr;
        *out = childIfSingleParent;
        return true;
    }
    if(binaryTree::isSingleChildParent(node) && value == childIfSingleParent->m_data) {
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
        found = (value == search->m_data);
        search = (value < search->m_data) ? search->m_left : search->m_right;
    }
    if(!found) {
        *out = node;
        return false;
    }


    bool        alreadyDeleted  = false;
    binaryTree* maybeNewRoot    = nullptr;
    binaryTree  deletedNodeCopy;


    /* nodesTouched.back() will return the element to be deleted, if found == true. */
    // binaryTree::shallowCopy(&deletedNodeCopy, nodesTouched.back());
    binaryTree::shallowCopy(nodesTouched.back(), &deletedNodeCopy);
    for(; !nodesTouched.empty() ;) {
        /* 
            If there was a rebalance, maybeNewRoot would change to the correct root
            Otherwise, maybeNewRoot is just the old root, and thus can be returned as normal.
        */
        auto* currentParent = nodesTouched.back();
        maybeNewRoot = currentParent;


        if(!alreadyDeleted) { /* currentParent is the node to be deleted ; may enter twice if the node has 2 children. */
            auto&       parent   = currentParent->m_parent;
            binaryTree* getChild = nullptr;

            if(binaryTree::isLeaf(currentParent)) {
                parent->m_left  = (parent->m_left  == currentParent) ? getChild : parent->m_left;
                parent->m_right = (parent->m_right == currentParent) ? getChild : parent->m_right;
                delete currentParent;
                alreadyDeleted = true;
            } 
            else if(binaryTree::isSingleChildParent(currentParent)) {
                getChild = currentParent->m_left ? currentParent->m_left : currentParent->m_right;
                getChild->m_parent = parent;
                parent->m_left  = (parent->m_left  == currentParent) ? getChild : parent->m_left;
                parent->m_right = (parent->m_right == currentParent) ? getChild : parent->m_right;
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
                binaryTree* successorNode = nullptr;
                if(currentParent->m_left->m_height < currentParent->m_right->m_height) { /* search in the smaller subtree */
                    successorNode = findMaxAndPushParents(currentParent->m_left, nodesTouched);
                } else {
                    successorNode = findMinAndPushParents(currentParent->m_right, nodesTouched);
                }
                assert(successorNode != nullptr && "There should always be a successor node available\n");

                /* 2. Swap Values */
                currentParent->m_data = successorNode->m_data;
                successorNode->m_data = deletedNodeCopy.m_data;

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
        

        currentParent->m_height = computeHeight(currentParent);
        currentParent->m_bf     = computeBalanceFactor(currentParent);
        auto bfright = computeBalanceFactor(currentParent->m_right);
        auto bfleft  = computeBalanceFactor(currentParent->m_left);
        AVLTreeRotationState state = AVLTreeRotationState::NONE;
        state = (currentParent->m_bf == -2 && bfleft  <= 0) ? AVLTreeRotationState::LEFTLEFT   : state;
        state = (currentParent->m_bf == -2 && bfleft  >  0) ? AVLTreeRotationState::LEFTRIGHT  : state;
        state = (currentParent->m_bf == +2 && bfright >= 0) ? AVLTreeRotationState::RIGHTRIGHT : state;
        state = (currentParent->m_bf == +2 && bfright <  0) ? AVLTreeRotationState::RIGHTLEFT  : state;
        switch(state) {
            case AVLTreeRotationState::LEFTLEFT:
            maybeNewRoot = rotateRight(currentParent);
            break;

            case AVLTreeRotationState::LEFTRIGHT:
            currentParent->m_left = rotateLeft(currentParent->m_left);
            maybeNewRoot = rotateRight(currentParent);
            break;

            case AVLTreeRotationState::RIGHTRIGHT:
            maybeNewRoot = rotateLeft(currentParent);
            break;

            case AVLTreeRotationState::RIGHTLEFT:
            currentParent->m_right = rotateRight(currentParent->m_right);
            maybeNewRoot = rotateLeft(currentParent);
            break;

            case AVLTreeRotationState::NONE:
            default:
            break;
        }


        nodesTouched.pop_back();
    }


    *out = maybeNewRoot;
    return true;
}




AVLTree::AVLTree() : m_root{nullptr}, m_nodeCount{0} {}


void AVLTree::clear() noexcept {
    binaryTree::destroy(m_root);
    m_root = nullptr;
    m_nodeCount = 0;
    return;
}

bool AVLTree::insert(uint32_t val) {
    binaryTree* newRootMaybe = nullptr;
    bool        status       = binaryTree::AVLinsertIterative(m_root, val, &newRootMaybe);
    
    m_root       = newRootMaybe != nullptr ? newRootMaybe : m_root;
    m_nodeCount += status;
    return status;
}

bool AVLTree::remove(uint32_t val) {
    binaryTree* newRootMaybe = nullptr;
    bool        status       = binaryTree::AVLdeleteIterative(m_root, val, &newRootMaybe);


    if(status == true) {
        m_root = newRootMaybe;
        --m_nodeCount;
        return true;
    }
    return false;
}

bool AVLTree::search(uint32_t val) {
    if(empty()) {
        return false;
    }
    binaryTree* found = m_root;
    binaryTree::searchval(val, found);
    return found ? true : false;
}



[[nodiscard]] bool AVLTree::isValidBST() const noexcept {
    return binaryTree::isValidBSTRecursive(m_root);
}

[[nodiscard]] bool AVLTree::isBalanced() const noexcept {
    return binaryTree::isValidAVL(m_root);
}

[[nodiscard]] bool AVLTree::empty() const {
    return m_nodeCount == 0;
}
[[nodiscard]] size_t AVLTree::size() const {
    return m_nodeCount;
}
[[nodiscard]] size_t AVLTree::height() const {
    return m_root->m_height;
}
[[nodiscard]] auto AVLTree::getRoot() const -> binaryTree const* {
    return m_root;
}
[[nodiscard]] auto AVLTree::getLeftChild() const -> binaryTree const* {
    return m_root->m_left;
}
[[nodiscard]] auto AVLTree::getRightChild() const -> binaryTree const* {
    return m_root->m_right;
}

void AVLTree::printRecursive(const binaryTree* root, int space) const {
    constexpr auto kCOUNT = 10;
    
    if (root == NULL) {
        return;
    }
    space += kCOUNT;
    
    
    printRecursive(root->m_right, space);
    printf("\n%*s%d (%u, %d)\n", space - kCOUNT, "", root->m_data, root->m_height, root->m_bf);
    printRecursive(root->m_left, space);
    return;
}


void AVLTree::printIterative(const binaryTree* root, int space) const {
    constexpr auto kCOUNT = 8;
    
    
    if (root == nullptr) {
        return;
    }
    using TreeIdentPair = std::pair<const binaryTree*, uint32_t>;


    std::stack<TreeIdentPair> s;
    const binaryTree* curr = root;
    uint32_t currentSpace = 0;
    while (curr != nullptr || !s.empty()) {
        while (curr != nullptr) {
            currentSpace += kCOUNT;
            s.push({curr, currentSpace});
            curr = curr->m_right;
        }


        TreeIdentPair top = s.top();
        s.pop();
        
        curr = top.first;
        int space = top.second;

        printf("\n%*s%d (%u, %d)\n", space - kCOUNT, "", curr->m_data, curr->m_height, curr->m_bf);

        curr = curr->m_left;
        currentSpace = space;
    }
}