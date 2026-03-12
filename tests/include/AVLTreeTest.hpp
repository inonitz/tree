#pragma once
#include <gtest/gtest.h>
#include <util2/C/base_type.h>


#ifdef _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#endif


void setup_report_buffer();
void teardown_report_buffer();


class AVLTreeTest : public ::testing::Test {
protected:
public:
    enum class OpType : u8 {
        INSERT_OP,
        DELETE_OP,
        SEARCH_RAND_OP,
        SEARCH_SET_OP,
        MAX_OP
    };
};