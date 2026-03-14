#pragma once
#include <gtest/gtest.h>
#include <util2/C/macro.h>
#include <util2/C/base_type.h>
#include <string>
#ifdef _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#endif


template<uint16_t Size> struct pack GenericMemoryBlob {
static_assert(Size > 8);

    u64 m_id;
    u8  m_mem[Size - 8];
};


struct DummyRecord 
{
    DummyRecord() : m_id{DEFAULT64} {}
    DummyRecord(uint64_t id) : m_id{id} {}

    bool operator<(const DummyRecord& other) const {
        return m_id < other.m_id;
    }
    bool operator>(const DummyRecord& other) const {
        return m_id > other.m_id;
    }
    bool operator==(const DummyRecord& other) const {
        return m_id == other.m_id;
    }

private:
    uint64_t m_id;
    double   m_values[8]{0};
    char     m_metadata[32]{0};
};




using Implementations = ::testing::Types<
    u64,
    u32,
    u16,
    u8,
    i64,
    i32,
    i16,
    i8,
    f32,
    f64,
    DummyRecord,
    std::string
>;


template<typename T>
class GenericAVLTreeTest : public ::testing::Test {
private:
    FILE* m_reportFile           = nullptr;
    char* m_massiveBuffer        = nullptr;
    u64   m_massiveBufferCurrIdx = 0;

protected:
    static constexpr const char* gk_test_report_name   = "generic_avl_test_report"; 
    static constexpr uint32_t    gk_stest_total_ops    = 1 * 1000 * 1000;
    static constexpr uint32_t    gk_stest_val_dist_min = 1;
    static constexpr uint32_t    gk_stest_val_dist_max = 100000;
    static constexpr u64         gk_massiveBufferSize  = 128ull * 1024 * 1024;
    
    virtual void SetUp();
    virtual void TearDown();
public:

    enum class OperationType : u8 {
        INSERT_OP,
        DELETE_OP,
        SEARCH_RAND_OP,
        SEARCH_SET_OP,
        MAX_OP
    };

    void generic_write_to_test_buffer(const char* formatstr, ...);
    void printTreeToMassiveBuf(void const* root, int space);
};


TYPED_TEST_SUITE(GenericAVLTreeTest, Implementations);




