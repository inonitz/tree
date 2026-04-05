#ifndef __TREELIB_BINARY_TREE_CONSTRUCT_AND_DESTRUCT_GENERIC_VALUE_DEFINITION_HEADER__
#define __TREELIB_BINARY_TREE_CONSTRUCT_AND_DESTRUCT_GENERIC_VALUE_DEFINITION_HEADER__
#include <stdint.h>


/*
    Complex Structs that require a custom initialize/copy require a
    constructor/destructor pair
    the function returns 0 on success, anything else on failure
*/
typedef int8_t (*binaryTreeGenericValueCopyConstructorFunc)(void* dst, const void* src);
typedef void   (*binaryTreeGenericValueDestructorFunc)(void* toDestroy);

typedef binaryTreeGenericValueCopyConstructorFunc binaryTreeValCopyFunc; 
typedef binaryTreeGenericValueDestructorFunc      binaryTreeValDeleteFunc; 


#endif /* __TREELIB_BINARY_TREE_CONSTRUCT_AND_DESTRUCT_GENERIC_VALUE_DEFINITION_HEADER__ */
