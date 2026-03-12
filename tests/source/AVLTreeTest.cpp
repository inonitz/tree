#include "AVLTreeTest.hpp"
#include <util2/C/aligned_malloc.h>
#include <util2/C/ifcrash2.h>
#include <tmp/AVLTreeDraft.hpp>
#include <tmp/binaryTreeDraft.hpp>
#include <random>
#include <cinttypes>


static FILE* g_reportFile           = nullptr;
static char* g_massiveBuffer        = nullptr;
static u64   g_massiveBufferCurrIdx = 0;
static constexpr uint32_t gk_stest_total_ops    = 1 * 1000 * 1000;
static constexpr uint32_t gk_stest_val_dist_min = 1;
static constexpr uint32_t gk_stest_val_dist_max = 100000;
static constexpr u64      gk_massiveBufferSize  = 128ull * 1024 * 1024;

#define write_to_test_buffer(formatstr, ...) \
    g_massiveBufferCurrIdx += sprintf(&g_massiveBuffer[g_massiveBufferCurrIdx], formatstr ,##__VA_ARGS__); \
    ifcrashfmt(g_massiveBufferCurrIdx >= gk_massiveBufferSize, "Report Buffer Index Reached %" PRIu64 "/%" PRIu64 " Bytes\n", g_massiveBufferCurrIdx, gk_massiveBufferSize); \




void setup_report_buffer() {
    g_massiveBuffer = __rcast(char*, util2_aligned_malloc(gk_massiveBufferSize, CACHE_LINE_BYTES));
    g_reportFile    = fopen("avl_test_report.txt", "w");
    ifcrash(g_reportFile == nullptr || g_massiveBuffer == nullptr);
    
    g_massiveBuffer[gk_massiveBufferSize - 1] = '\0';
    return;
}

void teardown_report_buffer() {
    write_to_test_buffer("g_massiveBuffer Consumed %" PRIu64 "/%" PRIu64 " Bytes for %u Operations\n",  g_massiveBufferCurrIdx, gk_massiveBufferSize, gk_stest_total_ops);
    (void)fprintf(g_reportFile, "%s", g_massiveBuffer);
    fclose(g_reportFile);
    util2_aligned_free(g_massiveBuffer);
    return;
}


static void printTreeToMassiveBuf(binaryTree const* root, int space) {
    constexpr auto kCOUNT = 5;
    
    if (root == NULL) {
        return;
    }
    space += kCOUNT;
    
    
    printTreeToMassiveBuf(root->m_right, space);
    write_to_test_buffer("\n\n\n%*s%d (%u, %d)\n", space - kCOUNT, "", root->m_data, root->m_height, root->m_bf);
    printTreeToMassiveBuf(root->m_left, space);
    return;
}




TEST_F(AVLTreeTest, BasicInsertionAndSearch) {
    AVLTree tree;

    EXPECT_TRUE(tree.empty());
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    
    EXPECT_EQ(tree.size(), 3);
    EXPECT_TRUE(tree.search(50));
    EXPECT_TRUE(tree.search(30));
    EXPECT_FALSE(tree.search(100));
    tree.clear();
    return;
}


TEST_F(AVLTreeTest, SingleRotationsLeftLeft) {
    AVLTree tree;

    EXPECT_TRUE(tree.insert(30));
    EXPECT_TRUE(tree.insert(20));
    EXPECT_TRUE(tree.insert(10));
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());

    EXPECT_EQ(tree.size(), 3);
    EXPECT_TRUE(tree.search(10));
    EXPECT_TRUE(tree.search(20));
    EXPECT_TRUE(tree.search(30));
    EXPECT_FALSE(tree.search(40));
    tree.clear();
    return;
}

TEST_F(AVLTreeTest, SingleRotationsRightRight) {
    AVLTree tree;

    EXPECT_TRUE(tree.insert(10));
    EXPECT_TRUE(tree.insert(20));
    EXPECT_TRUE(tree.insert(30));
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());

    EXPECT_EQ(tree.size(), 3);
    EXPECT_TRUE(tree.search(10));
    EXPECT_TRUE(tree.search(20));
    EXPECT_TRUE(tree.search(30));
    EXPECT_FALSE(tree.search(40));
    tree.clear();
    return;
}


TEST_F(AVLTreeTest, DoubleRotationsLeftRight) {
    AVLTree tree;

    /* Rebalancing will rotate Left then Right */
    EXPECT_TRUE(tree.insert(30)); 
    EXPECT_TRUE(tree.insert(10)); 
    EXPECT_TRUE(tree.insert(20));
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());
    tree.clear();
    return;
}


TEST_F(AVLTreeTest, DoubleRotationsRightLeft) {
    AVLTree tree;

    /* Rebalancing will rotate Right then Left */
    EXPECT_TRUE(tree.insert(10));
    EXPECT_TRUE(tree.insert(30));
    EXPECT_TRUE(tree.insert(20));
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());
    tree.clear();
    return;
}


TEST_F(AVLTreeTest, DeletionRebalancing) {
    AVLTree tree;
    std::vector<int> vals = {50, 25, 75, 10, 35, 60, 90};

    for (int v : vals) {
        EXPECT_TRUE(tree.insert(v));
    }
    // Remove leaf
    EXPECT_TRUE(tree.remove(10));
    EXPECT_FALSE(tree.search(10));
    EXPECT_TRUE(tree.isBalanced());

    // Remove node with two children (often requires finding successor)
    EXPECT_TRUE(tree.remove(25));
    EXPECT_TRUE(tree.isBalanced());
    EXPECT_TRUE(tree.isValidBST());
    tree.clear();
    return;
}


TEST_F(AVLTreeTest, ManualVerificationInsertDeleteTest) {
    constexpr u64 treeSize = 100;
    std::random_device rd;
    std::mt19937       gen(rd());
    std::uniform_int_distribution<> distrib(0, 100);
    std::uniform_int_distribution<> distribIndices(0, treeSize - 1);

    auto generate_random_data = [&gen, &distrib](u32 dataAmount) -> std::vector<uint32_t> {
        std::vector<uint32_t> dataToGen(dataAmount);
        for (auto& i : dataToGen) {
            i = distrib(gen);
        }
        return dataToGen;
    };


    AVLTree test{};
    bool    opStatus = false;
    auto    data = generate_random_data(treeSize);
    int c = 0;
    for (auto& val : data) {
        opStatus = test.insert(val);

        write_to_test_buffer("--- 2D Tree Visualization (Rotate head left) ---\n");
        write_to_test_buffer("[c=%3u] Insertion Of %3u -> %s\n", c, val, opStatus ? "SUCCESS" : "FAILURE");
        printTreeToMassiveBuf(test.getRoot(), 0);
        write_to_test_buffer("Post Insertion AVL Tree Valid(?) %u\n", test.isBalanced());
        write_to_test_buffer("-----------------------------------------------\n");
        ++c;

        if(c % 10 == 0) {
            (void(0)); /* for debugging points */
        }
    }

    write_to_test_buffer("-----------------------------------------------\n");
    write_to_test_buffer("--------------------------------------------AAA\n");
    write_to_test_buffer("-----------------------------------------------\n");
    for (auto& val : data) {
        auto& randValToDel = data[ distribIndices(gen) ];
        opStatus = test.remove(randValToDel);
        
        write_to_test_buffer("--- 2D Tree Visualization (Rotate head left) ---\n");
        write_to_test_buffer("[c=%3u] Deletion Of %3u -> %s\n", c, randValToDel, opStatus ? "SUCCESS" : "FAILURE");
        printTreeToMassiveBuf(test.getRoot(), 0);
        write_to_test_buffer("Post Deletion AVL Tree Valid(?) %u\n", test.isBalanced());
        write_to_test_buffer("-----------------------------------------------\n");
        ++c;

        if(c % 10 == 0) {
            (void(0)); /* for debugging points */
        }
    }


    return;
}


/* RANDOMIZED STRESS TEST */
TEST_F(AVLTreeTest, StochasticStressTest) {
    AVLTree               testTree;
    std::vector<uint32_t> treeValueSet; 
    std::random_device rd;
    std::mt19937 gen;
    
    std::uniform_int_distribution<> val_dist(gk_stest_val_dist_min, gk_stest_val_dist_max);
    std::uniform_int_distribution<> op_dist(0, (u8)OpType::MAX_OP);
    

    uint32_t      seed = rd(); 
    uint32_t      val = 0;
    OpType op  = OpType::MAX_OP;
    uint32_t tmpValue    = 0;
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
    for (uint32_t i = 0; i < gk_stest_total_ops; ++i) {
        printf("\r\r\r\r\r\r");
        val = val_dist(gen);
        op = __scast(OpType, op_dist(gen) );


        switch(op) {
            case OpType::INSERT_OP:
            if(testTree.search(val) == true) {
                continue;
            }

            status = testTree.insert(val);
            treeValueSet.push_back(val);
            ++insertion[status ? 1 : 0];

            write_to_test_buffer("%07u: i %06u (%u)\n", i, val, status);
            break;

            case OpType::DELETE_OP:
            if (treeValueSet.empty()) {
                continue;
            }

            tmpValueIdx = gen() % treeValueSet.size();
            tmpValue    = treeValueSet[tmpValueIdx];
            status      = testTree.remove(tmpValue);
            treeValueSet.erase(treeValueSet.begin() + tmpValueIdx);
            ++deletion[status ? 1 : 0];

            write_to_test_buffer("%07u: d %06u (%u)\n",i, tmpValue, status);

            break;

            case OpType::SEARCH_RAND_OP:
            if(treeValueSet.empty()) {
                continue;
            }

            tmpValue = val;
            status = testTree.search(tmpValue);
            ++searchRandomValueOp;
            searchRandomValueSuccess += status;
            searchRandomValueFailure += !status;


            write_to_test_buffer("%07u: rs %06u (%u)\n", i, tmpValue, status);
            break;


            case OpType::SEARCH_SET_OP:
            if(treeValueSet.empty()) {
                continue;
            }

            tmpValueIdx = gen() % treeValueSet.size();
            tmpValue    = treeValueSet[tmpValueIdx];
            status = testTree.search(tmpValue);
            ++searchExistingValueOp;
            searchExistingValueSuccess += status;
            searchExistingValueFailure += !status;


            write_to_test_buffer("%07u: ss %06u (%u)\n", i, tmpValue, status);
            break;

            case OpType::MAX_OP:
            default:
            break;
        }


        // Integrity check every 1000 ops
        if (i % 10 == 0) {
            ASSERT_TRUE(testTree.isBalanced()) << "Unbalanced at op " << i << " (Seed: " << seed << ")";
            ASSERT_TRUE(testTree.isValidBST()) << "BST violation at op " << i << " (Seed: " << seed << ")";
            ASSERT_EQ(testTree.size(), treeValueSet.size()) << "Size mismatch at op " << i;
        }
        printf("%06u", i);
    }
    printf("\n");

    write_to_test_buffer("\n[==========] Stochastic Stress Diagnostics\n");
    write_to_test_buffer("             Seed:                                       %u\n", seed);
    write_to_test_buffer("             Insertions              (Success, Failure): %06u %06u\n", insertion[1], insertion[0]);
    write_to_test_buffer("             Deletions               (Success, Failure): %06u %06u\n", deletion[1] , deletion[0] );
    write_to_test_buffer("             Searches                (Random, Existing): %06u %06u\n", searchRandomValueOp,        searchExistingValueOp     );
    write_to_test_buffer("             Random   Value Searches (Success, Failure): %06u %06u\n", searchRandomValueSuccess,   searchRandomValueFailure  );
    write_to_test_buffer("             Existing Value Searches (Success, Failure): %06u %06u\n", searchExistingValueSuccess, searchExistingValueFailure);
    write_to_test_buffer("             Final Size : %" PRIu64 "\n", testTree.size());
    write_to_test_buffer("             Tree Height: %u\n", testTree.getRoot()->m_height);
    return;
}
