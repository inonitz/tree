#ifndef __FLAT_AVL_TREE_GENERIC_MEMBER_ACCESS_DEFINITION_HEADER__
#define __FLAT_AVL_TREE_GENERIC_MEMBER_ACCESS_DEFINITION_HEADER__
#include <tree/FlatAVLTree.hpp>
#include <utility>


namespace flat_avl_tree_internal {


template<typename T> class FlatAVLTreeTestingMemberAccess
{
private:
    friend class FlatAVLTree<T>;
    using AllocationIndexType = decltype(std::declval<FlatAVLTree<T>>().allocateNode());
public:
    explicit FlatAVLTreeTestingMemberAccess(FlatAVLTree<T>& tree) : m_toAccess(tree) {}

    auto rootIndex() const { return m_toAccess.m_rootIdx; }
    auto& valueRW(uint32_t index)    { return m_toAccess.getNodeValue(index);    }
    auto& metadataRW(uint32_t index) { return m_toAccess.getNodeMetadata(index); }
    auto const& valueRO(uint32_t index)    const { return m_toAccess.readValue(index);    }
    auto const& metadataRO(uint32_t index) const { return m_toAccess.readMetadata(index); }
    
    AllocationIndexType allocateNode() {
        return m_toAccess.allocateNode();
    }

    void freeNode(AllocationIndexType idx) {
        m_toAccess.freeNode(idx);
        return;
    }

    FlatAVLTree<T>& tree() { return m_toAccess; }

private:
    FlatAVLTree<T>& m_toAccess;
};


} /* namespace flat_avl_tree_internal */


#endif /* __FLAT_AVL_TREE_GENERIC_MEMBER_ACCESS_DEFINITION_HEADER__*/
