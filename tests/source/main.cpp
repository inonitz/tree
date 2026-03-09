#include <gtest/gtest.h>
#include "AVLTreeTest.hpp"
#include "AVLTreeGenericTest.hpp"


int main(int argc, char *argv[]) 
{
    setup_report_buffer();
    setup_generic_report_buffer();

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;
    int result = RUN_ALL_TESTS();
    
    
    teardown_report_buffer();
    teardown_generic_report_buffer();
    return result;
}
