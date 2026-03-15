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
bool AVLTree<T>::insert(T const& val) {
    binaryTree<T>* newRootMaybe = nullptr;
    bool           status       = binaryTree<T>::AVLinsertIterative(m_root, val, &newRootMaybe);
    
    m_root       = newRootMaybe != nullptr ? newRootMaybe : m_root;
    m_nodeCount += status;
    return status;
}

template<typename T>
bool AVLTree<T>::remove(T const& val) {
    binaryTree<T>* newRootMaybe = nullptr;
    bool           status       = binaryTree<T>::AVLdeleteIterative(m_root, val, &newRootMaybe);


    if(status == true) {
        m_root = newRootMaybe;
        --m_nodeCount;
        return true;
    }
    return false;
}

template<typename T>
bool AVLTree<T>::search(T const& val) {
    if(empty()) {
        return false;
    }
    binaryTree<T>* found = m_root;
    binaryTree<T>::searchval(val, found);
    return found ? true : false;
}



template<typename T> [[nodiscard]] bool AVLTree<T>::isValidBST() const noexcept {
    return binaryTree<T>::isValidBSTRecursive(m_root);
}

template<typename T> [[nodiscard]] bool AVLTree<T>::isBalanced() const noexcept {
    return binaryTree<T>::isValidAVL(m_root);
}

template<typename T> [[nodiscard]] bool AVLTree<T>::empty() const {
    return m_nodeCount == 0;
}
template<typename T> [[nodiscard]] size_t AVLTree<T>::size() const {
    return m_nodeCount;
}
template<typename T> [[nodiscard]] size_t AVLTree<T>::height() const {
    return m_root->m_height;
}
template<typename T> [[nodiscard]] auto AVLTree<T>::getRoot() const -> binaryTree<T> const* {
    return m_root;
}
template<typename T> [[nodiscard]] auto AVLTree<T>::getLeftChild() const -> binaryTree<T> const* {
    return m_root->m_left;
}
template<typename T> [[nodiscard]] auto AVLTree<T>::getRightChild() const -> binaryTree<T> const* {
    return m_root->m_right;
}

template<typename T>
template<typename Functor> 
void AVLTree<T>::printRecursive(const binaryTree<T>* root, uint32_t space, Functor const& printTypename) const
{
    constexpr auto kCOUNT = 10;
    
    if (root == NULL) {
        return;
    }
    space += kCOUNT;
    
    
    printRecursive(root->m_right, space, printTypename);
    printf("\n%*s", space - kCOUNT, "");
    printTypename(root->m_data.get());
    printf(" (%u, %d)\n", root->m_height, root->m_bf);
    printRecursive(root->m_left, space, printTypename);
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
    uint32_t currentSpace = 0;
    while (curr != nullptr || !s.empty()) {
        while (curr != nullptr) {
            currentSpace += kCOUNT;
            s.push({curr, currentSpace});
            curr = curr->m_right;
        }


        TreeIdentPair top = s.top();
        s.pop();
        
        curr = top.first;
        int space = top.second;

        printf("\n%*s", space - kCOUNT, "");
        printTypename(root->m_data.get());
        printf(" (%u, %d)\n", root->m_height, root->m_bf);

        curr = curr->m_left;
        currentSpace = space;
    }
}


#endif /* __BINARY_TREE_GENERIC_IMPL_HEADER__ */