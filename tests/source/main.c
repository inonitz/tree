#include <stdlib.h>
#include "StackTest.h"
#include "QueueTest.h"
#include "AVLTreeCTest.h"
#include "VectorTest.h"


int main(int argc, char *argv[])
{
    int result = 0;

    _putenv("CMOCKA_TEST_ABORT=1"); /* Incase of SIGABRT/SIGSEGV inside c_avl_tree_tests */
    result = run_all_stack_tests();
    result = run_all_queue_tests();
    result = run_all_c_avl_tree_tests();
    result = run_all_vector_tests();
    return result;
}
