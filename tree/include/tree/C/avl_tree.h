#ifndef __TREELIB_AVL_TREE_C_VERSION_DEFINITION_HEADER__
#define __TREELIB_AVL_TREE_C_VERSION_DEFINITION_HEADER__
#include <tree/C/treelib_api.h>
#include <tree/C/treelib_extern.h>
#include <tree/C/compare_func.h>
#include <tree/C/construct_func.h>
#include <tree/C/op_result.h>
#include <tree/C/print_func.h>
#include <stddef.h>


TREELIB_EXTERNC_DECL_BEGIN


typedef struct binaryTreeNode binaryTreeNode;

typedef struct __avl_tree_definition {
	binaryTreeNode*          m_root;
	binaryTreeComparatorFunc m_cmp;
    binaryTreeValCopyFunc    m_ValueCopyConstructor;
    binaryTreeValDeleteFunc  m_ValueDestructor;
	uint32_t                 m_nodeCount;
	uint32_t                 m_dataSizeBytes;
} AVLTree;


void TREELIB_API AVLTreeCreate(
    AVLTree*                 root,
    binaryTreeComparatorFunc valueCompare,
    binaryTreeValCopyFunc    valueCopyConstructorFunctor_NullForPODTypes,
    binaryTreeValDeleteFunc  valueDestructorFunctor_NullForPODTypes,
    uint32_t                 valueSizeInBytes
);
binaryTreeResult_t TREELIB_API AVLTreeCreateCopy(
    AVLTree* toCopyFrom,
    AVLTree* toCopyTo
);
void               TREELIB_API AVLTreeDestroy(AVLTree* root);
binaryTreeResult_t TREELIB_API AVLTreeInsert(AVLTree* root, void* value);
binaryTreeResult_t TREELIB_API AVLTreeRemove(AVLTree* root, void* value);
binaryTreeBool_t   TREELIB_API AVLTreeSearch(AVLTree const* root, void* value);

binaryTreeStatusPair_t TREELIB_API AVLTreeCompare(
    AVLTree const*           rootA, 
    AVLTree const*           rootB,
    binaryTreeComparatorFunc cmpFunc
);
binaryTreeStatusPair_t TREELIB_API AVLTreeIsValidBST(AVLTree const* root);
binaryTreeStatusPair_t TREELIB_API AVLTreeIsBalanced(AVLTree const* root);

binaryTreeBool_t   TREELIB_API AVLTreeEmpty(AVLTree const* root);
uint32_t           TREELIB_API AVLTreeSize(AVLTree const* root);
int8_t             TREELIB_API AVLTreeHeight(AVLTree const* root);
binaryTreeResult_t TREELIB_API AVLTreePrint(
    AVLTree const* root,
    void*          outputTargetPtr,
    uint64_t       bufferSize,
    binaryTreeNodeDataPrinterFunc optionalNodePrinter
);


TREELIB_EXTERNC_DECL_END


#endif /* __AVL_TREE_C_VERSION_DEFINITION_HEADER__ */
