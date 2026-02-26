#pragma once
#include <util2/C/ifcrash2.h>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <queue>


enum class AVLTreeRotationState : uint8_t {
    NONE       = 0,
    LEFTLEFT   = 1,
    RIGHTRIGHT = 2,
    LEFTRIGHT  = 3,
    RIGHTLEFT  = 4
};



struct binaryTree 
{
    union 
    {
        struct {
            binaryTree* m_left   = nullptr;
            binaryTree* m_right  = nullptr;
            binaryTree* m_parent = nullptr;
        };
        binaryTree* m_nodes[3];
    };
    uint32_t m_data   = 0;
    uint8_t  m_height = 0;
    int8_t   m_bf     = 0;
    uint8_t  m_reserved[2]{0};


    binaryTree(uint32_t value) : m_data{value} {}
    binaryTree(
        binaryTree* left, 
        binaryTree* right, 
        binaryTree* parent, 
        uint32_t    value
    ) : 
        m_left{left},
        m_right{right},
        m_parent{parent},
        m_data{value},
        m_height{0},
        m_bf{0},
        m_reserved{{0}}
        {}


    ~binaryTree() = default;

    void destroy() noexcept {
        uint32_t currentLevelSize = 0;
        std::queue<binaryTree*> currentHeightNodes;
        /* 
            Iterative Level Order Traversal 
            top node popped has already pushed its children to the queue
        */
        currentHeightNodes.push(this);
        while(!currentHeightNodes.empty()) {
            currentLevelSize = currentHeightNodes.size();

            while(currentLevelSize) {
                auto* node = currentHeightNodes.front();
                
                if(node->m_left) {
                    currentHeightNodes.push(node->m_left);
                }
                if(node->m_right) {
                    currentHeightNodes.push(node->m_right);
                }
                currentHeightNodes.pop();
                --currentLevelSize;
                delete node; /* calling delete on nullptr is safe */
            }
        }
        return;
    }

    static bool isLeaf(binaryTree* node) noexcept {
        if(node == nullptr) {
            return false;
        }
        return node->m_left == nullptr && node->m_right == nullptr;
    }


    static bool singleChildParent(binaryTree* node) noexcept {
        if(node == nullptr) {
            return false;
        }
        bool leftState  = (node->m_left  == nullptr);
        bool rightState = (node->m_right == nullptr);
        return ( leftState && !rightState ) || ( !leftState && rightState );
    }


    static uint8_t computeHeight(binaryTree* node) noexcept {
        if(node == nullptr) {
            return -1;
        }
        auto rh = node->m_right ? node->m_right->m_height : -1;
        auto lh = node->m_left  ? node->m_left->m_height  : -1;
        return 1 + std::max(rh, lh);
    }

    static int8_t computeBalanceFactor(binaryTree* node) noexcept {
        if(node == nullptr) {
            return 0;
        }
        int64_t rh = node->m_right ? node->m_right->m_height : -1;
        int64_t lh = node->m_left  ? node->m_left->m_height  : -1;
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
    static binaryTree* rotateLeft(binaryTree* node) {
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
    static binaryTree* rotateRight(binaryTree* node) {
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


    static binaryTree* findMax(binaryTree* node) {
        auto* search = node;
        while(search != nullptr) {
            node = search;
            search = search->m_right;
        }
        return node;
    }


    static binaryTree* findMin(binaryTree* node) {
        auto* search = node;
        while(search != nullptr) {
            node = search;
            search = search->m_left;
        }
        return node;
    }

    static binaryTree* findMaxAndPushParents(binaryTree* node, std::deque<binaryTree*>& parentQueue) {
        auto* search = node;
        while(search != nullptr) {
            node = search;
            parentQueue.push_back(node);
            search = search->m_right;
        }
        return node;
    }


    static binaryTree* findMinAndPushParents(binaryTree* node, std::deque<binaryTree*>& parentQueue) {
        auto* search = node;
        while(search != nullptr) {
            node = search;
            parentQueue.push_back(node);
            search = search->m_left;
        }
        return node;
    }


    static bool isValidBST(binaryTree* node) {
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
        return satisfiesCondition && isValidBST(node->m_left) && isValidBST(node->m_right);
    }


    static bool isValidAVL_Internal(binaryTree* node) {
        if(node == nullptr) {
            return true;
        }
        bool balanced = (node->m_bf == -1) || (node->m_bf == +1) ||  (node->m_bf == 0); 
        return balanced && isValidAVL_Internal(node->m_left) && isValidAVL_Internal(node->m_right);
    }
    static bool isValidAVL(binaryTree* node) {
        if(!isValidBST(node)) {
            return false;
        }
        return isValidAVL_Internal(node);
    }



    static void searchval(uint32_t value, binaryTree*& foundptr) {
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


    static binaryTree* insertRecursive(binaryTree* node, uint32_t value) {
        if(node == nullptr) {
            return new binaryTree{nullptr, nullptr, node, value};
        }

        if(value < node->m_data) {
            node->m_left = insertRecursive(node->m_left, value);
        } else if(value > node->m_data) {
            node->m_right = insertRecursive(node->m_right, value);
        }


        node->m_height = computeHeight(node);
        return balanceIfRequired(node);
    }


    static binaryTree* deleteRecursive(binaryTree* node, uint32_t value)
    {
        if(node == nullptr) {
            return nullptr;
        }
        
        if(value < node->m_data) {
            node->m_left = deleteRecursive(node->m_left, value);
        } else if(value > node->m_data) {
            node->m_right = deleteRecursive(node->m_right, value);
        } else { /* We found the node */
            // if(isLeaf(node)) {
            //     delete node;
            // }
            if(singleChildParent(node)) {
                if(node->m_left == nullptr) {
                    
                }
            }
        }

        node->m_height = computeHeight(node);
        return balanceIfRequired(node);
    }


    static binaryTree* balanceIfRequired(binaryTree* node) {
        auto bf = computeBalanceFactor(node);
        auto bfleft  = computeBalanceFactor(node->m_left);
        auto bfright = computeBalanceFactor(node->m_right);
        binaryTree* newRoot = nullptr;

        if(bf == 0 || bf == -1 || bf == +1) {
            return node; /* do nothing */
        }


        if(bf == -2) {
            if(bfright < 0) { /* Left Left Case */
                newRoot = rotateRight(node);
            } else { /* Left Right Case */
                node->m_left = rotateLeft(node->m_left);
                newRoot = rotateRight(node);
            }
        } else if(bf == +2) {
            if(bfright >= 0) { /* Right Right Case */
                newRoot = rotateLeft(node);
            } else { /* Right Left Case */
                node->m_right = rotateRight(node->m_right);
                newRoot = rotateLeft(node);
            }
        }

        return newRoot;
    }




    static binaryTree* insertIterative(binaryTree* node, uint32_t val) {
        binaryTree* search = node;
        binaryTree* allocNode = nullptr;
        std::deque<binaryTree*> nodesTouched;


        node->searchval(val, search);
        if(search != nullptr) {
            return node;
        }
        if(node == nullptr) {
            return new binaryTree{val};
        }


        search    = node;
        allocNode = new binaryTree{val};
        for(; search != nullptr ;) {
            nodesTouched.push_back(search);
            search = (val < search->m_data) ? search->m_left : search->m_right;
        }


        bool   inserted = false;
        int8_t bf = 0, bfright = 0;
        binaryTree* maybeNewRoot = nullptr;
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


            currentParent->m_height = computeHeight(currentParent);
            currentParent->m_bf     = computeBalanceFactor(currentParent);
            auto bfright = computeBalanceFactor(currentParent->m_right);
            auto bfleft  = computeBalanceFactor(currentParent->m_left);
            // if(currentParent->m_bf == -2) {
            //     if(bfleft == -1) { /* Left Left Case */
            //         maybeNewRoot = rotateRight(currentParent);
            //     } else if(bfleft == +1) { /* Left Right Case */
            //         currentParent->m_left = rotateLeft(currentParent->m_left);
            //         maybeNewRoot = rotateRight(currentParent);
            //     }
            // } else if(currentParent->m_bf == +2) {
            //     if(bfright == +1) { /* Right Right Case */
            //         maybeNewRoot = rotateLeft(currentParent);
            //     } else if(bfright == -1) { /* Right Left Case */
            //         currentParent->m_right = rotateRight(currentParent->m_right);
            //         maybeNewRoot = rotateLeft(currentParent);
            //     }
            // }

            /* [NOTE]: Test Code Lines 359-389 */
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


        return maybeNewRoot;
    }


    static bool deleteIterative(binaryTree* node, uint32_t value, binaryTree** out) {
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
        binaryTree  deletedNodeCopy = *nodesTouched.back(); /* nodesTouched.back() will return the element to be deleted, if found == true. */
        /* 
            TODO: Optimize the shit outta this, 
            while this works, and might be a little clever at best,
            has way too many branching (especially the expensive path)
        */
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
                else if(binaryTree::singleChildParent(currentParent)) {
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
                    ifcrashstr(successorNode == nullptr, "There should always be a successor node available\n");

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
};


struct AVLTree {
    binaryTree* m_root      = nullptr;
    uint32_t    m_nodeCount = 0;

    bool isEmpty() const {
        return m_nodeCount == 0;
    }

    size_t size() const {
        return m_nodeCount;
    }


    auto const* getRoot() const {
        return m_root;
    }

    auto const* getLeftChild() const {
        return m_root->m_left;
    }

    auto const* getRightChild() const {
        return m_root->m_left;
    }


    void clear() noexcept {
        m_root->destroy();
        m_root = nullptr;
        m_nodeCount = 0;
        return;
    }


    bool insert(uint32_t val) {
        binaryTree* found = m_root;
        
        if(m_root == nullptr) {
            m_root = binaryTree::insertIterative(m_root, val);
            ++m_nodeCount;
            return true;
        }


        m_root->searchval(val, found);
        if(found != nullptr) {
            return false;
        }
        m_root = binaryTree::insertIterative(m_root, val);
        ++m_nodeCount;
        return true;
    }


    bool remove(uint32_t val) {
        // m_root = binaryTree::deleteRecursive(m_root, val);
        if(m_root == nullptr) {
            return false;
        }

        binaryTree* newRootMaybe = nullptr;
        bool        status       = binaryTree::deleteIterative(m_root, val, &newRootMaybe);
        
        m_root = newRootMaybe != nullptr ? newRootMaybe : m_root;
        return status;
    }


    bool search(uint32_t val) {
        binaryTree* found = m_root;
        binaryTree::searchval(val, found);
        return found ? true : false;
    }


    bool isValidBST() const noexcept {
        return binaryTree::isValidBST(m_root);
    }

    bool isBalanced() const noexcept {
        return binaryTree::isValidAVL(m_root);
    }


    // void print() const {
    //     uint32_t currentLevelSize = 0;
    //     std::queue<binaryTree*> currentHeightNodes;


    //     /* 
    //         Iterative Level Order Traversal 
    //         top node popped has already pushed its children to the queue
    //     */
    //     currentHeightNodes.push(m_root);
    //     while(!currentHeightNodes.empty()) {
    //         currentLevelSize = currentHeightNodes.size();

    //         while(currentLevelSize) {
    //             auto node = currentHeightNodes.front();
    //             if(node->m_left) {
    //                 currentHeightNodes.push(node->m_left);
    //             }

    //             if(node->m_right) {
    //                 currentHeightNodes.push(node->m_right);
    //             }

    //             currentHeightNodes.pop();
    //             --currentLevelSize;
    //         }
    //     }


    //     return;
    // }

    void printRecursive(binaryTree* root, int space) {
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
};

