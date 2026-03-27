#ifndef __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__
#define __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__
#include <tree/C/treelib_api.h>
#include <tree/internal/nodeMetadata.hpp>
#include <vector>
#include <stack>


namespace flat_avl_tree_internal {
    template<typename T, bool IsConst> class PreOrderIterator;
    template<typename T, bool IsConst> class InOrderIterator;
    template<typename T, bool IsConst> class PostOrderIterator;
    template<typename T, bool IsConst> class LevelOrderIterator;
}


template<typename T>
class TREELIB_API FlatAVLTree 
{
private:
    template<typename U, bool IsConst> friend class flat_avl_tree_internal::PreOrderIterator;
    template<typename U, bool IsConst> friend class flat_avl_tree_internal::InOrderIterator;
    template<typename U, bool IsConst> friend class flat_avl_tree_internal::PostOrderIterator;
    template<typename U, bool IsConst> friend class flat_avl_tree_internal::LevelOrderIterator;

    /* Utility Types */
    using u8 = uint8_t;
    using i8 = int8_t;
    enum class NodeState : uint8_t {
        UNSET   = 0,
        SET     = 1,
        INVALID = 2
    };
    using nodeMetadata = flat_avl_tree_internal::Metadata;


    T&            getNodeValue(uint32_t index);
    nodeMetadata& getNodeMetadata(uint32_t index);
    T const&            readValue(uint32_t index) const;
    nodeMetadata const& readMetadata(uint32_t index) const;
    bool          nodeExists(uint32_t nodeIndex) const noexcept;
    bool          nodeIsFull(uint32_t nodeIndex) const noexcept;
    int8_t        computeHeight(uint32_t nodeIndex) const noexcept;
    int8_t        computeBalanceFactor(uint32_t nodeIndex) const noexcept;
    
    bool searchAndPushParents(
        uint32_t nodeIndex, 
        T const& valueToSearch, 
        std::stack<uint32_t>& nodeIdxStack
    );
    uint32_t findMaxAndPushParents(
        uint32_t nodeIdx,
        std::stack<uint32_t>& nodeIdxStack
    );
    uint32_t findMinAndPushParents(
        uint32_t nodeIdx,
        std::stack<uint32_t>& nodeIdxStack
    );
    uint32_t removeNodeAndLinkParentWithChild(uint32_t nodeIdx, uint32_t nodeParentIdx);

    uint32_t rotateLeft(uint32_t nodeIdx, uint32_t parentIdx);
    uint32_t rotateRight(uint32_t nodeIdx, uint32_t parentIdx);
    void     rebalance(uint32_t nodeIdx, uint32_t nodeParentIdx, uint32_t* newRootIndex);
    
    void     resize();
    uint32_t allocateNode();
    void     freeNode(uint32_t nodeIdx);

public:
    FlatAVLTree()  = default;
    ~FlatAVLTree() = default;


    void clear() noexcept;
    bool insert(T const& val);
    bool remove(T const& val);
    bool search(T const& val);

    // bool insertRecursive(T const& val);
    // bool removeRecursive(T const& val);
    // bool searchRecursive(T const& val);

    auto pre_order_range() noexcept;
    auto pre_order_range() const noexcept;
    auto in_order_range() noexcept;
    auto in_order_range() const noexcept;
    auto post_order_range() noexcept;
    auto post_order_range() const noexcept;
    auto level_order_range() noexcept;
    auto level_order_range() const noexcept;

    [[nodiscard]] bool isValidBST() const noexcept;
    [[nodiscard]] bool isBalanced() const noexcept;
    [[nodiscard]] bool     empty()  const noexcept;
    [[nodiscard]] uint32_t size()   const noexcept;
    [[nodiscard]] int8_t   height() const;
    template<typename OutputStream> OutputStream& print(
        uint32_t      space, 
        OutputStream& outputBuf
    ) const;

private:
    std::vector<T>            m_nodeVal;
    std::vector<nodeMetadata> m_nodeMetadata;
    std::stack<uint32_t>      m_freedNodes;
    uint32_t                  m_freeNodeIdx = 0;
    uint32_t                  m_rootIdx     = nodeMetadata::k_nullIndex;
};


#endif /* __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__ */
