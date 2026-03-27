#ifndef __FLAT_AVL_TREE_NODE_METADATA_DEFINITION_HEADER__
#define __FLAT_AVL_TREE_NODE_METADATA_DEFINITION_HEADER__
#include <cstdint>


namespace flat_avl_tree_internal {


/* Utility Type to represent Array-Indexing up-to -1 + (1 << 28), i.e ~268 Million Elements. */
struct Metadata {
public:
    Metadata() {
        setHeight(0);
        setLeftChild(k_nullIndex);
        setRightChild(k_nullIndex);
        return;
    }

    Metadata(
        uint32_t left, 
        uint32_t right, 
        int8_t   height
    ) {
        setHeight(height);
        setLeftChild(left);
        setRightChild(right);
        return;
    }

    uint32_t getConditionalChild(bool trueLeftFalseRight) const noexcept {
        const uint64_t bitMask  = trueLeftFalseRight ? k_leftChildBitmask  : k_rightChildBitmask;
        const uint32_t bitShift = trueLeftFalseRight ? k_leftChildBitShift : k_rightChildBitShift;
        return (m_data & bitMask) >> bitShift;
    }

    uint32_t getLeftChild() const noexcept {
        return static_cast<uint32_t>( (m_data & k_leftChildBitmask) >> k_leftChildBitShift );
    }
    uint32_t getRightChild() const noexcept {
        return static_cast<uint32_t>( (m_data & k_rightChildBitmask) >> k_rightChildBitShift );
    }
    int8_t getHeight() const noexcept {
        return static_cast<int8_t>( 
            static_cast<uint8_t>(m_data >> k_heightBitShift) 
        );
    }


    void setConditionalChild(bool trueLeftFalseRight, uint32_t index) noexcept {
        const uint64_t bitmask  = trueLeftFalseRight ? k_leftChildBitmask : k_rightChildBitmask;
        const uint64_t bitshift = trueLeftFalseRight ? k_leftChildBitShift : k_rightChildBitShift; 

        const uint64_t normIdx = static_cast<uint64_t>(index & k_indexBitmask) << bitshift;
        m_data = (m_data & ~bitmask) | normIdx;
        return;
    }

    void setLeftChild(uint32_t index) noexcept {
        const uint64_t normIdx = index & k_indexBitmask;
        m_data = (m_data & ~k_leftChildBitmask) | (normIdx << k_leftChildBitShift);
        return;
    }
    void setRightChild(uint32_t index) noexcept {
        const uint64_t normIdx = index & k_indexBitmask;
        m_data = (m_data & ~k_rightChildBitmask) | (normIdx << k_rightChildBitShift);
        return;
    }
    void setHeight(int8_t height) noexcept {
        /* casting to uint8_t will preserve the original bit representation, casting to uint64_t just extends */
        const uint64_t normHeight = static_cast<uint64_t>(
            static_cast<uint8_t>(height)
        ) << k_heightBitShift;

        m_data = (m_data & ~k_heightBitmask) | normHeight;
        return;
    }

private:
    static constexpr uint32_t k_bitsPerIndex = 28;
    static constexpr uint32_t k_indexBitmask = 0x0fffffff;

    static constexpr uint64_t k_leftChildBitmask  = 0x000000000fffffff;
    static constexpr uint64_t k_rightChildBitmask = 0x00fffffff0000000;
    static constexpr uint64_t k_heightBitmask     = 0xff00000000000000;


    static constexpr uint32_t k_leftChildBitShift  = 0;
    static constexpr uint32_t k_rightChildBitShift = k_bitsPerIndex;
    static constexpr uint32_t k_heightBitShift     = 2 * k_bitsPerIndex;

    uint64_t m_data;
    /* The Data Layout on a Little Endian Machine, reserve for Big endian */
    // union {
    //     struct {
    //         uint8_t m_childIndices[7];
    //         int8_t  m_height;
    //     };
    // };
public:
    static constexpr uint32_t k_nullIndex = k_indexBitmask;
};


} /* namespace flat_avl_tree_internal */


#endif /* __FLAT_AVL_TREE_NODE_METADATA_DEFINITION_HEADER__ */
