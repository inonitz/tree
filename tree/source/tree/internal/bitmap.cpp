#include <tree/internal/bitmap.hpp>
#include <stdexcept>


namespace flat_avl_tree_internal {


Bitmap::Bitmap(uint64_t size) : m_bitCount(size) {
    m_storage.resize((size + 63) / 64, 0);
    return;
}


bool Bitmap::operator[](uint64_t i) const {
    return m_storage[qwordIndex(i)] & indexBitmask(i);
}

bool Bitmap::at(uint64_t i) const {
    if (i >= m_bitCount) {
        throw std::out_of_range("Bitmap::at index out of range");
    }
    return (*this)[i];
}


void Bitmap::toggle(uint64_t i) 
{
    if (i >= m_bitCount) resize(i + 1);
    m_storage[qwordIndex(i)] |= indexBitmask(i);
}

void Bitmap::clear(uint64_t i) 
{
    if (i < m_bitCount) {
        m_storage[qwordIndex(i)] &= ~indexBitmask(i);
    }
}

void Bitmap::flip(uint64_t i) 
{
    if (i >= m_bitCount) resize(i + 1);
    m_storage[qwordIndex(i)] ^= indexBitmask(i);
}


void Bitmap::toogle_if(uint64_t index, bool condition)
{

}

void Bitmap::clear_if (uint64_t index, bool condition)
{

}

void Bitmap::flip_if  (uint64_t index, bool condition)
{

}



void Bitmap::resize(uint64_t new_size) {
    m_bitCount = new_size;
    m_storage.resize((new_size + 63) / 64, 0);
}
void Bitmap::clear() noexcept {
    m_bitCount = 0;
    m_storage.clear();
    return;
}
uint64_t Bitmap::sizeInBits() const { return m_bitCount; }
uint64_t Bitmap::sizeInBytes() const { return m_storage.size(); }


} /* namespace flat_avl_tree_internal */
