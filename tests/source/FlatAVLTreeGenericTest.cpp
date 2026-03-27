#include "FlatAVLTreeGenericTest.hpp"
#include <tree/FlatAVLTreeImpl.hpp>
#include <shared_mutex>
#include <utility>
#include <iomanip>
#include <random>
#include <array>


static std::random_device g_testSeedGenerator;
static uint32_t           g_testSeed = UINT32_MAX;


class NullStreamBuffer : public std::streambuf {
public:
    int overflow(int c) override {
        return c;
    }
};

struct LogStreamProxy
{
    std::fstream&                       m_stream;
    std::shared_lock<std::shared_mutex> m_lock;

    LogStreamProxy(std::shared_mutex& mtx, std::fstream& os)
        : m_lock(mtx), m_stream(os) {}

    template <typename T> LogStreamProxy& operator<<(const T& val) {
        m_stream << val;
        return *this;
    }

    LogStreamProxy& operator<<(std::ostream& (*manip)(std::ostream&)) {
        manip(m_stream);
        return *this;
    }
};

class AtomicLogger {
public:
    AtomicLogger(
        const std::string& filename,
        bool               toLogInitially,
        bool               flushAfterWrite
    ) : m_logging(toLogInitially)
    {
        m_fileStream.open(filename, std::ios::out | std::ios::app);
        m_originalStreamBufHandle = m_fileStream.rdbuf();
        m_nullStreamBufHandle     = &m_nullStreamBuf;

        /* Reserve Temp Buffer for Stream buffering, default should be (?) 4KiB */
        m_streamBufferUnderlyingMemory.resize(k_bufferingMemorySize);
        m_originalStreamBufHandle->pubsetbuf(
            m_streamBufferUnderlyingMemory.data(),
            k_bufferingMemorySize
        );


        if(!toLogInitially) {
            m_fileStream.set_rdbuf(m_nullStreamBufHandle);
        }
        return;
    }

    ~AtomicLogger() {
        std::unique_lock lock(m_bufferMutex);

        m_fileStream.set_rdbuf(m_originalStreamBufHandle);
        if(m_fileStream.is_open()) {
            m_fileStream.close();
        }
        m_streamBufferUnderlyingMemory.clear();
        return;
    }


    LogStreamProxy get() {
        return LogStreamProxy(m_bufferMutex, m_fileStream);
    }

    void setLogging(bool enable) {
        std::unique_lock lock(m_bufferMutex); /* Wait for all Proxy operations to finish */
        if (m_logging == enable)
            return;

        m_fileStream.set_rdbuf(enable ? m_originalStreamBufHandle : m_nullStreamBufHandle);
        m_logging = enable;
        return;
    }

    /* Useful for debugging, when needing immediate output */
    void setBufferingState(bool flushAfterWrite) {
        std::unique_lock lock(m_bufferMutex);
        m_fileStream << (flushAfterWrite ? std::unitbuf : std::nounitbuf);
        return;
    }


private:
    static constexpr uint64_t k_bufferingMemorySize = 128 * 1024;

    bool    m_logging;
    uint8_t m_reserved[7];
    std::shared_mutex m_bufferMutex;
    std::streambuf*   m_originalStreamBufHandle;
    std::streambuf*   m_nullStreamBufHandle;
    std::vector<char> m_streamBufferUnderlyingMemory;
    std::fstream      m_fileStream;
    NullStreamBuffer  m_nullStreamBuf;
};




template<typename T> void FlatAVLTreeGenericTest<T>::SetUp() {
    g_testSeed = g_testSeedGenerator();
    /* g_testSeed = 3403245766; */

    m_reportFile = std::make_unique<AtomicLogger>(gk_test_report_name,
        UTIL2_DEBUG_BUILD,
        UTIL2_DEBUG_BUILD
    );
    (m_reportFile->get() << "Test Begin (Seed=") << g_testSeed << ")\n";
    return;
}

template<typename T> void FlatAVLTreeGenericTest<T>::TearDown() {
    static const auto getFileSize = [](const char* filePath) -> std::streampos {
        std::streampos fsize = 0;
        std::ifstream file( filePath, std::ios::binary );

        fsize = file.tellg();
        file.seekg( 0, std::ios::end );
        fsize = file.tellg() - fsize;
        file.close();
        return fsize;
    };

    m_reportFile->get() << "Test End\nTest Report Memory Consumption: " << getFileSize(gk_test_report_name) << " Bytes\n";
    m_reportFile.reset();
    return;
}

template<typename T> auto FlatAVLTreeGenericTest<T>::log() {
    return m_reportFile->get();
}


template<typename T, typename OutputStream>
OutputStream&& IterationLogBegin(OutputStream&& stream, FlatAVLTree<T> const& tree, uint32_t iteration) {
    const std::string hash80(80, '#');
    const std::string dash79(79, '-');
    const std::string right58(58, '>');
    stream
        << "\n" << hash80
        << "\n# [ ITERATION BEGIN [" << iteration << "] ]" << right58
        << "\n#" << dash79
        << "\n# Tree State: [ Size: " << std::right << std::setw(6) << std::setfill(' ') << tree.size() << " ] "
        << "[ Height: "               << std::right << std::setw(2) << std::setfill(' ') << static_cast<int32_t>(tree.height()) << " ]"
        << "\n# Iteration:  [  "      << std::right << std::setw(10) << std::setfill(' ') << static_cast<int32_t>(tree.height()) << " ]"
        << "\n" << hash80 << "\n";

    return std::forward<OutputStream>(stream);
}

template<typename OutputStream>
OutputStream&& IterationLogEnd(OutputStream&& stream, uint32_t iteration) {
    const std::string hash80(80, '#');
    const std::string dash79(79, '-');
    const std::string left60(60, '<');
    stream
        << "\n" << hash80
        << "\n# [ ITERATION END [" << iteration << "] ]" << left60
        << "\n" << hash80 << "\n";

    return std::forward<OutputStream>(stream);
}




template<typename T>
static T generateRandomNumericalValue(
    uint32_t min = 0,
    uint32_t max = UINT32_MAX
) {
    static std::mt19937 gen(g_testSeed);
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

    static std::mt19937 gen(g_testSeed);
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




TYPED_TEST(FlatAVLTreeGenericTest, BasicInsertionAndSearch) {
    FlatAVLTree<TypeParam> testTree;
    auto                   testData = generateDataForSimpleTypedTests<TypeParam>();

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


TYPED_TEST(FlatAVLTreeGenericTest, SingleRotationsLeftLeft) {
    FlatAVLTree<TypeParam> testTree;
    std::stack<TypeParam>  expectedOrder;
    auto                   testData = generateDataForSimpleTypedTests<TypeParam>();

    expectedOrder.push(testData[2]);
    expectedOrder.push(testData[0]);
    expectedOrder.push(testData[1]);
    EXPECT_TRUE(testTree.insert(testData[2]));
    EXPECT_TRUE(testTree.insert(testData[1]));
    EXPECT_TRUE(testTree.insert(testData[0]));
    for(const auto& value : testTree.level_order_range()) {
        EXPECT_EQ(value, expectedOrder.top());
        expectedOrder.pop();
    }
    EXPECT_TRUE(testTree.isBalanced());
    EXPECT_EQ(testTree.size(), 3);

    EXPECT_TRUE(testTree.search(testData[0]));
    EXPECT_TRUE(testTree.search(testData[1]));
    EXPECT_TRUE(testTree.search(testData[2]));
    EXPECT_FALSE(testTree.search(testData[3]));
    testTree.clear();
    return;
}

TYPED_TEST(FlatAVLTreeGenericTest, SingleRotationsRightRight) {
    FlatAVLTree<TypeParam> testTree;
    std::stack<TypeParam>  expectedOrder;
    auto                   testData = generateDataForSimpleTypedTests<TypeParam>();

    expectedOrder.push(testData[3]);
    expectedOrder.push(testData[1]);
    expectedOrder.push(testData[2]);
    EXPECT_TRUE(testTree.insert(testData[1]));
    EXPECT_TRUE(testTree.insert(testData[2]));
    EXPECT_TRUE(testTree.insert(testData[3]));
    for(const auto& value : testTree.level_order_range()) {
        EXPECT_EQ(value, expectedOrder.top());
        expectedOrder.pop();
    }
    EXPECT_TRUE(testTree.isBalanced());

    EXPECT_EQ(testTree.size(), 3);
    EXPECT_TRUE(testTree.search(testData[1]));
    EXPECT_TRUE(testTree.search(testData[2]));
    EXPECT_TRUE(testTree.search(testData[3]));
    EXPECT_FALSE(testTree.search(testData[0]));
    testTree.clear();
    return;
}

TYPED_TEST(FlatAVLTreeGenericTest, DoubleRotationsLeftRight) {
    /* Rebalancing will rotate Left then Right */
    FlatAVLTree<TypeParam> testTree;
    std::stack<TypeParam>  expectedOrder;
    auto                   testData = generateDataForSimpleTypedTests<TypeParam>();

    expectedOrder.push(testData[2]);
    expectedOrder.push(testData[0]);
    expectedOrder.push(testData[1]);
    EXPECT_TRUE(testTree.empty());
    EXPECT_TRUE(testTree.insert(testData[2]));
    EXPECT_TRUE(testTree.insert(testData[0]));
    EXPECT_TRUE(testTree.insert(testData[1]));
    for(const auto& value : testTree.level_order_range()) {
        EXPECT_EQ(value, expectedOrder.top());
        expectedOrder.pop();
    }
    EXPECT_TRUE(testTree.isBalanced());
    testTree.clear();
    return;
}

TYPED_TEST(FlatAVLTreeGenericTest, DoubleRotationsRightLeft) {
    /* Rebalancing will rotate Right then Left */
    FlatAVLTree<TypeParam> testTree;
    std::stack<TypeParam>  expectedOrder;
    auto                   testData = generateDataForSimpleTypedTests<TypeParam>();

    expectedOrder.push(testData[2]);
    expectedOrder.push(testData[0]);
    expectedOrder.push(testData[1]);
    EXPECT_TRUE(testTree.insert(testData[0]));
    EXPECT_TRUE(testTree.insert(testData[2]));
    EXPECT_TRUE(testTree.insert(testData[1]));
    for(const auto& value : testTree.level_order_range()) {
        EXPECT_EQ(value, expectedOrder.top());
        expectedOrder.pop();
    }
    EXPECT_TRUE(testTree.isBalanced());
    testTree.clear();
    return;
}


TYPED_TEST(FlatAVLTreeGenericTest, DeletionRebalancing) {
    FlatAVLTree<TypeParam>   tree;
    std::array<TypeParam, 7> testData = generateDataForRebalanceTest<TypeParam>();

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
    tree.clear();
    return;
}




TYPED_TEST(FlatAVLTreeGenericTest, StochasticStressTest) {
    using OpType = typename FlatAVLTreeGenericTest<TypeParam>::OperationType;

    FlatAVLTree<TypeParam> tree;
    std::vector<TypeParam> treeValueSet;
    std::mt19937 gen;

    std::uniform_int_distribution<> op_dist(0, (u8)OpType::MAX_OP);

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


    gen.seed(g_testSeed);
    for (uint32_t i = 0; i < FlatAVLTreeGenericTest<TypeParam>::gk_stest_total_ops; ++i) {
        printf("\r\r\r\r\r\r");
        /* Debug breakpoints for different scenarios for commented out seed in SetUp() */
        // util2_debugbreakif(i == 2);
        // util2_debugbreakif(i == 41);
        // util2_debugbreakif(i == 47);
        // util2_debugbreakif(i == 136);
        // IterationLogBegin(this->log(), tree, i);


        val = generateValueForStressTest<TypeParam>();
        op = __scast(OpType, op_dist(gen) );
        switch(op) {
            case OpType::INSERT_OP:
            if(tree.search(val) == true) {
                this->log()
                    << std::right
                    << std::setw(7)
                    << std::setfill(' ')
                    << i
                    << ": ins (tree.search(val) == true)\n";
                continue;
            }
            status = tree.insert(val);
            treeValueSet.push_back(val);
            ++insertion[status ? 1 : 0];
            this->log()
                << std::right
                << std::setw(7)
                << std::setfill(' ')
                << i
                << ": ins (" << status << ")\n";
            break;

            case OpType::DELETE_OP:
            if (treeValueSet.empty()) {
                this->log()
                    << std::right
                    << std::setw(7)
                    << std::setfill(' ')
                    << i
                    << ": del (valueSet.empty() == true)\n";
                continue;
            }
            tmpValueIdx = gen() % treeValueSet.size();
            tmpValue    = treeValueSet[tmpValueIdx];
            status      = tree.remove(tmpValue);
            treeValueSet.erase(treeValueSet.begin() + tmpValueIdx);
            ++deletion[status ? 1 : 0];
            this->log()
                << std::right
                << std::setw(7)
                << std::setfill(' ')
                << i
                << ": del (" << status << ")\n";
            break;

            case OpType::SEARCH_RAND_OP:
            if(treeValueSet.empty()) {
                this->log()
                    << std::right
                    << std::setw(7)
                    << std::setfill(' ')
                    << i
                    << ": rndsearch (valueSet.empty() == true)\n";
                continue;
            }
            tmpValue = val;
            status = tree.search(tmpValue);
            ++searchRandomValueOp;
            searchRandomValueSuccess += status;
            searchRandomValueFailure += !status;
            this->log()
                << std::right
                << std::setw(7)
                << std::setfill(' ')
                << i
                << ": rndsearch (" << status << ")\n";
            break;


            case OpType::SEARCH_SET_OP:
            if(treeValueSet.empty()) {
                this->log()
                << std::right
                << std::setw(7)
                << std::setfill(' ')
                << i
                << ": setsearch (valueSet.empty() == true)\n";
                continue;
            }
            tmpValueIdx = gen() % treeValueSet.size();
            tmpValue    = treeValueSet[tmpValueIdx];
            status = tree.search(tmpValue);
            ++searchExistingValueOp;
            searchExistingValueSuccess += status;
            searchExistingValueFailure += !status;
            this->log()
                << std::right
                << std::setw(7)
                << std::setfill(' ')
                << i
                << ": setsearch (" << status << ")\n";
            break;

            case OpType::MAX_OP:
            default:
            break;
        }


        // Integrity check every 1000 ops
        // if (i % 10 == 0) {
        // tree.print(4, this->log());
        ASSERT_TRUE(tree.isBalanced()) << "Unbalanced at op " << i << " (Seed: " << g_testSeed << ")";
        ASSERT_TRUE(tree.isValidBST()) << "BST violation at op " << i << " (Seed: " << g_testSeed << ")";
        ASSERT_EQ(tree.size(), treeValueSet.size()) << "Size mismatch at op " << i;
        // }
        printf("%06u", i);
        IterationLogEnd(this->log(), i);
    }
    printf("\n");


    this->log() << "\n[==========] Stochastic Stress Diagnostics\n";
    this->log() << "             Seed:                                       " << g_testSeed << "\n";
    this->log() << "             Insertions              (Success, Failure): " << std::setw(6) << std::setfill('0') << insertion[1]               << " " << std::setw(6) << std::setfill('0') << insertion[0]               << "\n";
    this->log() << "             Deletions               (Success, Failure): " << std::setw(6) << std::setfill('0') << deletion[1]                << " " << std::setw(6) << std::setfill('0') << deletion[0]                << "\n";
    this->log() << "             Searches                (Random, Existing): " << std::setw(6) << std::setfill('0') << searchRandomValueOp        << " " << std::setw(6) << std::setfill('0') << searchExistingValueOp      << "\n";
    this->log() << "             Random   Value Searches (Success, Failure): " << std::setw(6) << std::setfill('0') << searchRandomValueSuccess   << " " << std::setw(6) << std::setfill('0') << searchRandomValueFailure   << "\n";
    this->log() << "             Existing Value Searches (Success, Failure): " << std::setw(6) << std::setfill('0') << searchExistingValueSuccess << " " << std::setw(6) << std::setfill('0') << searchExistingValueFailure << "\n";
    this->log() << "             Final Size : " << tree.size() << "\n";
    this->log() << "             Tree Height: " << tree.height() << "\n";
    tree.clear();
    return;
}
