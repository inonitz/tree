#ifndef __FLAT_AVL_TREE_GENERIC_ITERATORS_DEFINITION_HEADER__
#define __FLAT_AVL_TREE_GENERIC_ITERATORS_DEFINITION_HEADER__
#include <tree/FlatAVLTree.hpp>
#include <tree/internal/nodeMetadata.hpp>
#include <type_traits>
#include <queue>


namespace flat_avl_tree_internal {


template<typename Iterator>
struct TraversalRange {
    Iterator m_begin;
    Iterator m_end;

    Iterator begin() const { return m_begin; }
    Iterator end()   const { return m_end; }
};


template<typename T, bool IsConst>
class PreOrderIterator 
{
private:
    friend class FlatAVLTree<T>;
    using iterator_category = std::forward_iterator_tag; 
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = std::conditional_t<IsConst, const T*, T*>;
    using reference         = std::conditional_t<IsConst, const T&, T&>;
    
    using nodeMetadata = flat_avl_tree_internal::Metadata;
    using TreeRef      = std::conditional_t<IsConst, const FlatAVLTree<T>&, FlatAVLTree<T>&>;
    using IndexStack   = std::stack<uint32_t>;
    using ValRef       = std::conditional_t<IsConst, const T&, T&>;
    using ValPtr       = std::conditional_t<IsConst, const T*, T*>;

public:
    PreOrderIterator(TreeRef tree, uint32_t startIdx) : m_tree(tree) {
        if (startIdx != nodeMetadata::k_nullIndex) {
            m_stack.push(startIdx);
            advance();
        }
    }

    auto operator*()  const { return m_tree.readValue(m_currIdx); }
    auto operator->() const { return &m_tree.readValue(m_currIdx); }

    PreOrderIterator& operator++() {
        advance();
        return *this;
    }

    bool operator==(const PreOrderIterator& other) const { 
        return m_currIdx == other.m_currIdx && &m_tree == &other.m_tree; 
    }
    bool operator!=(const PreOrderIterator& other) const { return !(*this == other); }

private:
    void advance() {
        if (m_stack.empty()) {
            m_currIdx = nodeMetadata::k_nullIndex;
            return;
        }
        m_currIdx = m_stack.top();
        m_stack.pop();

        auto const& meta = m_tree.readMetadata(m_currIdx);
        uint32_t right = meta.getRightChild();
        uint32_t left  = meta.getLeftChild();

        if (right != nodeMetadata::k_nullIndex) m_stack.push(right);
        if (left  != nodeMetadata::k_nullIndex) m_stack.push(left);
    }

    TreeRef    m_tree;
    uint32_t   m_currIdx = nodeMetadata::k_nullIndex;
    IndexStack m_stack;
};


template<typename T, bool IsConst>
class InOrderIterator 
{
private:
    friend class FlatAVLTree<T>;
    using iterator_category = std::forward_iterator_tag; 
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = std::conditional_t<IsConst, const T*, T*>;
    using reference         = std::conditional_t<IsConst, const T&, T&>;
    
    using nodeMetadata = flat_avl_tree_internal::Metadata;
    using TreeRef      = std::conditional_t<IsConst, const FlatAVLTree<T>&, FlatAVLTree<T>&>;
    using IndexStack   = std::stack<uint32_t>;
    using ValRef       = std::conditional_t<IsConst, const T&, T&>;
    using ValPtr       = std::conditional_t<IsConst, const T*, T*>;

public:
    InOrderIterator(TreeRef tree, uint32_t startIdx) : m_tree(tree) {
        pushLeft(startIdx);
        advance();
    }

    auto operator*()  const { return m_tree.readValue(m_currIdx); }
    auto operator->() const { return &m_tree.readValue(m_currIdx); }

    InOrderIterator& operator++() {
        advance();
        return *this;
    }

    bool operator==(const InOrderIterator& other) const { 
        return m_currIdx == other.m_currIdx && &m_tree == &other.m_tree; 
    }
    bool operator!=(const InOrderIterator& other) const { return !(*this == other); }

private:
    void pushLeft(uint32_t idx) {
        while (idx != nodeMetadata::k_nullIndex) {
            m_stack.push(idx);
            idx = m_tree.readMetadata(idx).getLeftChild();
        }
    }

    void advance() {
        if (m_stack.empty()) {
            m_currIdx = nodeMetadata::k_nullIndex;
            return;
        }
        m_currIdx = m_stack.top();
        m_stack.pop();
        pushLeft(m_tree.readMetadata(m_currIdx).getRightChild());
    }

    TreeRef    m_tree;
    uint32_t   m_currIdx = nodeMetadata::k_nullIndex;
    IndexStack m_stack;
};


template<typename T, bool IsConst>
class PostOrderIterator 
{
private:
    friend class FlatAVLTree<T>;
    using iterator_category = std::forward_iterator_tag; 
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = std::conditional_t<IsConst, const T*, T*>;
    using reference         = std::conditional_t<IsConst, const T&, T&>;
    
    using nodeMetadata = flat_avl_tree_internal::Metadata;
    using TreeRef      = std::conditional_t<IsConst, const FlatAVLTree<T>&, FlatAVLTree<T>&>;
    using IndexStack   = std::stack<uint32_t>;
    using ValRef       = std::conditional_t<IsConst, const T&, T&>;
    using ValPtr       = std::conditional_t<IsConst, const T*, T*>;

public:
    PostOrderIterator(TreeRef tree, uint32_t startIdx) : m_tree(tree) {
        pushLeft(startIdx);
        advance();
    }

    auto operator*()  const { return m_tree.readValue(m_currIdx); }
    auto operator->() const { return &m_tree.readValue(m_currIdx); }

    PostOrderIterator& operator++() {
        advance();
        return *this;
    }

    bool operator==(const PostOrderIterator& other) const { 
        return m_currIdx == other.m_currIdx && &m_tree == &other.m_tree; 
    }
    bool operator!=(const PostOrderIterator& other) const { return !(*this == other); }

private:
    void pushLeft(uint32_t idx) {
        while (idx != nodeMetadata::k_nullIndex) {
            m_stack.push(idx);
            idx = m_tree.readMetadata(idx).getLeftChild();
        }
    }

    void advance() {
        while (!m_stack.empty()) {
            uint32_t peekIdx = m_stack.top();
            uint32_t rightIdx = m_tree.readMetadata(peekIdx).getRightChild();

            // Only move right if a right child exists AND we aren't returning from it
            if (rightIdx != nodeMetadata::k_nullIndex && m_lastVisitedIdx != rightIdx) {
                pushLeft(rightIdx);
            } else {
                m_currIdx = peekIdx;
                m_lastVisitedIdx = m_currIdx;
                m_stack.pop();
                return;
            }
        }
        m_currIdx = nodeMetadata::k_nullIndex;
    }

    TreeRef    m_tree;
    uint32_t   m_currIdx        = nodeMetadata::k_nullIndex;
    uint32_t   m_lastVisitedIdx = nodeMetadata::k_nullIndex;
    IndexStack m_stack;
};


template<typename T, bool IsConst>

class LevelOrderIterator 
{
private:
    friend class FlatAVLTree<T>;
    using iterator_category = std::forward_iterator_tag; 
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = std::conditional_t<IsConst, const T*, T*>;
    using reference         = std::conditional_t<IsConst, const T&, T&>;
    
    using nodeMetadata = flat_avl_tree_internal::Metadata;
    using TreeRef      = std::conditional_t<IsConst, const FlatAVLTree<T>&, FlatAVLTree<T>&>;
    using IndexQueue   = std::queue<uint32_t>;
    using ValRef       = std::conditional_t<IsConst, const T&, T&>;
    using ValPtr       = std::conditional_t<IsConst, const T*, T*>;

public:
    LevelOrderIterator(TreeRef tree, uint32_t startIdx) : m_tree(tree) {
        if (startIdx != nodeMetadata::k_nullIndex) {
            m_queue.push(startIdx);
            advance();
        }
    }

    auto operator*()  const { return m_tree.readValue(m_currIdx); }
    auto operator->() const { return &m_tree.readValue(m_currIdx); }

    LevelOrderIterator& operator++() {
        advance();
        return *this;
    }

    bool operator==(const LevelOrderIterator& other) const { 
        return m_currIdx == other.m_currIdx && &m_tree == &other.m_tree; 
    }
    bool operator!=(const LevelOrderIterator& other) const { return !(*this == other); }

private:
    void advance() {
        if (m_queue.empty()) {
            m_currIdx = nodeMetadata::k_nullIndex;
            return;
        }
        m_currIdx = m_queue.front();
        m_queue.pop();

        auto const& meta = m_tree.readMetadata(m_currIdx);
        if (meta.getLeftChild()  != nodeMetadata::k_nullIndex) m_queue.push(meta.getLeftChild());
        if (meta.getRightChild() != nodeMetadata::k_nullIndex) m_queue.push(meta.getRightChild());
    }

    TreeRef    m_tree;
    uint32_t   m_currIdx = nodeMetadata::k_nullIndex;
    IndexQueue m_queue;
};


template<typename T> using PreOrderIteratorMutable   = PreOrderIterator<T, false>;
template<typename T> using PreOrderIteratorConst     = PreOrderIterator<T, true>;
template<typename T> using InOrderIteratorMutable    = InOrderIterator<T, false>;
template<typename T> using InOrderIteratorConst      = InOrderIterator<T, true>;
template<typename T> using PostOrderIteratorMutable  = PostOrderIterator<T, false>;
template<typename T> using PostOrderIteratorConst    = PostOrderIterator<T, true>;
template<typename T> using LevelOrderIteratorMutable = LevelOrderIterator<T, false>;
template<typename T> using LevelOrderIteratorConst   = LevelOrderIterator<T, true>;

template<typename T> using PreOrderRange        = TraversalRange< PreOrderIteratorMutable<T>   >;
template<typename T> using PreOrderRangeConst   = TraversalRange< PreOrderIteratorConst<T>     >;
template<typename T> using InOrderRange         = TraversalRange< InOrderIteratorMutable<T>    >;
template<typename T> using InOrderRangeConst    = TraversalRange< InOrderIteratorConst<T>      >;
template<typename T> using PostOrderRange       = TraversalRange< PostOrderIteratorMutable<T>  >;
template<typename T> using PostOrderRangeConst  = TraversalRange< PostOrderIteratorConst<T>    >;
template<typename T> using LevelOrderRange      = TraversalRange< LevelOrderIteratorMutable<T> >;
template<typename T> using LevelOrderRangeConst = TraversalRange< LevelOrderIteratorConst<T>   >;


} /* namespace flat_avl_tree_internal */


#endif /* __FLAT_AVL_TREE_GENERIC_ITERATORS_DEFINITION_HEADER__ */
