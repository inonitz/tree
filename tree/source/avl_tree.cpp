#include <tree/avl_tree.hpp>
#include <util2/C/ifcrash2.h>
#include <cstring>
#include <vector>
#include <stack>
#include <queue>


template<typename T>
void treeToBufferIterative(AVLTree<T>* root, std::vector<int>& toSet) {
    std::stack<AVLTree<T>*> st;

    AVLTree<T>* currNode = root;
    while(currNode != nullptr || !st.empty()) {
        while(currNode != nullptr) {
            st.push(currNode);
            currNode = currNode->left;
        }

        currNode = st.top();
        st.pop();
        toSet.push_back(currNode->val);
        currNode = currNode->right;
    }
    return;
}


template<typename T, typename Func, typename ... Args>
void inOrderIterativeFunctional(AVLTree<T>* root, Func&& toCall, Args ... args) {
    static_assert(std::is_invocable_v<Func, AVLTree<T>*, Args...>, 
        "inOrderIterativeFunctional(toCall, args...): toCall is not callable with the provided arguments"
    );
    if (!root) {
        return;
    }

    std::stack<AVLTree<T>*> st;
    AVLTree<T>*             currNode = root;
    while(currNode != nullptr || !st.empty()) 
    {
        while(currNode != nullptr) {
            st.push(currNode);
            currNode = currNode->left;
        }

        currNode = st.top();
        st.pop();
        toCall(currNode, args ...);
        currNode = currNode->right;
    }
    return;
}


template<typename T, typename Func, typename ... Args>
void postOrderIterativeFunctional(AVLTree<T>* root, Func&& toCall, Args ... args) {
    static_assert(std::is_invocable_v<Func, AVLTree<T>*, Args...>, 
        "inOrderIterativeFunctional(toCall, args...): toCall is not callable with the provided arguments"
    );
    if (!root) {
        return;
    }


    std::stack<AVLTree<T>*> s1, s2;
    AVLTree<T>* currNode = nullptr;
    
    /*
        Verify With Picture - don't have time rn
    */
    s1.push(root);
    while( !s1.empty() ) {
        currNode = s1.top();
        s1.pop();
        s2.push(currNode);

        if (currNode->left) s1.push(currNode->left);
        if (currNode->right) s1.push(currNode->right);
    }

    while (!s2.empty()) {
        toCall(currNode, args ...);
        s2.pop();
    }
}


template<typename T>
uint32_t treeHeightRecursive(AVLTree<T>* root) {
    if(root == nullptr) {
        return 0;
    }
    if(root->m_left == nullptr && root->m_right == nullptr) {
        return 1;
    }
    return 1 + std::max( treeHeightRecursive(root->m_left), treeHeightRecursive(root->m_right) );
}


template<typename T>
uint32_t treeHeightIterative(AVLTree<T>* root) {
    uint32_t height = 0;
    uint32_t currentLevelSize = 0;
    std::queue<AVLTree<T>*> currentHeightNodes;

    if(root == nullptr) {
        return 0;
    }


    currentHeightNodes.push(root);
    while(!currentHeightNodes.empty()) {
        ++height;

        currentLevelSize = currentHeightNodes.size();
        while(currentLevelSize) {
            auto& node = currentHeightNodes.front();
            if(node->left) {
                currentHeightNodes.push(node->left);
            }

            if(node->right) {
                currentHeightNodes.push(node->right);
            }

            currentHeightNodes.pop();
            --currentLevelSize;
        }
    }


    return height;
}


template<typename T>
uint32_t treeSizeRecursive(AVLTree<T>* root) {

    if(root == nullptr) {
        return 0;
    }
    if(root->m_left == nullptr && root->m_right == nullptr) {
        return 1;
    }
    return 1 + treeSizeRecursive(root->m_left) + treeSizeRecursive(root->m_right);
}


template<typename T>
uint32_t treeSizeIterative(AVLTree<T>* root) {
    uint32_t nodeCount        = 0;
    uint32_t currentLevelSize = 0;
    std::queue<AVLTree<T>*> currentHeightNodes;

    if(root == nullptr) {
        return 0;
    }


    currentHeightNodes.push(root);
    while(!currentHeightNodes.empty()) {
        currentLevelSize = currentHeightNodes.size();
        nodeCount += currentLevelSize;


        while(currentLevelSize) {
            auto& node = currentHeightNodes.front();
            if(node->left) {
                currentHeightNodes.push(node->left);
            }

            if(node->right) {
                currentHeightNodes.push(node->right);
            }

            currentHeightNodes.pop();
            --currentLevelSize;
        }
    }


    return nodeCount;
}



template<typename T> void AVLTree<T>::createValue(AVLTree<T>::valueType value) noexcept
{
    create();
    m_value = value;
    return;
}

template<typename T> void AVLTree<T>::create() noexcept
{
    m_left = m_right = nullptr;
    std::memset(&m_value, 0x00, sizeof(m_value));
    m_balance = 0;
    m_height  = 0;
    std::memset(m_reserved, 0x00, reservedBytesSize());
    return;
}

template<typename T> void AVLTree<T>::destroy() noexcept
{
    uint32_t currentLevelSize = 0;
    std::queue<AVLTree<T>*> currentHeightNodes;


    /* 
        Iterative Level Order Traversal 
        top node popped has already pushed its children to the queue
    */
    currentHeightNodes.push(this);
    while(!currentHeightNodes.empty()) {
        currentLevelSize = currentHeightNodes.size();

        while(currentLevelSize) {
            auto node = currentHeightNodes.front();
            if(node->left) {
                currentHeightNodes.push(node->left);
            }

            if(node->right) {
                currentHeightNodes.push(node->right);
            }

            currentHeightNodes.pop();
            --currentLevelSize;
            free(node);
        }
    }


    return;
}

// template<typename T> void AVLTree<T>::insertNode(valueType toInsert) noexcept
// {
//     auto* insertedNode = __rcast(AVLTree<T>*, malloc(nodeSize()) );
//     bool  goLeft       = false;
    
//     insertedNode->createValue(toInsert);
    
//     /* Search for the parent in the tree which should hold this value */
//     AVLTree<T>* search = this, *searchParent = nullptr;
//     for(; (search != nullptr); ) {
//         bool goLeft = (toInsert < search->m_value);


//         searchParent = search;
//         search->m_balance += goLeft ? -1 : 1;
//         search = goLeft ? search->m_left : search->m_right;
//     }


//     /* Conditionally set the parents' left/right */
//     goLeft = (toInsert < search->m_value);
//     searchParent->m_left  = goLeft ? 
//         insertedNode : searchParent->m_left;
//     searchParent->m_right = !goLeft ? 
//         insertedNode : searchParent->m_right;
    

//     rebalanceTree();
//     return;
// }


template<typename T> bool AVLTree<T>::insertNode(valueType toInsert) noexcept
{
    auto* insertedNode = __rcast(AVLTree<T>*, malloc(nodeSize()) );
    bool  tmp          = searchInternal(toInsert);


    if(tmp == true) {
        return false;
    }

    // insertNode()
    return true;
}


template<typename T> bool AVLTree<T>::deleteNode(valueType toDelete)
{
    classPtr foundNode = nullptr;
    if(searchInternal(toDelete, &foundNode) == false) {
        return false;
    }



    rebalanceTree();
    return true;
    /* Proceed with deleting the node in foundNode. rebalance the tree afterwards */
    /* we also need the parent of the node, so add that to search(val, ptr) */
}

template<typename T> bool AVLTree<T>::search(valueType toSearch)
{
    return searchInternal(toSearch, nullptr);
}


template<typename T> bool AVLTree<T>::isChild(classPtr root) {
    return root->m_left == nullptr && root->m_right == nullptr;
}

template<typename T> bool AVLTree<T>::isParent(classPtr root) {
    return root->m_left != nullptr || root->m_right != nullptr;
}

template<typename T> bool AVLTree<T>::isEmpty(classPtr root) {
    return root == nullptr;
}

template<typename T> size_t AVLTree<T>::height() const
{
    return treeHeightIterative(this);
}

template<typename T> size_t AVLTree<T>::totalNodes() const
{
    return treeSizeIterative(this);
}


template<typename T> void AVLTree<T>::shallowCopy(AVLTree<T>& dest) const {
    dest = {
        m_left,
        m_right,
        m_value,
        m_balance
    };
    return;
}

template<typename T> void* AVLTree<T>::deepCopy   (AVLTree<T>& dest) const {
    void* destNodes = malloc(nodeSize() * totalNodes());



    return destNodes;
}




// template<typename T> i8 AVLTree<T>::balance(classPtr root) const {
//     if(root == nullptr) {
//         return 0;
//     }
//     i64 tmpLeft  = treeHeightIterative(root->m_left);
//     i64 tmpRight = treeHeightIterative(root->m_right);
//     tmpLeft -= tmpRight;
//     ifcrash(tmpLeft < -128 || tmpLeft > 127);


//     return __scast(i8, tmpLeft);
// };


template<typename T> bool AVLTree<T>::searchInternal(valueType searchValue, classPtr* toInit)
{
    bool        found = false;
    AVLTree<T>* search = nullptr;

    for(search = this; (search != nullptr) && !found; ) {
        found = (search->m_value == searchValue);
        if(!found) {
            search = (searchValue < search->m_value) ? search->m_left : search->m_right;
        }
    }

    if(toInit != nullptr) {
        *toInit = search;
    }
    return found;
}

template<typename T> AVLTree<T>* AVLTree<T>::insertInternal(
    classPtr root, 
    classPtr allocatedNode
) {
    /* can't do anything at this stage */
    if(root == nullptr) {
        return allocatedNode;
    }

    bool goLeft = allocatedNode->m_value < root->m_value;
    
    if(goLeft) {
        
    }
}


template<typename T> void AVLTree<T>::rebalanceTree()
{
    return;
}

