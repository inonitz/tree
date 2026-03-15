#ifndef __BINARY_TREE_GENERIC_DEFINITION_HEADER__
#define __BINARY_TREE_GENERIC_DEFINITION_HEADER__
#include <tree/C/treelib_api.h>
#include <tree/internal/treelib_util.hpp>
#include <cstdint>
#include <deque>


template<typename T>
class TREELIB_NO_EXPORT binaryTree 
{
    static_assert(internal::has_less_v<T>   , "type T MUST have an overload for operator<\n");
    static_assert(internal::has_greater_v<T>, "type T MUST have an overload for operator>\n");
    static_assert(internal::has_equal_v<T>  , "type T MUST have an overload for operator==\n");    
private:
    using valueType = typename internal::CompactifiedType<T>;
    using classPtr  = binaryTree*;
    
    static constexpr auto reservedBytesSize() -> uint64_t {
        auto currentClassSize = 3 * sizeof(classPtr) + 2 + sizeof(valueType);
        auto alignTo8Multiple = 8 * ( (currentClassSize / 8) + (currentClassSize % 8 > 0) );

        /* Return whatever difference is left - that is how much is required for an 8 byte multiple */
        return alignTo8Multiple - currentClassSize;
    }

    constexpr uint64_t nodeSize() const {
        return sizeof(binaryTree<T>);
    }

public:
    static int8_t      computeHeight(binaryTree* node) noexcept;
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
    explicit binaryTree(T const& value);
    binaryTree(
        binaryTree* left, 
        binaryTree* right, 
        binaryTree* parent, 
        T const&    value
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

    /* Utillity Functions */
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
    static void searchval(T const& value, binaryTree*& foundptr);
    static bool AVLinsertIterative(binaryTree* node, T const& value, binaryTree** out);
    static bool AVLdeleteIterative(binaryTree* node, T const& value, binaryTree** out);


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
    valueType m_data;
    int8_t    m_height = 0;
    int8_t    m_bf     = 0;
    uint8_t   m_reserved[reservedBytesSize()]{0};
};


#endif /* __BINARY_TREE_GENERIC_DEFINITION_HEADER__ */
