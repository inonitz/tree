#include <gtest/gtest.h>
#include "StackTest.h"
#include "QueueTest.h"
#include "AVLTreeCTest.h"
#include "VectorTest.h"


int main(int argc, char *argv[]) 
{
    int result = 0;

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;

    // putenv("CMOCKA_TEST_ABORT=1"); /* Incase of SIGABRT/SIGSEGV inside c_avl_tree_tests */
    result = run_all_stack_tests();
    result = run_all_queue_tests();
    result = run_all_c_avl_tree_tests();
    result = run_all_vector_tests();

    /* 
        I can't believe I had to use google Gemini to find the filter flags... 
        --gtest_filter [TestSuiteName]/[InstantiatedType].[SpecificTestName]
    */
    // ::testing::GTEST_FLAG(filter) = "GenericRecursiveAVLTreeTest/*.*";
    // result = RUN_ALL_TESTS();
    // ::testing::GTEST_FLAG(filter) = "GenericAVLTreeTest*";
    // result = RUN_ALL_TESTS();
    // ::testing::GTEST_FLAG(filter) = "AVLTreeTest*";
    // result = RUN_ALL_TESTS();

    ::testing::GTEST_FLAG(filter) = "*";
    result = RUN_ALL_TESTS();
    return result;
}
