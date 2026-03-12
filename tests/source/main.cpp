#include <gtest/gtest.h>
#include "AVLTreeTest.hpp"
#include "AVLTreeGenericTest.hpp"
#include "StackTest.h"
#include "QueueTest.h"
#include "AVLTreeCTest.h"


int main(int argc, char *argv[]) 
{
    int result = 0;
    setup_report_buffer();
    setup_generic_report_buffer();

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;
    // result = RUN_ALL_TESTS();
    result = run_all_stack_tests();
    result = run_all_queue_tests();
    result = run_all_c_avl_tree_tests();
    
    teardown_report_buffer();
    teardown_generic_report_buffer();
    return result;
}
