#include <tree/avl_tree.hpp>
#include <vector>
#include <stack>


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



template<typename T> void AVLTree<T>::create() noexcept
{
    m_left = m_right = nullptr;
    std::memset(&m_value, 0x00, sizeof(m_value));
    m_balance = 0;
    std::memset(m_reserved, 0x00, reservedBytesSize());
    return;
}

template<typename T> void AVLTree<T>::destroy() noexcept
{

}

template<typename T> void AVLTree<T>::insertNode(valueType toInsert) noexcept
{

}

template<typename T> bool AVLTree<T>::deleteNode(valueType toDelete)
{

}

template<typename T> bool AVLTree<T>::search(valueType toSearch)
{
    bool        found = false;
    AVLTree<T>* search = nullptr;

    for(search = this; (search != nullptr) && !found; ) {
        found = (search->m_value == toSearch);
        if(!found) {
            search = (toSearch < search->m_value) ? search->m_left : search->m_right;
        }
    }
    // return found ? search : nullptr;
    return found;
}


template<typename T> size_t AVLTree<T>::height() const
{
    size_t treeHeight = 0;
    auto getHeightState = [](AVLTree<T>* root, size_t* heightToInc) {
        
    };
    postOrderIterativeFunctional(this, getHeightState)
    // inOrderIterativeFunctional
}

template<typename T> size_t AVLTree<T>::totalNodes() const
{

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
