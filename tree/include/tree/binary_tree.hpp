#pragma once
#include <util2/C/base_type.h>
#include <util2/type_trait.hpp>



template<typename T>
class BinaryTree {
private:
    using valueType = typename util2::type_trait::value_ptr<T>::type;


private:
    BinaryTree* m_left  = nullptr;
    BinaryTree* m_right = nullptr;
    valueType   m_value;

public:
    BinaryTree()  = default;
    ~BinaryTree() = default;
    BinaryTree(const BinaryTree&)            = delete;
    BinaryTree& operator=(const BinaryTree&) = delete;
    BinaryTree(BinaryTree&&)            = delete;
    BinaryTree& operator=(BinaryTree&&) = delete;
};