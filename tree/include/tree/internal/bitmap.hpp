#ifndef __BITMAP_DEFINITION_HEADER__
#define __BITMAP_DEFINITION_HEADER__
#include <vector>
#include <cstdint>


namespace flat_avl_tree_internal {


class Bitmap 
{
public:
    explicit Bitmap(uint64_t size = 8);
    ~Bitmap() = default;


    bool operator[](uint64_t bitIndex) const;
    bool at    (uint64_t bitIndex) const;
    void toggle(uint64_t bitIndex);
    void clear (uint64_t bitIndex);
    void flip  (uint64_t bitIndex);

    void toogle_if(uint64_t index, bool condition);
    void clear_if (uint64_t index, bool condition);
    void flip_if  (uint64_t index, bool condition);

    void     resize(uint64_t new_size);
    void     clear() noexcept;
    uint64_t sizeInBits() const;
    uint64_t sizeInBytes() const;

private:
    uint64_t m_bitCount = 0;
    std::vector<uint64_t> m_storage;

    constexpr uint64_t indexBitmask(uint64_t bitIndex) const {
        constexpr uint64_t uint64_size_bits_minus_one = 8 * sizeof(uint64_t) - 1;
        /* Limit the bitIndex between 0->63 then return a single-bit bitmask */
        return 1ull << (bitIndex & uint64_size_bits_minus_one);
    }

    constexpr uint64_t qwordIndex(uint64_t bitIndex) const {
        /* constexpr uint64_t discardLowEndianFirstSixBits = 0xffffffffffffffc0; */
        /* return bitIndex & discardLowEndianFirstSixBits; */
        return bitIndex >> 6;
    }
};


} /* namespace flat_avl_tree_internal */


#endif /* __BITMAP_DEFINITION_HEADER__ */
