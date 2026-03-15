#pragma once
#include <gtest/gtest.h>
#include <util2/C/base_type.h>
#ifdef _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#endif


class AVLTreeTest : public ::testing::Test {
private:
    FILE* m_reportFile           = nullptr;
    char* m_massiveBuffer        = nullptr;
    u64   m_massiveBufferCurrIdx = 0;

protected:
    static constexpr const char* gk_test_report_name   = "avl_test_report.txt"; 
    static constexpr uint32_t gk_stest_total_ops    = 1 * 1000 * 1000;
    static constexpr uint32_t gk_stest_val_dist_min = 1;
    static constexpr uint32_t gk_stest_val_dist_max = 100000;
    static constexpr u64      gk_massiveBufferSize  = 128ull * 1024 * 1024;

    virtual void SetUp();
    virtual void TearDown();
public:

    enum class OpType : u8 {
        INSERT_OP,
        DELETE_OP,
        SEARCH_RAND_OP,
        SEARCH_SET_OP,
        MAX_OP
    };


    void write_to_test_buffer(const char* formatstr, ...);
    void printTreeToMassiveBuf(void const* root, int space);
};