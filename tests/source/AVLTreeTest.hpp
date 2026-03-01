#pragma once
#include <cstdio>
#include <gtest/gtest.h>
#include <util2/C/base_type.h>
#include "AVLTree.hpp"


extern FILE*    g_reportFile;
extern char*    g_massiveBuffer;
extern uint32_t g_massiveBufferCurrIdx;


#define write_to_test_buffer(formatstr, ...) \
    g_massiveBufferCurrIdx += sprintf(&g_massiveBuffer[g_massiveBufferCurrIdx], formatstr, __VA_ARGS__); \


void printTreeToMassiveBuf(binaryTree const* root, int space);


class AVLTreeTest : public ::testing::Test {
protected:
    AVLTree tree;

public:
    enum class OperationType : u8 {
        INSERT_OP,
        DELETE_OP,
        SEARCH_RAND_OP,
        SEARCH_SET_OP,
        MAX_OP
    };
};