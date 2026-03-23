#ifndef __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__
#define __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__
#include <tree/C/treelib_api.h>
#include <tree/C/dynamic_pool.h>
#include <tree/internal/bitmap.hpp>
#include <tree/internal/nodeMetadata.hpp>
#include <stack>


template<typename T>
class TREELIB_API FlatAVLTree 
{
private:
    /* Utility Types */
    using u8 = uint8_t;
    using i8 = int8_t;
    enum class NodeState : uint8_t {
        UNSET   = 0,
        SET     = 1,
        INVALID = 2
    };
    using nodeMetadata = flat_avl_tree_internal::Metadata;
    using nodeBitmap   = flat_avl_tree_internal::Bitmap;


    /* Utility Functions */
    constexpr uint32_t leftNodeIndex(uint32_t currNodeIndex) {
        return 2 * currNodeIndex + 1;
    }
    constexpr uint32_t rightNodeIndex(uint32_t currNodeIndex) {
        return 2 * currNodeIndex + 2;
    }
    constexpr uint32_t pickChildIndex(uint32_t currNodeIndex, bool valueLessThanCurrNode) {
        return 2 * currNodeIndex + !valueLessThanCurrNode;
    }
    constexpr uint32_t rootNodeIndex() {
        return 0;
    }


    T const&  getNodeValue(uint32_t index);
    void      setNode(uint32_t index, T const& val);
    bool      nodeExists(uint32_t index) const;
    NodeState nodeState(uint32_t index) const;
    nodeMetadata& getNodeMetadata(uint32_t index);

    int8_t   getNodeHeight(uint32_t nodeIndex) noexcept;
    int8_t   getNodeBalanceFactor(uint32_t nodeIndex) noexcept;
    
    bool searchAndPushParents(
        uint32_t nodeIndex, 
        T const& valueToSearch, 
        std::stack<uint32_t>& nodeIdxStack
    );
    void rebalance(uint32_t nodeIndex, uint32_t* newRootIndex);
    void resize_on_demand();
    
public:
    FlatAVLTree()  = default;
    ~FlatAVLTree() = default;


    void clear() noexcept;
    bool insertOld(T const& val);
    bool insert(T const& val);
    bool remove(T const& val);
    bool search(T const& val);

    // bool insertRecursive(T const& val);
    // bool removeRecursive(T const& val);
    // bool searchRecursive(T const& val);


    [[nodiscard]] bool isValidBST() const noexcept;
    [[nodiscard]] bool isBalanced() const noexcept;
    [[nodiscard]] bool     empty()  const;
    [[nodiscard]] uint32_t size()   const;
    [[nodiscard]] int8_t   height() const;
    template<typename Functor> void print(
        uint32_t       space, 
        Functor const& printTypename
    ) const;

private:
    std::vector<T>            m_nodeVal;
    genericDynamicPool        m_metadataPool;
    nodeBitmap                m_activeNodes; /* Bitmap can be incorported into m_nodeMeta (using a single-bit inside m_height) */
    uint32_t                  m_nodeCount = 0;
    uint8_t                   m_reserved[4]{0};
};


#endif /* __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__ */
