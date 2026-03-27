#include "AVLTreeGenericTest.hpp"
#include <util2/C/aligned_malloc.h>
#include <util2/C/ifcrash2.h>
#include <tree/AVLTreeImpl.hpp>
#include <cinttypes>
#include <iomanip>
#include <random>
#include <array>
#include <cstdarg>


template<typename T>
void GenericAVLTreeTest<T>::generic_write_to_test_buffer(const char* formatstr, ...) {
    va_list args;
    va_start(args, formatstr);

    u64 bytesWritten = vsnprintf(&m_massiveBuffer[m_massiveBufferCurrIdx], gk_massiveBufferSize - m_massiveBufferCurrIdx, formatstr, args);
    
    va_end(args);

    m_massiveBufferCurrIdx += (bytesWritten > 0) ? bytesWritten : 0;
    ifcrashfmt(m_massiveBufferCurrIdx >= gk_massiveBufferSize, 
        "Report Buffer Index Reached %" PRIu64 "/%" PRIu64 " Bytes\n", 
        m_massiveBufferCurrIdx, 
        gk_massiveBufferSize
    );
}


template<typename T> void GenericAVLTreeTest<T>::SetUp() {
    m_massiveBuffer = __rcast(char*, util2_aligned_malloc(gk_massiveBufferSize, CACHE_LINE_BYTES));
    m_reportFile    = fopen(gk_test_report_name, "w");
    ifcrash(m_reportFile == nullptr || m_massiveBuffer == nullptr);
    
    m_massiveBuffer[gk_massiveBufferSize - 1] = '\0';
    return;
}

template<typename T> void GenericAVLTreeTest<T>::TearDown() {
    generic_write_to_test_buffer("g_massiveBuffer Consumed %" PRIu64 "/%" PRIu64 " Bytes for %u Operations\n",  m_massiveBufferCurrIdx, gk_massiveBufferSize, gk_stest_total_ops);
    (void)fprintf(m_reportFile, "%s", m_massiveBuffer);
    fclose(m_reportFile);
    util2_aligned_free(m_massiveBuffer);
    return;
}


template<typename T>
void GenericAVLTreeTest<T>::printTreeToMassiveBuf(void const* root, int space) {
    constexpr auto kCOUNT = 5;
    
    if (root == NULL) {
        return;
    }
    space += kCOUNT;
    
    binaryTree<T>* _root = (binaryTree<T>*)root;
    printTreeToMassiveBuf(_root->m_right, space);
    generic_write_to_test_buffer("\n\n\n%*s%d (%u, %d)\n", space - kCOUNT, "", 
        _root->m_data, 
        _root->m_height, 
        _root->m_bf
    );
    printTreeToMassiveBuf(_root->m_left, space);
    return;
}




template<typename T>
static T generateRandomNumericalValue(
    uint32_t min = 0,
    uint32_t max = UINT32_MAX
) {
    static std::random_device rd{};
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int64_t> dis(min + 1, max - 1);

    /* 
        When Casting to a smaller bit representation, there might be overflow 
        and the value may be truncated, e.g (uint16_t)(UINT32_MAX) -> 0xFFFF
    */
    T generatedVal = __scast(T, dis(gen)) - 1;
    return generatedVal;
}

static std::string generateRandomString(size_t length) {
    static const std::string charset = 
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    static std::random_device rd{};
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<size_t> dis(0, charset.size() - 1);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        result += charset[dis(gen)];
    }
    return result;
}




template<typename T>
static std::array<T, 4> generateDataForSimpleTypedTests() {
    return { 10, 20, 30, 40 };
}
template<> std::array<f32, 4> generateDataForSimpleTypedTests<>() {
    return { 1.0f, 2.0f, 3.0f, 5.0f };
}
template<> std::array<f64, 4> generateDataForSimpleTypedTests<>() {
    return { 1.0, 2.0, 3.0, 5.0 };
}
template<> std::array<DummyRecord, 4> generateDataForSimpleTypedTests<>() {
    return {
        DummyRecord{ 10 },
        DummyRecord{ 20 },
        DummyRecord{ 30 },
        DummyRecord{ 40 },
    };
}
template<> std::array<std::string, 4> generateDataForSimpleTypedTests<>() {
    auto to_padded_str = [](int val) {
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << val;
        return oss.str();
    };

    return {
        to_padded_str(10), 
        to_padded_str(20), 
        to_padded_str(30),
        to_padded_str(40)
    };
}



template<typename T> 
static std::array<T, 7> generateDataForRebalanceTest() {
    return { 
        __scast(T, 50), 
        __scast(T, 25), 
        __scast(T, 75), 
        __scast(T, 10), 
        __scast(T, 35), 
        __scast(T, 60), 
        __scast(T, 90) 
    };
}
template<> std::array<DummyRecord, 7> generateDataForRebalanceTest<>() {
    return { 
        DummyRecord{50}, 
        DummyRecord{25}, 
        DummyRecord{75}, 
        DummyRecord{10}, 
        DummyRecord{35}, 
        DummyRecord{60}, 
        DummyRecord{90} 
    };
}
template<> std::array<std::string, 7> generateDataForRebalanceTest<>() {
    auto to_padded_str = [](int val) {
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << val;
        return oss.str();
    };

    return {
        to_padded_str(50), to_padded_str(25), to_padded_str(75),
        to_padded_str(10), to_padded_str(35), to_padded_str(60),
        to_padded_str(90)
    };
}



// template<typename T> void generateDataForStressTest(std::vector<T>& out, uint32_t size) {
//     out.clear();
//     out.reserve(size);
//     for(; --size ;) {
//         out.push_back(generateRandomNumericalValue<T>());
//     }
//     return;
// }
// template<> void generateDataForStressTest(std::vector<std::string>& out, uint32_t size) {
//     out.clear();
//     out.reserve(size);
//     for(; --size ;) {
//         out.push_back(generateRandomString(10));
//     }
//     return;
// }
// template<> void generateDataForStressTest(std::vector<DummyRecord>& out, uint32_t size) {
//     out.clear();
//     out.reserve(size);
//     for(; --size ;) {
//         out.push_back(DummyRecord{ generateRandomNumericalValue<uint64_t>() });
//     }
//     return;
// }
template<typename T> 
static T generateValueForStressTest() {
    return generateRandomNumericalValue<T>();
}
template<> std::string generateValueForStressTest() {
    return generateRandomString(10);
}
template<> DummyRecord generateValueForStressTest() {
    return DummyRecord{ generateRandomNumericalValue<uint64_t>() };
}




TYPED_TEST(GenericAVLTreeTest, BasicInsertionAndSearch) {
    AVLTree<TypeParam> testTree;
    auto               testData = generateDataForSimpleTypedTests<TypeParam>();
    
    EXPECT_TRUE(testTree.empty());

    EXPECT_TRUE(testTree.insert(testData[0]));
    EXPECT_TRUE(testTree.insert(testData[1]));
    EXPECT_TRUE(testTree.insert(testData[2]));
    EXPECT_EQ(testTree.size(), 3);

    EXPECT_TRUE(testTree.search(testData[0]));
    EXPECT_TRUE(testTree.search(testData[1]));
    EXPECT_TRUE(testTree.search(testData[2]));
    EXPECT_FALSE(testTree.search(testData[3]));
    testTree.clear();
    return;
}


TYPED_TEST(GenericAVLTreeTest, SingleRotationsLeftLeft) {
    AVLTree<TypeParam> testTree;
    auto               testData = generateDataForSimpleTypedTests<TypeParam>();


    EXPECT_TRUE(testTree.insert(testData[2]));
    EXPECT_TRUE(testTree.insert(testData[1]));
    EXPECT_TRUE(testTree.insert(testData[0]));
    EXPECT_EQ(testTree.getRoot()->m_data.get(), testData[1]);
    EXPECT_TRUE(testTree.isBalanced());
    EXPECT_EQ(testTree.size(), 3);

    EXPECT_TRUE(testTree.search(testData[0]));
    EXPECT_TRUE(testTree.search(testData[1]));
    EXPECT_TRUE(testTree.search(testData[2]));
    EXPECT_FALSE(testTree.search(testData[3]));
    testTree.clear();
    return;
}

TYPED_TEST(GenericAVLTreeTest, SingleRotationsRightRight) {
    AVLTree<TypeParam> testTree;
    auto               testData = generateDataForSimpleTypedTests<TypeParam>();

    EXPECT_TRUE(testTree.insert(testData[1]));
    EXPECT_TRUE(testTree.insert(testData[2]));
    EXPECT_TRUE(testTree.insert(testData[3]));
    EXPECT_EQ(testTree.getRoot()->m_data.get(), testData[2]);
    EXPECT_TRUE(testTree.isBalanced());

    EXPECT_EQ(testTree.size(), 3);
    EXPECT_TRUE(testTree.search(testData[1]));
    EXPECT_TRUE(testTree.search(testData[2]));
    EXPECT_TRUE(testTree.search(testData[3]));
    EXPECT_FALSE(testTree.search(testData[0]));
    testTree.clear();
    return;
}

TYPED_TEST(GenericAVLTreeTest, DoubleRotationsLeftRight) {
    /* Rebalancing will rotate Left then Right */
    AVLTree<TypeParam> testTree;
    auto               testData = generateDataForSimpleTypedTests<TypeParam>();

    EXPECT_TRUE(testTree.empty());
    EXPECT_TRUE(testTree.insert(testData[2]));
    EXPECT_TRUE(testTree.insert(testData[0]));
    EXPECT_TRUE(testTree.insert(testData[1]));
    EXPECT_EQ(testTree.getRoot()->m_data.get(), testData[1]);
    EXPECT_TRUE(testTree.isBalanced());
    testTree.clear();
    return;
}

TYPED_TEST(GenericAVLTreeTest, DoubleRotationsRightLeft) {
    /* Rebalancing will rotate Right then Left */
    AVLTree<TypeParam> testTree;
    auto               testData = generateDataForSimpleTypedTests<TypeParam>();
    
    EXPECT_TRUE(testTree.insert(testData[0])); 
    EXPECT_TRUE(testTree.insert(testData[2])); 
    EXPECT_TRUE(testTree.insert(testData[1]));
    EXPECT_EQ(testTree.getRoot()->m_data.get(), testData[1]);
    EXPECT_TRUE(testTree.isBalanced());
    testTree.clear();
    return;
}


TYPED_TEST(GenericAVLTreeTest, DeletionRebalancing) {
    AVLTree<TypeParam> tree;
    auto               testData = generateDataForRebalanceTest<TypeParam>();

    for (auto& v : testData) {
        EXPECT_TRUE(tree.insert(v));
    }

    // Remove leaf
    EXPECT_TRUE(tree.remove(testData[3]));
    EXPECT_FALSE(tree.search(testData[3]));
    EXPECT_TRUE(tree.isBalanced());

    // Remove node with two children (often requires finding successor)
    EXPECT_TRUE(tree.remove(testData[1]));
    EXPECT_TRUE(tree.isBalanced());
    EXPECT_TRUE(tree.isValidBST());
    tree.clear();
}


/* RANDOMIZED STRESS TEST */
TYPED_TEST(GenericAVLTreeTest, StochasticStressTest) {
    using OpType = typename GenericAVLTreeTest<TypeParam>::OperationType;

    AVLTree<TypeParam>     tree;
    std::vector<TypeParam> treeValueSet;
    std::random_device rd;
    std::mt19937 gen;

    std::uniform_int_distribution<> op_dist(0, (u8)OpType::MAX_OP);
    

    uint32_t  seed = rd();
    TypeParam val{};
    OpType    op   = OpType::MAX_OP;
    TypeParam tmpValue{};
    uint32_t tmpValueIdx = 0;
    bool     searchedValueIsRandom = false;
    bool     status                = false;
    uint32_t insertion[2] = {0, 0};
    uint32_t deletion[2]  = {0, 0};
    uint32_t searchType[2]    = {0, 0 };
    uint32_t searchRandVal[2] = {0, 0};
    uint32_t searchInSet[2]   = {0, 0};

    uint32_t& searchRandomValueOp   = searchType[0];
    uint32_t& searchExistingValueOp = searchType[1];
    uint32_t& searchRandomValueSuccess = searchRandVal[0];
    uint32_t& searchRandomValueFailure = searchRandVal[1];
    uint32_t& searchExistingValueSuccess = searchInSet[0];
    uint32_t& searchExistingValueFailure = searchInSet[1];


    gen.seed(seed);
    for (uint32_t i = 0; i < GenericAVLTreeTest<TypeParam>::gk_stest_total_ops; ++i) {
        // printf("\r\r\r\r\r\r");
        val = generateValueForStressTest<TypeParam>();
        op = __scast(OpType, op_dist(gen) );


        switch(op) {
            case OpType::INSERT_OP:
            if(tree.search(val) == true) {
                continue;
            }

            status = tree.insert(val);
            treeValueSet.push_back(val);
            ++insertion[status ? 1 : 0];

            this->generic_write_to_test_buffer("%07u: i (%u)\n", i, status);
            break;

            case OpType::DELETE_OP:
            if (treeValueSet.empty()) {
                continue;
            }

            tmpValueIdx = gen() % treeValueSet.size();
            tmpValue    = treeValueSet[tmpValueIdx];
            status      = tree.remove(tmpValue);
            treeValueSet.erase(treeValueSet.begin() + tmpValueIdx);
            ++deletion[status ? 1 : 0];

            this->generic_write_to_test_buffer("%07u: d (%u)\n", i, status);

            break;

            case OpType::SEARCH_RAND_OP:
            if(treeValueSet.empty()) {
                continue;
            }

            tmpValue = val;
            status = tree.search(tmpValue);
            ++searchRandomValueOp;
            searchRandomValueSuccess += status;
            searchRandomValueFailure += !status;


            this->generic_write_to_test_buffer("%07u: rs (%u)\n", i, status);
            break;


            case OpType::SEARCH_SET_OP:
            if(treeValueSet.empty()) {
                continue;
            }

            tmpValueIdx = gen() % treeValueSet.size();
            tmpValue    = treeValueSet[tmpValueIdx];
            status = tree.search(tmpValue);
            ++searchExistingValueOp;
            searchExistingValueSuccess += status;
            searchExistingValueFailure += !status;


            this->generic_write_to_test_buffer("%07u: ss (%u)\n", i, status);
            break;

            case OpType::MAX_OP:
            default:
            break;
        }


        // Integrity check every 1000 ops
        if (i % 10 == 0) {
            ASSERT_TRUE(tree.isBalanced()) << "Unbalanced at op " << i << " (Seed: " << seed << ")";
            ASSERT_TRUE(tree.isValidBST()) << "BST violation at op " << i << " (Seed: " << seed << ")";
            ASSERT_EQ(tree.size(), treeValueSet.size()) << "Size mismatch at op " << i;
        }
        // printf("%06u", i);
    }
    printf("\n");

    this->generic_write_to_test_buffer("\n[==========] Stochastic Stress Diagnostics\n");
    this->generic_write_to_test_buffer("             Seed:                                       %u\n", seed);
    this->generic_write_to_test_buffer("             Insertions              (Success, Failure): %06u %06u\n", insertion[1], insertion[0]);
    this->generic_write_to_test_buffer("             Deletions               (Success, Failure): %06u %06u\n", deletion[1] , deletion[0] );
    this->generic_write_to_test_buffer("             Searches                (Random, Existing): %06u %06u\n", searchRandomValueOp,        searchExistingValueOp     );
    this->generic_write_to_test_buffer("             Random   Value Searches (Success, Failure): %06u %06u\n", searchRandomValueSuccess,   searchRandomValueFailure  );
    this->generic_write_to_test_buffer("             Existing Value Searches (Success, Failure): %06u %06u\n", searchExistingValueSuccess, searchExistingValueFailure);
    this->generic_write_to_test_buffer("             Final Size : %" PRIu64 "\n", tree.size());
    if(tree.getRoot()) {
        this->generic_write_to_test_buffer("             Tree Height: %u\n", tree.getRoot()->m_height);
    }

    tree.clear();
    return;
}
