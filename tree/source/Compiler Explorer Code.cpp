#include <cstdint>
#include <cstring>
#include <cstdio>
#include <queue>
#include <stack>



struct binaryTree {
    binaryTree* m_left   = nullptr;
    binaryTree* m_right  = nullptr;
    binaryTree* m_parent = nullptr;
    uint32_t    m_data   = 0;
    uint8_t     m_height = 0;
    uint8_t     m_reserved[3]{0};


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
        m_reserved{{0}}
        {}

    static bool isLeaf(binaryTree* node) {
        if(node == nullptr) {
            return false;
        }
        return node->m_left == nullptr && node->m_right == nullptr;
    }


    static bool singleChildParent(binaryTree* node) {
        if(node == nullptr) {
            return false;
        }
        return (node->m_left == nullptr && node->m_right != nullptr) 
                ||
                (node->m_left != nullptr && node->m_right == nullptr); 
    }


    static uint8_t computeHeight(binaryTree* node) {
        auto rh = node->m_right ? node->m_right->m_height : -1;
        auto lh = node->m_left  ? node->m_left->m_height  : -1;
        return 1 + std::max(rh, lh);
    }

    static int8_t computeBalanceFactor(binaryTree* node) {
        int64_t rh = node->m_right ? node->m_right->m_height : -1;
        int64_t lh = node->m_left  ? node->m_left->m_height  : -1;
        return rh - lh;
    }


    static binaryTree* rotateLeft(binaryTree* node) {

    }

    static binaryTree* rotateRight(binaryTree* node) {
        
    }


    void search(uint32_t value, binaryTree*& foundptr) {
        bool        foundcond = false;
        binaryTree* searchptr = nullptr;

        for(searchptr = foundptr; (searchptr != nullptr) && !foundcond; ) {
            foundcond = (searchptr->m_data == value);

            if(!foundcond) {
                searchptr = (m_data < searchptr->m_data) ? 
                    searchptr->m_left : 
                    searchptr->m_right;
            }
        }

        foundptr = searchptr;
        return;
    }


    // void searchWithAncestors(
    //     uint32_t    value, 
    //     binaryTree* foundptr, 
    //     std::stack<binaryTree*>& ancestors
    // ) {
    //     bool        found = false;
    //     binaryTree* search = nullptr;

    //     for(search = this; (search != nullptr) && !found; ) {
    //         found = (search->m_data == value);
    //         ancestors.push(search);

    //         if(!found) {
    //             search = (m_data < search->m_data) ? 
    //                 search->m_left : 
    //                 search->m_right;
    //         }
    //     }

    //     foundptr = search;
    //     return;
    // }


    // bool insertIterative(uint32_t value) {
    //     std::stack<binaryTree*> ancestorsToModify;
    //     binaryTree* valueptr = nullptr;


    //     searchWithAncestors(value, valueptr, ancestorsToModify);
    //     if(valueptr != nullptr) {
    //         return false;
    //     }


    //     for(; !ancestorsToModify.empty(); ) {

    //     }

    //     return true;
    // }


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



};


struct AVLTree {
    binaryTree* m_root;
    uint32_t    m_nodeCount;


    bool insertValue(uint32_t val) {
        binaryTree* found = nullptr;
        
        if(m_root == nullptr) {
            m_root = binaryTree::insertRecursive(m_root, val);
            return true;
        }


        m_root->search(val, found);
        if(found != nullptr) {
            return false;
        }
        m_root = binaryTree::insertRecursive(m_root, val);
        return true;
    }


    bool deleteValue(uint32_t val) {
        binaryTree* found = nullptr;

        m_root->search(val, found);
        if(found == nullptr) {
            return false;
        }

        m_root = binaryTree::deleteRecursive(m_root, val);
        return true;
    }
};


int main() {

}
