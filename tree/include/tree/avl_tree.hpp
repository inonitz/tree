#pragma once
#include <util2/C/base_type.h>
#include <util2/type_trait.hpp>


template<typename T>
class AVLTree 
{
private:
    using valueType = typename util2::type_trait::value_ptr<T>::type;
    
    
    static constexpr auto reservedBytesSize() -> size_t {
        /* value_ptr<t> folds any T above 8 bytes to a pointer. so sizeof(m_balance) + sizeof(m_reserved) == multiple of 8 */
        if constexpr (sizeof(T) >= 8) {
            return 7;
        } else {
            return 7 - sizeof(T);
        }
    }

    constexpr size_t nodeSize() const {
        return sizeof(AVLTree<T>);
    }


private:
    AVLTree*  m_left;
    AVLTree*  m_right;
    valueType m_value;
    u8        m_balance;
    u8        m_reserved[reservedBytesSize()];

public:
    AVLTree()  = default;
    ~AVLTree() = default;
    AVLTree(const AVLTree&)            = delete;
    AVLTree& operator=(const AVLTree&) = delete;
    AVLTree(AVLTree&&)            = delete;
    AVLTree& operator=(AVLTree&&) = delete;


    void create()  noexcept;
    void destroy() noexcept;
    void insertNode(valueType toInsert) noexcept;
    bool deleteNode(valueType toDelete);
    bool search(valueType toSearch);

    size_t height() const;
    size_t totalNodes() const;

    void  shallowCopy(AVLTree& dest) const; /* These functions allocate memory */
    void* deepCopy   (AVLTree& dest) const; /* These functions allocate memory */
};