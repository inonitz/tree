#pragma once
#include <tree/C/treelib_api.h>
#include <cstdint>
#include <deque>


class TREELIB_NO_EXPORT binaryTree 
{
public:
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


    static uint8_t     computeHeight(binaryTree* node) noexcept;
    static int8_t      computeBalanceFactor(binaryTree* node) noexcept;
    static binaryTree* findMaxAndPushParents(binaryTree* node, std::deque<binaryTree*>& parentQueue);
    static binaryTree* findMinAndPushParents(binaryTree* node, std::deque<binaryTree*>& parentQueue);
    static bool        isValidAVL_InternalRecursive(binaryTree* node);
    static uint32_t    writeTreeToBufferRecursive(
        char*       outputBuf, 
        uint32_t    outputBufIdx, 
        binaryTree* root, 
        uint32_t    space
    );

public:
    binaryTree() = default;
    explicit binaryTree(uint32_t value);
    binaryTree(
        binaryTree* left, 
        binaryTree* right, 
        binaryTree* parent, 
        uint32_t    value
    );

    binaryTree& operator=(const binaryTree& other) = delete;
    binaryTree& operator=(binaryTree&& other)      = delete;
    binaryTree(const binaryTree& other)            = delete;
    binaryTree(binaryTree&& other)                 = delete;

    ~binaryTree() = default;

    static void destroy(binaryTree* node) noexcept;
    static void deepCopy(
        binaryTree* nodeIn, 
        binaryTree* nodeOut
    );
    static void shallowCopy(
        binaryTree* nodeIn,
        binaryTree* nodeOut
    );
    static void move(
        binaryTree* toMoveFrom,
        binaryTree* toMoveTo
    );

    /* Utility Functions */
    static bool        isLeaf(binaryTree* node)              noexcept;
    static bool        isSingleChildParent(binaryTree* node) noexcept;
    static binaryTree* getLeft(binaryTree* node)             noexcept;
    static binaryTree* getRight(binaryTree* node)            noexcept;

    
    /* Functions that enable balancing in the Binary Tree */
    static binaryTree* rotateLeft(binaryTree* node);
    static binaryTree* rotateRight(binaryTree* node);
    static binaryTree* findMax(binaryTree* node);
    static binaryTree* findMin(binaryTree* node);

    /* Sanity Checks for testing */
    static bool isValidBSTRecursive(binaryTree* node);
    static bool isValidAVL(binaryTree* node);
    static void writeBufferRecursive(
        char*       outputBuf, 
        uint32_t    outputBufIdx, 
        binaryTree* root, 
        uint32_t    space
    );

    /* Regular BST Search With AVLTree Insertion & Deletion */
    static void searchval(uint32_t value, binaryTree*& foundptr);
    static bool AVLinsertIterative(binaryTree* node, uint32_t val, binaryTree** out);
    static bool AVLdeleteIterative(binaryTree* node, uint32_t value, binaryTree** out);
};
