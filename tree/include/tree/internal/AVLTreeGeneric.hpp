#ifndef __AVL_TREE_GENERIC_DEFINITION_HEADER__
#define __AVL_TREE_GENERIC_DEFINITION_HEADER__
#include <tree/C/treelib_api.h>
#include <cstdint>


template<typename T> class binaryTree;


template<typename T>
class TREELIB_API AVLTree 
{
private:

public:
    AVLTree()  = default;
    ~AVLTree() = default;


    void clear() noexcept;
    bool insert(T const& val);
    bool remove(T const& val);
    bool search(T const& val);

    bool insertRecursive(T const& val);
    bool removeRecursive(T const& val);
    bool searchRecursive(T const& val);


    [[nodiscard]] bool isValidBST() const noexcept;
    [[nodiscard]] bool isBalanced() const noexcept;
    [[nodiscard]] bool     empty()  const;
    [[nodiscard]] uint64_t size()   const;
    [[nodiscard]] int8_t   height() const;
    [[nodiscard]] auto getRoot()       const -> binaryTree<T> const*;
    [[nodiscard]] auto getLeftChild()  const -> binaryTree<T> const*;
    [[nodiscard]] auto getRightChild() const -> binaryTree<T> const*;
    template<typename Functor> void printRecursive(
        const binaryTree<T>* root, 
        uint32_t             space, 
        Functor const&       printTypename
    ) const;
    template<typename Functor> void printIterative(
        const binaryTree<T>* root, 
        uint32_t             space, 
        Functor const&       printTypename
    ) const;

private:
    binaryTree<T>* m_root      = nullptr;
    uint32_t       m_nodeCount = 0;
    uint8_t        m_reserved[4]{0};
};


#endif /* __AVL_TREE_GENERIC_DEFINITION_HEADER__ */
