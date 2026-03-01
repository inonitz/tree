#pragma once
#include <cstdint>


class binaryTree;


enum class AVLTreeRotationState : uint8_t {
    NONE       = 0,
    LEFTLEFT   = 1,
    RIGHTRIGHT = 2,
    LEFTRIGHT  = 3,
    RIGHTLEFT  = 4
};


class AVLTree {
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
    [[nodiscard]] bool   empty()  const;
    [[nodiscard]] size_t size()   const;
    [[nodiscard]] size_t height() const;
    [[nodiscard]] auto getRoot()       const -> binaryTree const*;
    [[nodiscard]] auto getLeftChild()  const -> binaryTree const*;
    [[nodiscard]] auto getRightChild() const -> binaryTree const*;
    void printRecursive(binaryTree* root, int space) const;
};