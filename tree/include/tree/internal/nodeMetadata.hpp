#ifndef __FLAT_AVL_TREE_NODE_METADATA_DEFINITION_HEADER__
#define __FLAT_AVL_TREE_NODE_METADATA_DEFINITION_HEADER__
#include <cstdint>


namespace flat_avl_tree_internal {


/* Utility Type to represent Array-Indexing up-to -1 + (1 << 28), i.e ~268 Million Elements. */
struct Metadata {
private:
    static constexpr uint32_t k_bitsPerIndex = 28;
    static constexpr uint32_t k_indexBitmask  = 0x0fffffff;
    static constexpr uint64_t k_leftChildBitmask  = 0x000000000fffffff;
    static constexpr uint64_t k_rightChildBitmask = 0x00fffffff0000000;
    static constexpr uint32_t k_rightChildBitShift = k_bitsPerIndex;

    union {
        uint64_t m_data;
        struct {
            uint8_t m_childIndices[7];
            int8_t  m_height;
        };
    };

public:
    Metadata(
        uint32_t left   = 0, 
        uint32_t right  = 0, 
        int8_t   height = 0
    ) {
        setHeight(height);
        setLeftChild(left);
        setRightChild(right);
        return;
    }

    uint32_t getConditionalChild(bool trueLeftFalseRight) const noexcept {
        const uint64_t bitMask  = trueLeftFalseRight ? k_leftChildBitmask : k_rightChildBitmask;
        const uint32_t bitShift = trueLeftFalseRight ? 0                  : k_rightChildBitShift;
        return (m_data & bitMask) >> bitShift;
    }

    uint32_t getLeftChild() const noexcept {
        return (m_data & k_leftChildBitmask);
    }
    uint32_t getRightChild() const noexcept {
        return (m_data & k_rightChildBitmask) >> k_rightChildBitShift;
    }
    int8_t getHeight() const noexcept {
        return m_height;
    }
    void setLeftChild(uint32_t index) noexcept {
        uint64_t normIdx = index & k_indexBitmask;
        m_data = (m_data & ~k_leftChildBitmask) | normIdx;
        return;
    }
    void setRightChild(uint32_t index) noexcept {
        uint64_t normIdx = index & k_indexBitmask;
        m_data = (m_data & ~k_rightChildBitmask) | normIdx;
        return;
    }
    void setHeight(int8_t height) noexcept {
        m_height = height;
        return;
    }
};


} /* namespace flat_avl_tree_internal */


#endif /* __FLAT_AVL_TREE_NODE_METADATA_DEFINITION_HEADER__ */
