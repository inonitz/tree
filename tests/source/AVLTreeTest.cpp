#include "AVLTreeTest.hpp"
#include "binaryTree.hpp"
#include <random>

#include <util2/C/macro.h>



FILE*    g_reportFile           = nullptr;
char*    g_massiveBuffer        = nullptr;
uint32_t g_massiveBufferCurrIdx = 0;


void printTreeToMassiveBuf(binaryTree const* root, int space) {
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
    /* Rebalancing will rotate Left then Right */
    tree.insert(30); tree.insert(10); tree.insert(20);
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());
    tree.clear();
    return;
}


TEST_F(AVLTreeTest, DoubleRotationsRightLeft) {
    /* Rebalancing will rotate Right then Left */
    tree.insert(10); tree.insert(30); tree.insert(20);
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());
    tree.clear();
    return;
}



TEST_F(AVLTreeTest, DeletionRebalancing) {
    std::vector<int> vals = {50, 25, 75, 10, 35, 60, 90};
    for (int v : vals) tree.insert(v);

    // Remove leaf
    EXPECT_TRUE(tree.remove(10));
    EXPECT_FALSE(tree.search(10));
    EXPECT_TRUE(tree.isBalanced());

    // Remove node with two children (often requires finding successor)
    EXPECT_TRUE(tree.remove(25));
    EXPECT_TRUE(tree.isBalanced());
    EXPECT_TRUE(tree.isValidBST());
    tree.clear();
}


/* RANDOMIZED STRESS TEST */
TEST_F(AVLTreeTest, StochasticStressTest) {
    constexpr uint32_t TOTAL_OPS = 1000 * 1000;
    constexpr uint32_t val_dist_min = 1;
    constexpr uint32_t val_dist_max = 100000;
    std::vector<uint32_t> treeValueSet; 
    std::random_device rd;
    std::mt19937 gen;
    
    std::uniform_int_distribution<> val_dist(val_dist_min, val_dist_max);
    std::uniform_int_distribution<> op_dist(0, (u8)OperationType::MAX_OP);
    

    uint32_t      seed = rd(); 
    uint32_t      val = 0;
    OperationType op  = OperationType::MAX_OP;
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
    for (int i = 0; i < TOTAL_OPS; ++i) {
        val = val_dist(gen);
        op = __scast(OperationType, op_dist(gen) );


        switch(op) {
            case OperationType::INSERT_OP:
            if(tree.search(val) == true) {
                continue;
            }

            status = tree.insert(val);
            treeValueSet.push_back(val);
            ++insertion[status ? 1 : 0];

            write_to_test_buffer(
                "%03u: insertion of %06u -> %u\n", 
                i, val, status
            );
            break;

            case OperationType::DELETE_OP:
            if (treeValueSet.empty()) {
                continue;
            }

            tmpValueIdx = gen() % treeValueSet.size();
            tmpValue    = treeValueSet[tmpValueIdx];
            status      = tree.remove(tmpValue);
            treeValueSet.erase(treeValueSet.begin() + tmpValueIdx);
            ++deletion[status ? 1 : 0];

            write_to_test_buffer(
                "%03u: deletion  of %06u -> %u\n", 
                i, tmpValue, status
            );

            break;

            case OperationType::SEARCH_RAND_OP:
            if(treeValueSet.empty()) {
                continue;
            }

            tmpValue = val;
            status = tree.search(tmpValue);
            ++searchRandomValueOp;
            searchRandomValueSuccess += status;
            searchRandomValueFailure += !status;


            write_to_test_buffer(
                "%03u: rndsearch of %06u -> %u\n", 
                i, tmpValue, status
            );
            break;


            case OperationType::SEARCH_SET_OP:
            if(treeValueSet.empty()) {
                continue;
            }

            tmpValueIdx = gen() % treeValueSet.size();
            tmpValue    = treeValueSet[tmpValueIdx];
            status = tree.search(tmpValue);
            ++searchExistingValueOp;
            searchExistingValueSuccess += status;
            searchExistingValueFailure += !status;


            write_to_test_buffer(
                "%03u: setsearch of %06u -> %u\n", 
                i, tmpValue, status
            );
            break;

            case OperationType::MAX_OP:
            default:
            break;
        }


        // Integrity check every 1000 ops
        if (i % 50 == 0) {
            ASSERT_TRUE(tree.isBalanced()) << "Unbalanced at op " << i << " (Seed: " << seed << ")";
            ASSERT_TRUE(tree.isValidBST()) << "BST violation at op " << i << " (Seed: " << seed << ")";
            ASSERT_EQ(tree.size(), treeValueSet.size()) << "Size mismatch at op " << i;
        }
    }


    write_to_test_buffer("\n[==========] Stochastic Stress Diagnostics\n");
    write_to_test_buffer("             Seed:                                       %u\n", seed);
    write_to_test_buffer("             Insertions              (Success, Failure): %06u %06u\n", insertion[1], insertion[0]);
    write_to_test_buffer("             Deletions               (Success, Failure): %06u %06u\n", deletion[1] , deletion[0] );
    write_to_test_buffer("             Searches                (Random, Existing): %06u %06u\n", searchRandomValueOp,        searchExistingValueOp     );
    write_to_test_buffer("             Random   Value Searches (Success, Failure): %06u %06u\n", searchRandomValueSuccess,   searchRandomValueFailure  );
    write_to_test_buffer("             Existing Value Searches (Success, Failure): %06u %06u\n", searchExistingValueSuccess, searchExistingValueFailure);
    write_to_test_buffer("             Final Size : %llu\n", tree.size());
    write_to_test_buffer("             Tree Height: %u\n", tree.getRoot()->m_height);
    return;
}
