#ifndef __FLAT_BINARY_TREE_GENERIC_IMPLEMENTATION_HEADER__
#   define __FLAT_BINARY_TREE_GENERIC_IMPLEMENTATION_HEADER__
#   ifndef __FLAT_BINARY_TREE_GENERIC_DEFINITION_HEADER__
#       include <tree/internal/FlatBinaryTree.hpp>
#   endif /* __FLAT_BINARY_TREE_GENERIC_DEFINITION_HEADER__ */


template <typename T>
void FlatBinaryTree<T>::setNode(uint64_t index, const T& val) {
    ensure_capacity(index);
    m_nodes[index] = val;
    m_activeNodes.toggle(index);
    return;
}

template <typename T>
void FlatBinaryTree<T>::setRoot(const T& val) { 
    setNode(0, val);
    return;
}


template <typename T>
void FlatBinaryTree<T>::setLeft(uint64_t parent, const T& val) {
    if (!exists(parent)) return;
    setNode((parent * 2) + 1, val);
}

template <typename T>
void FlatBinaryTree<T>::setRight(uint64_t parent, const T& val) {
    if (!exists(parent)) return;
    setNode((parent * 2) + 2, val);
}

template <typename T>
void FlatBinaryTree<T>::remove(uint64_t i) {
    if(!exists(i)) {
        return;
    }

    m_activeNodes.clear(i);
    std::memset(&m_nodes[i], 0x00, sizeof(T));
    return;
}

template <typename T>
bool FlatBinaryTree<T>::exists(uint64_t i) const {
    return i < m_activeNodes.sizeInBits() && m_activeNodes[i];
}

// template <typename T>
// void FlatBinaryTree<T>::debug_print() const {
//     for (size_t i = 0; i < m_nodes.size(); ++i) {
//         if (exists(i)) std::cout << "Idx " << i << ": " << m_nodes[i] << "\n";
//         else           std::cout << "Idx " << i << ": [EMPTY]\n";
//     }
// }

template <typename T>
void FlatBinaryTree<T>::ensure_capacity(uint64_t i) {
    if(i < m_nodes.size()) {
        return;
    }

    const uint64_t new_treeSize = 
    if (i >= m_nodes.size()) {
        m_nodes.resize(i + 1);
        m_activeNodes.resize(i + 1);
    }
}


#endif /* __FLAT_BINARY_TREE_GENERIC_IMPLEMENTATION_HEADER__ */
