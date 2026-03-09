#ifndef __BINARY_TREE_OPERATION_RETURN_STATUS_DEFINITION_HEADER__
#define __BINARY_TREE_OPERATION_RETURN_STATUS_DEFINITION_HEADER__
#include <stdint.h>

#ifdef BINARY_TREE_OP_RESULT_TYPE
#   error "Someone has already defined the macro 'BINARY_TREE_OP_RESULT_TYPE' somewhere else"
#endif /* BINARY_TREE_OP_RESULT_TYPE */

#ifdef BINARY_TREE_OP_SUCCESS
#   error "Someone has already defined the macro 'BINARY_TREE_OP_SUCCESS' somewhere else"
#endif /* BINARY_TREE_OP_SUCCESS */

#ifdef BINARY_TREE_OP_FAILURE
#   error "Someone has already defined the macro 'BINARY_TREE_OP_FAILURE' somewhere else"
#endif /* BINARY_TREE_OP_FAILURE */

#ifdef BINARY_TREE_BOOL_TRUE
#   error "Someone has already defined the macro 'BINARY_TREE_BOOL_TRUE' somewhere else"
#endif /* BINARY_TREE_BOOL_TRUE */

#ifdef BINARY_TREE_BOOL_FALSE
#   error "Someone has already defined the macro 'BINARY_TREE_BOOL_FALSE' somewhere else"
#endif /* BINARY_TREE_BOOL_FALSE */


#ifndef BINARY_TREE_OP_RESULT_TYPE
#   define BINARY_TREE_OP_RESULT_TYPE
    typedef uint8_t binaryTreeResult_t;
    typedef uint8_t binaryTreeBool_t;
#endif /* BINARY_TREE_OP_RESULT_TYPE */

#define BINARY_TREE_OP_SUCCESS (0)
#define BINARY_TREE_OP_FAILURE (1)
#define BINARY_TREE_BOOL_TRUE  (1)
#define BINARY_TREE_BOOL_FALSE (0)

#endif /* __BINARY_TREE_COMPARE_GENERIC_VALUE_DEFINITION_HEADER__ */