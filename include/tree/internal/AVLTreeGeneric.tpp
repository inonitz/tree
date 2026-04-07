#ifndef __AVL_TREE_GENERIC_IMPL_HEADER__
#   define __AVL_TREE_GENERIC_IMPL_HEADER__
#   ifndef __AVL_TREE_GENERIC_DEFINITION_HEADER__
#       include <tree/internal/AVLTreeGeneric.hpp>
#       include <tree/internal/binaryTreeGeneric.hpp>
#   endif /* __BINARY_TREE_GENERIC_DEFINITION_HEADER__ */
#include <cstdio>
#include <cassert>
#include <stack>
#include <utility>


template<typename T>
void AVLTree<T>::clear() noexcept {
    binaryTree<T>::destroy(m_root);
    m_root = nullptr;
    m_nodeCount = 0;
    return;
}

template<typename T>
void AVLTree<T>::copy(AVLTree const& other)
{
    if(other.empty()) {
        return;
    }
    if(!empty()) {
        clear();
    }
    binaryTree<T>::deepCopy(other.m_root, &m_root);
    m_nodeCount = other.m_nodeCount;
    return;
}


template<typename T>
bool AVLTree<T>::insert(T const& val) {
    binaryTree<T>* newRootMaybe = nullptr;
    bool           status       = binaryTree<T>::AVLInsertIterative(m_root, val, &newRootMaybe);
    
    m_root       = newRootMaybe != nullptr ? newRootMaybe : m_root;
    m_nodeCount += status;
    return status;
}

template<typename T>
bool AVLTree<T>::remove(T const& val) {
    binaryTree<T>* newRootMaybe = nullptr;
    bool           status       = binaryTree<T>::AVLDeleteIterative(m_root, val, &newRootMaybe);


    if(status == true) {
        m_root = newRootMaybe;
        --m_nodeCount;
        return true;
    }
    return false;
}

template<typename T>
bool AVLTree<T>::search(T const& val) const noexcept {
    if(empty()) {
        return false;
    }
    return (binaryTree<T>::searchIterative(m_root, val) != nullptr);
}

template<typename T>
bool AVLTree<T>::compare(AVLTree const& other) const {
    if(empty() && other.empty()) {
        return true;
    }
    if(m_nodeCount != other.m_nodeCount) {
        return false;
    }


    return binaryTree<T>::compareIterative(m_root, other.m_root);
}


template<typename T>
bool AVLTree<T>::insertRecursive(T const& val)
{
    if(searchRecursive(val) == true) {
        return false;
    }
    m_root = binaryTree<T>::AVLInsertRecursive(m_root, nullptr, val);
    ++m_nodeCount;
    return true;
}

template<typename T>
bool AVLTree<T>::removeRecursive(T const& val)
{
    if(searchRecursive(val) == false) {
        return false;
    }
    m_root = binaryTree<T>::AVLDeleteRecursive(m_root, nullptr, val);
    --m_nodeCount;
    return true;
}

template<typename T>
bool AVLTree<T>::searchRecursive(T const& val) const noexcept
{
    return binaryTree<T>::searchRecursive(m_root, val);
}

template<typename T>
bool AVLTree<T>::compareRecursive(AVLTree const& other) const
{
    return binaryTree<T>::compareRecursive(m_root, other.m_root);
}


template<typename T> [[nodiscard]] bool AVLTree<T>::isValidBST() const noexcept {
    binaryTree<T>* prev = nullptr;
    return binaryTree<T>::isValidBSTRecursive(m_root, &prev);
}

template<typename T> [[nodiscard]] bool AVLTree<T>::isBalanced() const noexcept {
    return binaryTree<T>::isValidAVL(m_root);
}

template<typename T> [[nodiscard]] bool AVLTree<T>::empty() const noexcept {
    return m_nodeCount == 0;
}
template<typename T> [[nodiscard]] uint64_t AVLTree<T>::size() const noexcept {
    return m_nodeCount;
}
template<typename T> [[nodiscard]] int8_t AVLTree<T>::height() const noexcept {
    return m_root->m_height;
}
template<typename T> [[nodiscard]] auto AVLTree<T>::getRoot() const -> binaryTree<T> const* {
    return m_root;
}
template<typename T> [[nodiscard]] auto AVLTree<T>::getLeftChild() const -> binaryTree<T> const* {
    if(m_root == nullptr) {
        return nullptr;
    }    
    return m_root->m_left;
}
template<typename T> [[nodiscard]] auto AVLTree<T>::getRightChild() const -> binaryTree<T> const* {
    if(m_root == nullptr) {
        return nullptr;
    }    
    return m_root->m_right;
}

template<typename T>
template<typename Functor> 
void AVLTree<T>::printRecursive(const binaryTree<T>* node, uint32_t space, Functor const& printTypename) const
{
    constexpr auto kCOUNT = 10;
    
    if (node == NULL) {
        return;
    }
    space += kCOUNT;
    
    
    printRecursive(node->m_right, space, printTypename);
    printf("\n%*s", space - kCOUNT, "");
    printTypename(node->m_data.get());
    printf(" (%u, %d)\n", node->m_height, node->m_bf);
    printRecursive(node->m_left, space, printTypename);
    return;
}

template<typename T>
template<typename Functor> 
void AVLTree<T>::printIterative(const binaryTree<T>* root, uint32_t space, Functor const& printTypename) const
{
    constexpr auto kCOUNT = 8;
    
    
    if (root == nullptr) {
        return;
    }
    using TreeIdentPair = std::pair<const binaryTree<T>*, uint32_t>;


    std::stack<TreeIdentPair> s;
    const binaryTree<T>* curr = root;
    uint32_t currentSpace = space;
    while (curr != nullptr || !s.empty()) {
        while (curr != nullptr) {
            currentSpace += kCOUNT;
            s.push({curr, currentSpace});
            curr = curr->m_right;
        }


        TreeIdentPair top = s.top();
        s.pop();
        
        curr = top.first;
        uint32_t currSpace = top.second;

        printf("\n%*s", currSpace - kCOUNT, "");
        printTypename(curr->m_data.get());
        printf(" (%u, %d)\n", curr->m_height, curr->m_bf);

        curr = curr->m_left;
        currentSpace = currSpace;
    }


    return;
}


#endif /* __BINARY_TREE_GENERIC_IMPL_HEADER__ */
