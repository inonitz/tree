#pragma once
#include <tree/treelib_api.h>
#include <cstdint>


class binaryTree;


enum class TREELIB_API AVLTreeRotationState : uint8_t {
    NONE       = 0,
    LEFTLEFT   = 1,
    RIGHTRIGHT = 2,
    LEFTRIGHT  = 3,
    RIGHTLEFT  = 4
};


class TREELIB_API AVLTree 
{
private:
    binaryTree* m_root      = nullptr;
    uint32_t    m_nodeCount = 0;
    uint8_t     m_reserved[4]{0};


public:
    AVLTree();
    ~AVLTree() = default;


    void clear() noexcept;
    bool insert(uint32_t val);
    bool remove(uint32_t val);
    bool search(uint32_t val);

    [[nodiscard]] bool isValidBST() const noexcept;
    [[nodiscard]] bool isBalanced() const noexcept;
    [[nodiscard]] bool     empty()  const;
    [[nodiscard]] uint64_t size()   const;
    [[nodiscard]] uint64_t height() const;
    [[nodiscard]] auto getRoot()       const -> binaryTree const*;
    [[nodiscard]] auto getLeftChild()  const -> binaryTree const*;
    [[nodiscard]] auto getRightChild() const -> binaryTree const*;
    void printRecursive(const binaryTree* root, int space) const;
    void printIterative(const binaryTree* root, int space) const;
};
