#pragma once
#include <cstdio>
#include <gtest/gtest.h>
#include <tree/AVLTree.hpp>
#include <util2/C/ifcrash2.h>


extern FILE*    g_reportFile;
extern char*    g_massiveBuffer;
extern u64      g_massiveBufferCurrIdx;
extern const u32 gk_stest_total_ops;
extern const u32 gk_stest_val_dist_min;
extern const u32 gk_stest_val_dist_max;
extern const u64 gk_massiveBufferSize;


#define write_to_test_buffer(formatstr, ...) \
    g_massiveBufferCurrIdx += sprintf(&g_massiveBuffer[g_massiveBufferCurrIdx], formatstr ,##__VA_ARGS__); \
    ifcrashfmt(g_massiveBufferCurrIdx >= gk_massiveBufferSize, "Report Buffer Index Reached %llu/%llu Bytes\n", g_massiveBufferCurrIdx, gk_massiveBufferSize); \


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