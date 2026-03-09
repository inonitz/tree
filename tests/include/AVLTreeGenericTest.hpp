#pragma once
#include <gtest/gtest.h>
#include <util2/C/macro.h>
#include <util2/C/base_type.h>
#include <string>


void setup_generic_report_buffer();
void teardown_generic_report_buffer();


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
public:
    enum class OperationType : u8 {
        INSERT_OP,
        DELETE_OP,
        SEARCH_RAND_OP,
        SEARCH_SET_OP,
        MAX_OP
    };
};


TYPED_TEST_SUITE(GenericAVLTreeTest, Implementations);




