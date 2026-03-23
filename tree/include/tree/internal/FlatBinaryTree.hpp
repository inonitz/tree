#ifndef __FLAT_BINARY_TREE_GENERIC_DEFINITION_HEADER__
#define __FLAT_BINARY_TREE_GENERIC_DEFINITION_HEADER__
#include <tree/internal/bitmap.hpp>


template <typename T>
class FlatBinaryTree 
{
public:
    void setNode(uint64_t index, const T& val);
    void setRoot(const T& val);
    void setLeft(uint64_t parent, const T& val);
    void setRight(uint64_t parent, const T& val);
    void remove(uint64_t i);
    bool exists(uint64_t i) const;

private:
    std::vector<T> m_nodes;
    Bitmap         m_activeNodes;

    void ensure_capacity(uint64_t i);
};


#endif /* __FLAT_BINARY_TREE_GENERIC_DEFINITION_HEADER__ */
