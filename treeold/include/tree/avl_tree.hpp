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

    constexpr u64 nodeSize() const {
        return sizeof(AVLTree<T>);
    }



    // i8 balance(classPtr root) const;
    bool     searchInternal(valueType searchValue, classPtr* toInit);
    classPtr insertInternal(classPtr root, classPtr allocatedNode);
    void rebalanceTree();

private:
    AVLTree*  m_left  = nullptr;
    AVLTree*  m_right = nullptr;
    valueType m_value;
    i8        m_balance = 0;
    u8        m_height  = 0;
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

    static bool isChild(classPtr root);
    static bool isParent(classPtr root);
    static bool isEmpty(classPtr root);


    u64  height()     const;
    u64  totalNodes() const;

    void  shallowCopy(AVLTree& dest) const; /* These functions allocate memory */
    void* deepCopy   (AVLTree& dest) const; /* These functions allocate memory */
};