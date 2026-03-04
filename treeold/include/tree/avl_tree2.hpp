#pragma once
#include <util2/C/base_type.h>
#include <util2/type_trait.hpp>


template<typename T>
class AVLTree 
{
private:
    using valueType = typename util2::type_trait::value_ptr<T>::type;
    using classPtr  = AVLTree*;
    

    static constexpr auto reservedBytesSize() -> u64 {
        /* 
            value_ptr<t> folds any T above 8 bytes to a pointer. 
            so sizeof(m_balance, m_height) + sizeof(m_reserved) == multiple of 8 
        */
        constexpr auto maxSizeInBytesOfValueType = 8;
        return 6 - ( (sizeof(T) < maxSizeInBytesOfValueType) ? sizeof(T) : 0 );
    }

    
    [[nodiscard]] constexpr u64 nodeSize() const {
        return sizeof(AVLTree<T>);
    }


    [[nodiscard]] u8 computeHeight() const;
    [[nodiscard]] i8 computeBalanceFactor() const;

    bool     search(valueType searchValue, classPtr* toInit);
    classPtr insert(classPtr root, classPtr allocatedNode);
    classPtr balance(classPtr root);
    classPtr rotateLeft(classPtr root);
    classPtr rotateRight(classPtr root);

private:
    AVLTree*  m_left   = nullptr;
    AVLTree*  m_right  = nullptr;
    AVLTree*  m_parent = nullptr;
    valueType m_value;
    u8        m_height = 0;
    u8        m_reserved[reservedBytesSize()]{};

public:
    AVLTree()  = default;
    ~AVLTree() = default;
    AVLTree(const AVLTree&)            = delete;
    AVLTree& operator=(const AVLTree&) = delete;
    AVLTree(AVLTree&&)            = delete;
    AVLTree& operator=(AVLTree&&) = delete;


    void createValue(valueType val) noexcept;
    void create()  noexcept;
    void destroy() noexcept;
    bool insertNode(valueType toInsert) noexcept;
    bool deleteNode(valueType toDelete);
    bool search(valueType toSearch);
};