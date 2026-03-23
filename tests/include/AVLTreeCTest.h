#ifndef __AVL_TREE_C_VERSION_TESTING_DEFINITION_HEADER__
#define __AVL_TREE_C_VERSION_TESTING_DEFINITION_HEADER__
#include <tree/C/treelib_extern.h>
#ifdef _MSC_VER
#   ifndef _CRT_SECURE_NO_WARNINGS
#       define _CRT_SECURE_NO_WARNINGS
#   endif /* _CRT_SECURE_NO_WARNINGS */
#endif


TREELIB_EXTERNC_DECL_BEGIN


int run_all_c_avl_tree_tests();

typedef enum __avl_test_random_operation_type {
    CTEST_AVL_OPER_INSERT_OP,
    CTEST_AVL_OPER_DELETE_OP,
    CTEST_AVL_OPER_SEARCH_RAND_OP,
    CTEST_AVL_OPER_SEARCH_SET_OP,
    CTEST_AVL_OPER_MAX_OP
} CTestOperationType;


TREELIB_EXTERNC_DECL_END


#endif /* __AVL_TREE_C_VERSION_TESTING_DEFINITION_HEADER__ */
