#include <tree/avl_tree2.hpp>
#include <util2/C/ifcrash2.h>
#include <cstring>
#include <queue>


template<typename T> void AVLTree<T>::createValue(valueType val) noexcept
{
    create();
    m_value = val;
    return;
}


template<typename T> void AVLTree<T>::create()  noexcept
{
    m_left = m_right = m_parent = nullptr;
    std::memset(&m_value, 0x00, sizeof(m_value));
    m_height = 0;
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


template<typename T> bool AVLTree<T>::insertNode(valueType toInsert) noexcept
{
    if(search(toInsert) == true) {
        return false;
    }


    auto* insertedNode = __rcast(AVLTree<T>*, malloc(nodeSize()) );
    if(m_value < toInsert) {
        m_left = insert(m_left, insertedNode);
    } else if(m_value > toInsert) {
        m_right = insert(m_right, insertedNode);
    }

    // m_balance = m_right->m_height - m_left->m_height;
    m_height  = 1 + std::max(m_right->m_height, m_left->m_height);

    return true;
}


template<typename T> bool AVLTree<T>::deleteNode(valueType toDelete)
{
    
}


template<typename T> bool AVLTree<T>::search(valueType toSearch)
{
    return search(toSearch, nullptr);
}



template<typename T> [[nodiscard]] u8 AVLTree<T>::computeHeight() const
{
    auto leftHeight  = m_left  ? m_left->m_height  : -1;
    auto rightHeight = m_right ? m_right->m_height : -1;
    return 1 + std::max(rightHeight, leftHeight);
}

template<typename T> [[nodiscard]] i8 AVLTree<T>::computeBalanceFactor() const
{
    auto leftHeight  = m_left  ? m_left->m_height  : -1;
    auto rightHeight = m_right ? m_right->m_height : -1;
    return rightHeight - leftHeight;
}


template<typename T> bool AVLTree<T>::search(valueType searchValue, classPtr* toInit)
{
    bool     found  = false;
    classPtr search = nullptr;

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


template<typename T> AVLTree<T>* AVLTree<T>::insert(
    classPtr root, 
    classPtr allocatedNode
) {
    if(root == nullptr) {
        return allocatedNode;
    }

    bool goLeft = allocatedNode->m_value < root->m_value;

    if(goLeft) {
        root->m_left = insert(root->m_left, allocatedNode);
    } else {
        root->m_right = insert(root->m_right, allocatedNode);
    }

    m_height = computeHeight();
    return balance(root);
}


template<typename T> AVLTree<T>* AVLTree<T>::balance(classPtr root)
{
    auto balance = computeBalanceFactor();
    bool doNothing = 0
        || (balance == 0)
        || (balance == -1)
        || (balance == 1);

    if(doNothing) {
        return root;
    }


    /* Right Heavy */
    if(balance > 1) {
        if(root->m_right >= 0) { /* Right-Right */
            /* left_rotation(root) */
            root = rotateLeft(root);
        }

        else { /* Right-Left */
            /* right_rotation(root->right) */
            /* left_rotation(root) */
            root = rotateRight(root->m_right);
            root = rotateLeft(root);
        }
    }

    /* Left Heavy */
    else {
        if(root->m_left >= 0) { /* Left-Left */
            /* right_rotation(root) */
            root = rotateRight(root);
        } 
        
        else { /* Left-Right */
            /* left_rotation(root->left) */
            /* right_rotation(root) */
            root = rotateLeft(root->m_left);
            root = rotateRight(root);
        }
    }

    return root;
}


template<typename T> AVLTree<T>* AVLTree<T>::rotateLeft(classPtr root)
{
    AVLTree<T> oldRootNode, oldRightNode;
    AVLTree<T>* newRoot = nullptr, *newLeft = nullptr;
    
    oldRootNode  = *root;
    oldRightNode = *root->m_right;    

    /* Swap the contents of the nodes as to not allocate memory */
    if(root->m_parent) {
        AVLTree<T>*& parentLeft  = root->m_parent->m_left;
        AVLTree<T>*& parentRight = root->m_parent->m_right;
        parentLeft  = (parentLeft  == root) ? root->m_left  : parentLeft;
        parentRight = (parentRight == root) ? root->m_right : parentRight;
    }
    newRoot  = root->m_right;
    newLeft  = root;
    *root->m_right = oldRootNode;
    *root          = oldRightNode;

    newRoot->m_left = newLeft;
    newRoot->m_left->m_right = oldRootNode.m_left;
    ++newRoot->m_height;
    --newLeft->m_height;

    return newRoot;
}


template<typename T> AVLTree<T>* AVLTree<T>::rotateRight(classPtr root)
{
    AVLTree<T> oldRootNode, oldLeftNode;
    AVLTree<T>* newRoot = nullptr, *newRight = nullptr;
    
    oldRootNode  = *root;
    oldLeftNode = *root->m_left;    

    /* Swap the contents of the nodes as to not allocate memory */
    if(root->m_parent) {
        AVLTree<T>*& parentLeft  = root->m_parent->m_left;
        AVLTree<T>*& parentRight = root->m_parent->m_right;
        parentLeft  = (parentLeft  == root) ? root->m_left  : parentLeft;
        parentRight = (parentRight == root) ? root->m_right : parentRight;
    }
    newRoot  = root->m_left;
    newRight  = root;
    *root->m_left = oldRootNode;
    *root         = oldLeftNode;

    newRoot->m_right = newRight;
    newRoot->m_right->m_left = oldRootNode.m_right;
    ++newRoot->m_height;
    --newRight->m_height;

    return newRoot;
}
