#ifndef __FLAT_AVL_TREE_GENERIC_IMPLEMENTATION_HEADER__
#   define __FLAT_AVL_TREE_GENERIC_IMPLEMENTATION_HEADER__
#   ifndef __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__
#	    include <tree/internal/FlatAVLTreeGeneric.hpp>
#   endif /* __FLAT_AVL_TREE_GENERIC_DEFINITION_HEADER__ */
#   include <tree/internal/AVLTreeRotateState.hpp>
#   include <tree/internal/FlatAVLTreeGenericIterators.hpp>
#   include <cmath>
#   include <string>


namespace ati = flat_avl_tree_internal;


namespace stream_type_trait
{
    template<typename S, typename T, typename = void>
    struct StreamOperatorExists : std::false_type {};

    template<typename S, typename T>
    struct StreamOperatorExists<S, T, std::void_t<decltype(std::declval<S&>() << std::declval<T>())>>
        : std::true_type {};
}


template <typename T>
void FlatAVLTree<T>::clear() noexcept {
    m_rootIdx     = ati::Metadata::k_nullIndex;
    m_freeNodeIdx = 0;
    while(!m_freedNodes.empty()) { m_freedNodes.pop(); }

    m_nodeMetadata.clear();
    m_nodeVal.clear();
    return;
}

template <typename T>
bool FlatAVLTree<T>::insert(T const& val) {
    if(empty()) {
        m_rootIdx = allocateNode();
        getNodeMetadata(m_rootIdx) = ati::Metadata{};
        getNodeValue(m_rootIdx)    = val;
        return true;
    }


    std::stack<uint32_t> nodeIdxStack;
    uint32_t maybeNewRootIdx = m_rootIdx;
    uint32_t parentIdx   = ati::Metadata::k_nullIndex;
    uint32_t currNodeIdx = 0;
    uint32_t childIdx    = 0;
    auto getParentOfTopNode = [](std::stack<uint32_t>& nodeStack) -> uint32_t {
        uint32_t top = nodeStack.top();
        uint32_t parent = ati::Metadata::k_nullIndex;

        nodeStack.pop();
        if(!nodeStack.empty()) {
            parent = nodeStack.top();
        }
        nodeStack.push(top);
        return parent;
    };



    /* if(true) -> Node already exists in the tree */
    if(searchAndPushParents(m_rootIdx, val, nodeIdxStack)) {
        return false;
    }

    /* Node doesn't exist, needs to be inserted relative to its parent. */
    /* Also update the parents' height */
    currNodeIdx = nodeIdxStack.top();
    childIdx    = allocateNode();
    auto& currNodeMeta = getNodeMetadata(currNodeIdx);

    /* First Initialize Child, s.t parent height is correct */
    getNodeMetadata(childIdx) = ati::Metadata{};
    getNodeValue(childIdx)    = val;

    currNodeMeta.setConditionalChild(
        val < getNodeValue(currNodeIdx),
        childIdx
    );
    currNodeMeta.setHeight(computeHeight(currNodeIdx));


    nodeIdxStack.pop(); /* currNode doesn't need rebalance, we can skip it */
    /* Check Balance across all parent nodes */
    while(!nodeIdxStack.empty()) {
        currNodeIdx = nodeIdxStack.top();
        parentIdx   = getParentOfTopNode(nodeIdxStack);

        /*
            Incase this is the last node (i.e the root), and a rotation was done,
            maybeNewRootIdx will be set to the new root, and won't be reset again
            (because we exit the loop).
            The new value in maybeNewRootIdx will be the one updated from the rebalance,
            i.e the updated root.
        */
        maybeNewRootIdx = currNodeIdx;
        rebalance(currNodeIdx, parentIdx, &maybeNewRootIdx);
        nodeIdxStack.pop();
    }
    m_rootIdx = maybeNewRootIdx; /* Incase the root did change */


    return true;
}


template <typename T>
bool FlatAVLTree<T>::remove(T const& val) {
    std::stack<uint32_t> nodeIdxStack;
    uint32_t maybeNewRootIdx = m_rootIdx;
    uint32_t parentIdx   = ati::Metadata::k_nullIndex;
    uint32_t currNodeIdx = 0;
    uint32_t toDeleteIdx = 0;
    bool     fullNode    = false;


    auto getParentOfTopNode = [](std::stack<uint32_t>& nodeStack) -> uint32_t {
        uint32_t top = nodeStack.top();
        uint32_t parent = ati::Metadata::k_nullIndex;

        nodeStack.pop();
        if(!nodeStack.empty()) {
            parent = nodeStack.top();
        }
        nodeStack.push(top);
        return parent;
    };


    /* if(true) -> Node doesn't exist. */
    if(!searchAndPushParents(m_rootIdx, val, nodeIdxStack)) {
        return false;
    }


    currNodeIdx = nodeIdxStack.top();
    parentIdx   = getParentOfTopNode(nodeIdxStack);
    toDeleteIdx = currNodeIdx;
    if(nodeIsFull(currNodeIdx)) {
        auto& currNodeMeta = getNodeMetadata(currNodeIdx);
        uint32_t successorIdx = ati::Metadata::k_nullIndex;

        auto leftNodeIdx  = currNodeMeta.getLeftChild();
        auto rightNodeIdx = currNodeMeta.getRightChild();
        auto leftNodeHeight  = getNodeMetadata(leftNodeIdx ).getHeight();
        auto rightNodeHeight = getNodeMetadata(rightNodeIdx).getHeight();
        if(leftNodeHeight < rightNodeHeight) {
            successorIdx = findMaxAndPushParents(leftNodeIdx, nodeIdxStack);
        } else {
            successorIdx = findMinAndPushParents(rightNodeIdx, nodeIdxStack);
        }
        /*
            A. currNode needs to be deleted, but deleting successorNode is easier.
                we'll swap values, and delete successorNode
            B. parentIdx might've changed due to findMin/MaxAndPushParents, so we update it.
        */
        std::swap(getNodeValue(currNodeIdx), getNodeValue(successorIdx));
        toDeleteIdx = successorIdx;
        parentIdx   = getParentOfTopNode(nodeIdxStack);
    }

    maybeNewRootIdx = removeNodeAndLinkParentWithChild(toDeleteIdx, parentIdx);
    nodeIdxStack.pop(); /* Pop the value we just deleted from the tree. */

    /*
        stack.top() is the parent of the node we just deleted.
        if stack.empty() is false ->
            stack.top() is the current parentIdx,
            we need to get the new parent
        else ->
            parentIdx = nullIdx (we deleted the root node)
    */
    while(!nodeIdxStack.empty())
    {
        currNodeIdx = nodeIdxStack.top();
        parentIdx   = getParentOfTopNode(nodeIdxStack);

        maybeNewRootIdx = currNodeIdx;
        rebalance(currNodeIdx, parentIdx, &maybeNewRootIdx);
        nodeIdxStack.pop();
    }
    m_rootIdx = maybeNewRootIdx; /* Set incase root changed on the last level. */


    return true;
}


template <typename T>
bool FlatAVLTree<T>::search(T const& val) {
    bool found = false, cmp = false;

    for(uint32_t searchIndex = m_rootIdx; !found && nodeExists(searchIndex); ) {
        auto& currNode = getNodeValue(searchIndex);

        found = (currNode == val);
        searchIndex = getNodeMetadata(searchIndex).getConditionalChild(val < currNode);
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

template <typename T> auto FlatAVLTree<T>::pre_order_range() noexcept
{
    return ati::PreOrderRange<T>{
        ati::PreOrderIteratorMutable<T>(*this, m_rootIdx),
        ati::PreOrderIteratorMutable<T>(*this, nodeMetadata::k_nullIndex)
    };
}
template <typename T> auto FlatAVLTree<T>::pre_order_range() const noexcept
{
    return ati::PreOrderRangeConst<T>{
        ati::PreOrderIteratorConst<T>(*this, m_rootIdx),
        ati::PreOrderIteratorConst<T>(*this, nodeMetadata::k_nullIndex)
    };
}
template <typename T> auto FlatAVLTree<T>::in_order_range() noexcept
{
    return ati::InOrderRange<T>{
        ati::InOrderIteratorMutable<T>(*this, m_rootIdx),
        ati::InOrderIteratorMutable<T>(*this, nodeMetadata::k_nullIndex)
    };
}
template <typename T> auto FlatAVLTree<T>::in_order_range() const noexcept
{
    return ati::InOrderRangeConst<T>{
        ati::InOrderIteratorConst<T>(*this, m_rootIdx),
        ati::InOrderIteratorConst<T>(*this, nodeMetadata::k_nullIndex)
    };
}
template <typename T> auto FlatAVLTree<T>::post_order_range() noexcept
{
    return ati::PostOrderRange<T>{
        ati::PostOrderIteratorMutable<T>(*this, m_rootIdx),
        ati::PostOrderIteratorMutable<T>(*this, nodeMetadata::k_nullIndex)
    };
}
template <typename T> auto FlatAVLTree<T>::post_order_range() const noexcept
{
    return ati::PostOrderRangeConst<T>{
        ati::PostOrderIteratorConst<T>(*this, m_rootIdx),
        ati::PostOrderIteratorConst<T>(*this, nodeMetadata::k_nullIndex)
    };
}
template <typename T> auto FlatAVLTree<T>::level_order_range() noexcept
{
    return ati::LevelOrderRange<T>{
        ati::LevelOrderIteratorMutable<T>(*this, m_rootIdx),
        ati::LevelOrderIteratorMutable<T>(*this, nodeMetadata::k_nullIndex)
    };
}
template <typename T> auto FlatAVLTree<T>::level_order_range() const noexcept
{
    return ati::LevelOrderRangeConst<T>{
        ati::LevelOrderIteratorConst<T>(*this, m_rootIdx),
        ati::LevelOrderIteratorConst<T>(*this, nodeMetadata::k_nullIndex)
    };
}




template <typename T>
bool FlatAVLTree<T>::isValidBST() const noexcept {
    if(empty()) {
        return true;
    }

    bool     satisfiesCondition = true;
    bool     tmpCond            = true;
    uint32_t currLevelSize      = 0;
    uint32_t currNodeIdx  = m_rootIdx;
    uint32_t leftNodeIdx  = readMetadata(currNodeIdx).getLeftChild();
    uint32_t rightNodeIdx = readMetadata(currNodeIdx).getRightChild();
    std::queue<uint32_t> currLevelNodes;


    /*
        Extract the first check Outside the loop because
        the tree might be very small / doesn't satisfy the condition already
    */
    if(nodeExists(leftNodeIdx)) {
        tmpCond = tmpCond && ( readValue(leftNodeIdx) < readValue(currNodeIdx) );
        currLevelNodes.push(leftNodeIdx);
    }
    if(nodeExists(rightNodeIdx)) {
        tmpCond = tmpCond && ( readValue(rightNodeIdx) > readValue(currNodeIdx) );
        currLevelNodes.push(rightNodeIdx);
    }
    satisfiesCondition = satisfiesCondition && tmpCond;


    /* Iterative Level Order Traversal */
    while(satisfiesCondition && !currLevelNodes.empty() )
    {
        currLevelSize = currLevelNodes.size();
        while(currLevelSize) {
            currNodeIdx = currLevelNodes.front();
            leftNodeIdx  = readMetadata(currNodeIdx).getLeftChild();
            rightNodeIdx = readMetadata(currNodeIdx).getRightChild();

            if(nodeExists(leftNodeIdx)) {
                tmpCond = tmpCond && ( readValue(leftNodeIdx) < readValue(currNodeIdx) );
                currLevelNodes.push(leftNodeIdx);
            }
            if(nodeExists(rightNodeIdx)) {
                tmpCond = tmpCond && ( readValue(rightNodeIdx) > readValue(currNodeIdx) );
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
    bool     satisfiesCondition = true;
    int8_t   tmpVal = 0;
    uint32_t currNodeIdx = m_rootIdx;


    /* Iterative Reverse-In-Order Tree Traversal */
    while (  satisfiesCondition && ( nodeExists(currNodeIdx) || !nodeIdxStack.empty() )  )
    {
        while (nodeExists(currNodeIdx)) {
            nodeIdxStack.push(currNodeIdx);
            currNodeIdx = readMetadata(currNodeIdx).getRightChild();
        }

        currNodeIdx = nodeIdxStack.top();
        nodeIdxStack.pop();

        tmpVal = computeBalanceFactor(currNodeIdx);
        tmpVal =
            (tmpVal ==  0) ||
            (tmpVal == -1) ||
            (tmpVal == +1);

        satisfiesCondition = satisfiesCondition && static_cast<bool>(tmpVal);
        currNodeIdx = readMetadata(currNodeIdx).getLeftChild();
    }


    return satisfiesCondition;
}


template <typename T>
bool FlatAVLTree<T>::empty() const noexcept {
	return size() == 0;
}

template <typename T>
uint32_t FlatAVLTree<T>::size() const noexcept {
	return m_freeNodeIdx - m_freedNodes.size();
}

template <typename T>
int8_t FlatAVLTree<T>::height() const {
    int8_t result = -1;

    result = !empty() ? readMetadata(m_rootIdx).getHeight() : result;
    return result;
}

template <typename T>
template <typename OutputStream>
OutputStream& FlatAVLTree<T>::print(
    uint32_t      space,
    OutputStream& stream
) const
{
    static_assert(stream_type_trait::StreamOperatorExists<OutputStream, T>::value,
        "Stream type must overload operator<< for printing"
    );

    typedef struct PointerSpacePair {
        uint32_t              m_nodeIdx;
        uint32_t              m_space;
    } NodeSpacing_t;


    static const uint32_t kSpaceCount  = 8;
    NodeSpacing_t         tmpNode      = {};
    uint32_t              currentSpace = 0;
    uint32_t              currNode     = m_rootIdx;
    std::stack<NodeSpacing_t> nodeStack;


    stream << "\n--- FlatAVLTreePrintBegin() ---\n";
    if(empty()) {
        stream << "NULL (0, 0)\n---  FlatAVLTreePrintEnd()  ---\n";
        return stream;
    }

    while (nodeExists(currNode) || !nodeStack.empty())
    {
        while ( nodeExists(currNode) ) {
            currentSpace += kSpaceCount;
            nodeStack.push({ currNode, currentSpace });
            currNode = readMetadata(currNode).getRightChild();
        }


        tmpNode = nodeStack.top();
        currentSpace = tmpNode.m_space;
        nodeStack.pop();


        /* Actual Printing Begin */
        std::string emptySpaceStr(tmpNode.m_space - kSpaceCount, ' ');


        stream << emptySpaceStr << readValue(tmpNode.m_nodeIdx)
            << " " << static_cast<int32_t>( readMetadata(tmpNode.m_nodeIdx).getHeight() )
            << " " << static_cast<int32_t>( computeBalanceFactor(tmpNode.m_nodeIdx)     )
            << "\n";
        /* Actual Printing End */

        currNode = readMetadata(tmpNode.m_nodeIdx).getLeftChild();
    }


    stream << "\n---  FlatAVLTreePrintEnd()  ---\n";
    return stream;
}




template<typename T>
T& FlatAVLTree<T>::getNodeValue(uint32_t index) {
#ifdef NDEBUG
    return m_nodeVal[index];
#else
    return m_nodeVal.at(index); /* Useful for debugging */
#endif /* NDEBUG */
}

template<typename T>
typename ati::Metadata& FlatAVLTree<T>::getNodeMetadata(uint32_t index) {
#ifdef NDEBUG
    return m_nodeMetadata[index];
#else
    return m_nodeMetadata.at(index); /* Useful for debugging */
#endif /* NDEBUG */
}

template<typename T>
T const& FlatAVLTree<T>::readValue(uint32_t index) const {
#ifdef NDEBUG
    return m_nodeVal[index];
#else
    return m_nodeVal.at(index); /* Useful for debugging */
#endif /* NDEBUG */
}

template<typename T>
typename ati::Metadata const& FlatAVLTree<T>::readMetadata(uint32_t index) const {
#ifdef NDEBUG
    return m_nodeMetadata[index];
#else
    return m_nodeMetadata.at(index); /* Useful for debugging */
#endif /* NDEBUG */
}

template<typename T>
bool FlatAVLTree<T>::nodeExists(uint32_t index) const noexcept
{
    return index != nodeMetadata::k_nullIndex;
}

template<typename T>
bool FlatAVLTree<T>::nodeIsFull(uint32_t index) const noexcept
{
    if(!nodeExists(index)) {
        return false;
    }
    auto& meta = readMetadata(index);
    return
        (meta.getLeftChild() != ati::Metadata::k_nullIndex)
        &&
        (meta.getRightChild() != ati::Metadata::k_nullIndex);
}

template<typename T>
int8_t FlatAVLTree<T>::computeHeight(uint32_t nodeIndex) const noexcept
{
    int8_t result = -1;
    if(!nodeExists(nodeIndex)) {
        return result;
    }

    uint32_t rhi = readMetadata(nodeIndex).getRightChild();
    uint32_t lhi = readMetadata(nodeIndex).getLeftChild();
    int8_t   rh = nodeExists(rhi) ? readMetadata(rhi).getHeight() : -1;
    int8_t   lh = nodeExists(lhi) ? readMetadata(lhi).getHeight() : -1;
    result = 1 + std::max(rh, lh);
    return result;
}

template<typename T>
int8_t FlatAVLTree<T>::computeBalanceFactor(uint32_t nodeIndex) const noexcept
{
    int8_t result = 0;
    if(!nodeExists(nodeIndex)) {
        return result;
    }

    uint32_t rhi = readMetadata(nodeIndex).getRightChild();
    uint32_t lhi = readMetadata(nodeIndex).getLeftChild();
    int8_t   rh = nodeExists(rhi) ? readMetadata(rhi).getHeight() : -1;
    int8_t   lh = nodeExists(lhi) ? readMetadata(lhi).getHeight() : -1;
    result = rh - lh;
    return result;
}


template<typename T>
bool FlatAVLTree<T>::searchAndPushParents(
    uint32_t              nodeIndex,
    T const&              value,
    std::stack<uint32_t>& nodeIdxStack
) {
    bool found = false, cmp = false;

    for(uint32_t searchIndex = m_rootIdx; !found && nodeExists(searchIndex); ) {
        auto& currNode = readValue(searchIndex);

        found = (currNode == value);
        nodeIdxStack.push(searchIndex);
        searchIndex = readMetadata(searchIndex).getConditionalChild(value < currNode);
    }
    return found;
}

template<typename T>
uint32_t FlatAVLTree<T>::findMaxAndPushParents(uint32_t nodeIdx, std::stack<uint32_t>& nodeIdxStack)
{
    uint32_t searchIdx = nodeIdx;
    while(nodeExists(searchIdx)) {
        nodeIdx = searchIdx;
        nodeIdxStack.push(nodeIdx);
        searchIdx = readMetadata(searchIdx).getRightChild();
    }

    return nodeIdx;
}

template<typename T>
uint32_t FlatAVLTree<T>::findMinAndPushParents(uint32_t nodeIdx, std::stack<uint32_t>& nodeIdxStack)
{
    uint32_t searchIdx = nodeIdx;
    while(nodeExists(searchIdx)) {
        nodeIdx = searchIdx;
        nodeIdxStack.push(nodeIdx);
        searchIdx = readMetadata(searchIdx).getLeftChild();
    }

    return nodeIdx;
}

template<typename T>
uint32_t FlatAVLTree<T>::removeNodeAndLinkParentWithChild(uint32_t nodeIdx, uint32_t nodeParentIdx)
{
    auto& nodeMeta = readMetadata(nodeIdx);
    uint32_t childNodeIdx = nodeExists(nodeMeta.getLeftChild()) ?
        nodeMeta.getLeftChild()
        :
        nodeExists(nodeMeta.getRightChild()) ? nodeMeta.getRightChild() : ati::Metadata::k_nullIndex;


    if(nodeExists(nodeParentIdx)) {
        auto& parentMeta = getNodeMetadata(nodeParentIdx);
        parentMeta.setConditionalChild(
            parentMeta.getLeftChild() == nodeIdx,
            childNodeIdx
        );
    }
    freeNode(nodeIdx);


    return childNodeIdx;
}


/*
    Before the Rotation:
        X (Root)
       / \
      Z   Y
         / \
        B   C
    After The Rotation:
        Y (New Root)
       / \
      X   C
     / \
    Z   B
*/
template<typename T>
uint32_t FlatAVLTree<T>::rotateLeft(uint32_t nodeIdx, uint32_t parentIdx)
{
    auto root_parent = parentIdx;
    auto x = nodeIdx;
    auto y = readMetadata(x).getRightChild();
    auto b = readMetadata(y).getLeftChild();


    getNodeMetadata(x).setRightChild(b);
    getNodeMetadata(y).setLeftChild(x);

    if(nodeExists(root_parent)) {
        /* swap X for Y on the upper-parent level */
        auto& parentMeta = getNodeMetadata(root_parent);
        bool  nodeIsLeftChild = (parentMeta.getLeftChild() == x);
        parentMeta.setConditionalChild(nodeIsLeftChild, y);
    }

    getNodeMetadata(x).setHeight(computeHeight(x));
    getNodeMetadata(y).setHeight(computeHeight(y));
    return y;
}

/*
    Before The Rotation:
        Y (Old Root)
       / \
      X   C
     / \
    Z   B

    After the Rotation:
        X (Root)
       / \
      Z   Y
     / \
    B   C
*/
template<typename T>
uint32_t FlatAVLTree<T>::rotateRight(uint32_t nodeIdx, uint32_t parentIdx)
{
    auto root_parent = parentIdx;
    auto y = nodeIdx;
    auto x = readMetadata(y).getLeftChild();
    auto b = readMetadata(x).getRightChild();


    getNodeMetadata(y).setLeftChild(b);
    getNodeMetadata(x).setRightChild(y);

    if(nodeExists(root_parent)) {
        /* swap Y for X on the upper-parent level */
        auto& parentMeta = getNodeMetadata(root_parent);
        bool  nodeIsLeftChild = (parentMeta.getLeftChild() == y);
        parentMeta.setConditionalChild(nodeIsLeftChild, x);
    }

    getNodeMetadata(y).setHeight(computeHeight(y));
    getNodeMetadata(x).setHeight(computeHeight(x));
    return x;
}

template<typename T>
void FlatAVLTree<T>::rebalance(
    uint32_t  nodeIndex,
    uint32_t  nodeParentIdx,
    uint32_t* newRootIndex
) {
    auto& nodeMeta = getNodeMetadata(nodeIndex);

    /* Update Height & compute balance factors */
    nodeMeta.setHeight(computeHeight(nodeIndex));
    auto bf      = computeBalanceFactor(nodeIndex);
    auto bfright = computeBalanceFactor(nodeMeta.getRightChild());
    auto bfleft  = computeBalanceFactor(nodeMeta.getLeftChild());

    AVLTreeRotationState state = AVLTreeRotationState::NONE;
    state = (bf == -2 && bfleft  <= 0) ? AVLTreeRotationState::LEFTLEFT   : state;
    state = (bf == -2 && bfleft  >  0) ? AVLTreeRotationState::LEFTRIGHT  : state;
    state = (bf == +2 && bfright >= 0) ? AVLTreeRotationState::RIGHTRIGHT : state;
    state = (bf == +2 && bfright <  0) ? AVLTreeRotationState::RIGHTLEFT  : state;


    switch(state) {
        case AVLTreeRotationState::LEFTLEFT:
        *newRootIndex = rotateRight(nodeIndex, nodeParentIdx);
        break;

        case AVLTreeRotationState::LEFTRIGHT:
        nodeMeta.setLeftChild( rotateLeft(nodeMeta.getLeftChild(), nodeIndex) );
        *newRootIndex = rotateRight(nodeIndex, nodeParentIdx);
        break;

        case AVLTreeRotationState::RIGHTRIGHT:
        *newRootIndex = rotateLeft(nodeIndex, nodeParentIdx);
        break;

        case AVLTreeRotationState::RIGHTLEFT:
        nodeMeta.setRightChild( rotateRight(nodeMeta.getRightChild(), nodeIndex) );
        *newRootIndex = rotateLeft(nodeIndex, nodeParentIdx);
        break;

        case AVLTreeRotationState::NONE:
        default:
        break;
    }


    return;
}


template<typename T>
void FlatAVLTree<T>::resize()
{
    if(empty()) {
        const uint32_t newInitialSize = 15;
        m_nodeVal.resize(newInitialSize);
        m_nodeMetadata.resize(newInitialSize);
        return;
    }


    const double  currHeightFloat = std::log2(size());
    const uint8_t lastLevelHeight = static_cast<uint8_t>(currHeightFloat);
    const double  currHeightFractional = currHeightFloat - static_cast<double>(lastLevelHeight);
    /*
        rebalancing allows a height diff of at-most 2 levels, until rebalancing is performed.
        Meaning, we should be reallocate at-most 2 additional levels on every resize
    */
    const uint32_t newSize = 0
        + m_nodeVal.size()               /* Total Size upto now */
        + (1 << (lastLevelHeight + 1) );  /* Level N+1 Size */

    m_nodeVal.resize(newSize);
    m_nodeMetadata.resize(newSize);
    return;
}


template<typename T>
uint32_t FlatAVLTree<T>::allocateNode()
{
    uint32_t out = 0;
    if(!m_freedNodes.empty()) {
        out = m_freedNodes.top();
        m_freedNodes.pop();
        return out;
    }


    /* Simple bump allocation */
    if(m_freeNodeIdx == m_nodeMetadata.size()) {
        resize();
    }
    out = m_freeNodeIdx;
    ++m_freeNodeIdx;
    return out;
}

template<typename T>
void FlatAVLTree<T>::freeNode(uint32_t nodeIdx)
{
    m_freedNodes.push(nodeIdx);
    return;
}




#endif /* __FLAT_BINARY_TREE_GENERIC_IMPLEMENTATION_HEADER__ */
