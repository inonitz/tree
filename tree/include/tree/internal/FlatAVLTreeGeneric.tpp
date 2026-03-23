#ifndef __FLAT_AVL_TREE_GENERIC_IMPLEMENTATION_HEADER__
#   define __FLAT_AVL_TREE_GENERIC_IMPLEMENTATION_HEADER__
#include "tree/C/dynamic_pool.h"
#   ifndef __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__
#	    include <tree/internal/FlatAVLTreeGeneric.hpp>
// #       include <tree/internal/FlatBinaryTree.hpp>
#   endif /* __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__ */
#   include <queue>
#   include <stack>
#   include <cassert>
#   include <cmath>


namespace ati = flat_avl_tree_internal;


template <typename T>
void FlatAVLTree<T>::clear() noexcept {
    m_activeNodes.clear();
    m_nodeVal.clear();
	m_nodeCount = 0;
    GenericDynamicPoolDestroy(&m_metadataPool);
    return;
}

template <typename T>
bool FlatAVLTree<T>::insertOld(T const& val) {
    if(empty()) {
        resize_on_demand();
        setNode(rootNodeIndex(), val);
        return true;
    }


    std::stack<uint32_t> nodeIdxStack;
    uint32_t topParentIdx = 0;
    uint32_t childIdx     = 0;
    if(searchAndPushParents(rootNodeIndex(), val, nodeIdxStack)) {
        return false; /* Node already exists */
    }


    /* Insert the new node to its correct placement, relative to its parent. */
    topParentIdx = nodeIdxStack.top();
    childIdx     = getNodeMetadata(topParentIdx).getConditionalChild( val < getNodeValue(topParentIdx) );

    resize_on_demand(); /* Make sure there is enough memory space to accomodate the new child */
    assert(childIdx < m_activeNodes.sizeInBits());
    setNode(childIdx, val); /* Initialize the new node */
    

    /* Check Balance across all parent nodes */
    uint32_t maybeNewRootIdx = 0;
    while(!nodeIdxStack.empty()) {
        topParentIdx = nodeIdxStack.top();
        rebalance(topParentIdx, &maybeNewRootIdx);
        nodeIdxStack.pop();
    }
    
    ++m_nodeCount;
    return true;
}


template <typename T>
bool FlatAVLTree<T>::insert(T const& val) {
    if(empty()) {

    }
}


template <typename T>
bool FlatAVLTree<T>::remove(T const& val) {
	// TODO: Implement iterative AVL deletion with rotations
	return false;
}

template <typename T>
bool FlatAVLTree<T>::search(T const& val) {
    bool found = false, cmp = false;
    
    auto& currNode = m_nodeVal[0];
    for(uint32_t searchIndex = rootNodeIndex(); !found && searchIndex < m_nodeVal.size(); ) {
        currNode = m_nodeVal[searchIndex];

        found = (currNode == val);
        searchIndex = pickChildIndex(searchIndex, val < currNode); 
    }
    return found;
}

// template <typename T>
// bool FlatAVLTree<T>::insertRecursive(T const& val) {
// 	// TODO: Implement recursive AVL insertion
// 	return false;
// }

// template <typename T>
// bool FlatAVLTree<T>::removeRecursive(T const& val) {
// 	// TODO: Implement recursive AVL deletion
// 	return false;
// }

// template <typename T>
// bool FlatAVLTree<T>::searchRecursive(T const& val) {
// 	// TODO: Implement recursive binary search
// 	return false;
// }

template <typename T>
bool FlatAVLTree<T>::isValidBST() const noexcept {
    if(empty()) {
        return false;
    }

    bool     satisfiesCondition = true;
    bool     tmpCond            = true;
    uint32_t currLevelSize      = 0;
    uint32_t currNodeIdx  = 0;
    uint32_t leftNodeIdx  = 1;
    uint32_t rightNodeIdx = 2;
    std::queue<uint32_t> currLevelNodes;
    

    /* 
        Extract the first check Outside the loop because 
        the tree might be very small / doesn't satisfy the condition already 
    */
    if(nodeExists(leftNodeIdx)) {
        tmpCond = tmpCond && ( getNodeValue(leftNodeIdx) < getNodeValue(currNodeIdx) );
        currLevelNodes.push(leftNodeIdx);
    }
    if(nodeExists(rightNodeIdx)) {
        tmpCond = tmpCond && ( getNodeValue(rightNodeIdx) > getNodeValue(currNodeIdx) );
        currLevelNodes.push(rightNodeIdx);
    }
    satisfiesCondition = satisfiesCondition && tmpCond;
    
    
    /* Iterative Level Order Traversal */
    while(satisfiesCondition && !currLevelNodes.empty() ) 
    {
        currLevelSize = currLevelNodes.size();
        while(currLevelSize) {
            currNodeIdx = currLevelNodes.front();
            leftNodeIdx  = leftNodeIndex(currNodeIdx);
            rightNodeIdx = rightNodeIndex(currNodeIdx);

            if(nodeExists(leftNodeIdx)) {
                tmpCond = tmpCond && ( getNodeValue(leftNodeIdx) < getNodeValue(currNodeIdx) );
                currLevelNodes.push(leftNodeIdx);
            }
            if(nodeExists(rightNodeIdx)) {
                tmpCond = tmpCond && ( getNodeValue(rightNodeIdx) > getNodeValue(currNodeIdx) );
                currLevelNodes.push(rightNodeIdx);
            }

            currLevelNodes.pop();
            --currLevelSize;
        }


        satisfiesCondition = satisfiesCondition && tmpCond;
    }


    return satisfiesCondition;
}

template <typename T>
bool FlatAVLTree<T>::isBalanced() const noexcept {
    if(!isValidBST()) {
        return false;
    }


    std::stack<uint32_t> nodeIdxStack;
    bool     satisfiesCondition = 1;
    int8_t   tmpVal = 0;
    uint32_t currNodeIdx = rootNodeIndex();


    /* Iterative Reverse-In-Order Tree Traversal */
    while (  satisfiesCondition && ( nodeExists(currNodeIdx) || !nodeIdxStack.empty() )  ) 
    {
        while (nodeExists(currNodeIdx)) {
            nodeIdxStack.push(currNodeIdx);
            currNodeIdx = rightNodeIndex(currNodeIdx);
        }

        currNodeIdx = nodeIdxStack.top();
        nodeIdxStack.pop();

        tmpVal = getNodeBalanceFactor(currNodeIdx);
        tmpVal =
            (tmpVal ==  0) || 
            (tmpVal == -1) || 
            (tmpVal == +1);
            
        satisfiesCondition = satisfiesCondition && static_cast<bool>(tmpVal);
        currNodeIdx = leftNodeIndex(currNodeIdx);
    }


    return satisfiesCondition;
}


template <typename T>
bool FlatAVLTree<T>::empty() const {
	return m_nodeCount == 0;
}

template <typename T>
uint32_t FlatAVLTree<T>::size() const {
	return static_cast<uint32_t>(m_nodeCount);
}

template <typename T>
int8_t FlatAVLTree<T>::height() const {
    int8_t result = -1;
    
    result = !empty() ? m_nodeMeta[rootNodeIndex()] : result;
    return result;
}

template <typename T>
template <typename Functor>
void FlatAVLTree<T>::print(uint32_t space, Functor const& printTypename) const {
	// Implementation for the nested template print function
	// Example: printTypename(m_underlyingData->at(i));
}






template<typename T>
T const& FlatAVLTree<T>::getNodeValue(uint32_t index) {
#ifdef NDEBUG
    return m_nodeVal[index];
#else
    return m_nodeVal.at(index); /* Useful for debugging */
#endif /* NDEBUG */
}

template<typename T>
void FlatAVLTree<T>::setNode(uint32_t index, T const& val)
{
#ifdef NDEBUG
    m_nodeVal[index] = val;
#else
    m_nodeVal.at(index) = val; /* Useful for debugging */
#endif /* NDEBUG */

    m_nodeMeta[index] = ati::Metadata{};
    m_activeNodes.toggle(index);
    return;
}

template<typename T>
bool FlatAVLTree<T>::nodeExists(uint32_t index) const
{
    return nodeState(index) == NodeState::SET;
}

template<typename T>
typename FlatAVLTree<T>::NodeState FlatAVLTree<T>::nodeState(uint32_t index) const
{
    if(index < m_activeNodes.sizeInBits()) {
        return static_cast<NodeState>(m_activeNodes[index]);
    }
    return NodeState::INVALID;
}

template<typename T>
typename ati::Metadata& FlatAVLTree<T>::getNodeMetadata(uint32_t index) {
#ifdef NDEBUG
    return m_nodeMeta[index];
#else
    return m_nodeMeta.at(index); /* Useful for debugging */
#endif /* NDEBUG */
}


template<typename T>
int8_t FlatAVLTree<T>::getNodeHeight(uint32_t nodeIndex) noexcept
{
    if(!nodeExists(nodeIndex)) {
        return -1;
    }

    auto rh = rightNodeIndex(nodeIndex);
    auto lh = leftNodeIndex(nodeIndex);
    rh = nodeExists(rh) ? m_nodeMeta[rh] : -1;
    lh = nodeExists(lh) ? m_nodeMeta[lh] : -1;
    return 1 + std::max(rh, lh);
}

template<typename T>
int8_t FlatAVLTree<T>::getNodeBalanceFactor(uint32_t nodeIndex) noexcept
{
    if(!nodeExists(nodeIndex)) {
        return 0;
    }

    auto rh = rightNodeIndex(nodeIndex);
    auto lh = leftNodeIndex(nodeIndex);
    rh = nodeExists(rh) ? m_nodeMeta[rh] : -1;
    lh = nodeExists(lh) ? m_nodeMeta[lh] : -1;
    return rh - lh;
}


template<typename T>
bool FlatAVLTree<T>::searchAndPushParents(
    uint32_t              nodeIndex, 
    T const&              value, 
    std::stack<uint32_t>& nodeIdxStack
) {
    bool found = false, cmp = false;
    
    auto& currNode = m_nodeVal[0];
    for(uint32_t searchIndex = rootNodeIndex(); !found && nodeExists(searchIndex); ) {
        currNode = m_nodeVal[searchIndex];

        found = (currNode == value);
        nodeIdxStack.push(searchIndex);
        searchIndex = pickChildIndex(searchIndex, value < currNode); 
    }
    return found;
}


template<typename T>
void FlatAVLTree<T>::rebalance(uint32_t nodeIndex, uint32_t* newRootIndex)
{

}

template<typename T>
void FlatAVLTree<T>::resize_on_demand()
{
    if(empty()) {  
        const uint32_t newInitialSize = 16;
        m_nodeVal.resize(newInitialSize);
        m_nodeMeta.resize(newInitialSize);
        m_activeNodes.resize(newInitialSize);
        return;
    }


    const double  currHeightFloat = std::log2(m_nodeCount);
    const uint8_t lastLevelHeight = static_cast<uint8_t>(currHeightFloat);
    const double  currHeightFractional = currHeightFloat - static_cast<double>(lastLevelHeight);
    /* 
        rebalancing allows a height diff of at-most 2 levels, until rebalancing is performed.
        Meaning, we should be reallocate at-most 2 additional levels on every resize
    */
    const uint32_t newSize = 0 
        + m_nodeVal.size()               /* Total Size upto now */
        + (1 << (lastLevelHeight + 1) )  /* Level N+1 Size */
        + (1 << (lastLevelHeight + 2) ); /* Level N+2 Size */


    /* 
        If indices that may be accessed currently (given by m_nodeCount) 
        are nearing memory access on the last tree Level,
        we need to re-allocate at-least another tree level, if not more.
    */
    if(m_nodeVal.size() - m_nodeCount <= (1 << lastLevelHeight)) {
        m_nodeVal.resize(newSize);
        m_nodeMeta.resize(newSize);
        m_activeNodes.resize(newSize);
    }
    return;
}



#endif /* __FLAT_BINARY_TREE_GENERIC_IMPLEMENTATION_HEADER__ */
