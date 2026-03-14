#include <gtest/gtest.h>
#include "StackTest.h"
#include "QueueTest.h"
#include "AVLTreeCTest.h"


int main(int argc, char *argv[]) 
{
    int result = 0;

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;

    putenv("CMOCKA_TEST_ABORT=1");
    result = run_all_stack_tests();
    result = run_all_queue_tests();
    result = run_all_c_avl_tree_tests();
    result = RUN_ALL_TESTS();
    
    return result;
}
