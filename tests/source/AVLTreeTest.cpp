#include <gtest/gtest.h>
#include <random>
#include <iostream>
#include "AVLTree.hpp" 


class AVLTreeTest : public ::testing::Test {
protected:
    AVLTree tree;

    // Diagnostic helper to print results to the console
    void PrintDiagnostics(const std::string& section, int ins, int dels, int searches, uint32_t seed = 0) {
        std::cout << "\n[==========] " << section << " Diagnostics" << std::endl;
        if (seed != 0) std::cout << "             Seed:        " << seed << std::endl;
        std::cout << "             Insertions:  " << ins << std::endl;
        std::cout << "             Deletions:   " << dels << std::endl;
        std::cout << "             Searches:    " << searches << std::endl;
        std::cout << "             Final Size:  " << tree.size() << std::endl;
        std::cout << "             Tree Height: " << tree.getRoot()->m_height << std::endl;
    }
};


TEST_F(AVLTreeTest, BasicInsertionAndSearch) {
    EXPECT_TRUE(tree.isEmpty());
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    
    EXPECT_EQ(tree.size(), 3);
    EXPECT_TRUE(tree.search(50));
    EXPECT_TRUE(tree.search(30));
    EXPECT_FALSE(tree.search(100));
}


TEST_F(AVLTreeTest, SingleRotations) {
    // LL Case -> Right Rotation
    EXPECT_TRUE(tree.insert(30));
    EXPECT_TRUE(tree.insert(20));
    EXPECT_TRUE(tree.insert(10));
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());
    
    tree.clear();

    // RR Case -> Left Rotation
    EXPECT_TRUE(tree.insert(10));
    EXPECT_TRUE(tree.insert(20));
    EXPECT_TRUE(tree.insert(30));
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());
}


TEST_F(AVLTreeTest, DoubleRotations) {
    // LR Case -> Left then Right Rotation
    tree.insert(30); tree.insert(10); tree.insert(20);
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());
    tree.clear();

    // RL Case -> Right then Left Rotation
    tree.insert(10); tree.insert(30); tree.insert(20);
    EXPECT_EQ(tree.getRoot()->m_data, 20);
    EXPECT_TRUE(tree.isBalanced());
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
}


/* RANDOMIZED STRESS TEST */
TEST_F(AVLTreeTest, StochasticStressTest) {
    std::vector<int> mirror; 
    std::random_device rd;
    uint32_t seed = rd(); 
    std::mt19937 gen(seed);
    
    std::uniform_int_distribution<> val_dist(1, 100000);
    std::uniform_int_distribution<> op_dist(0, 2); // 0:Ins, 1:Del, 2:Search

    int ins = 0, dels = 0, searches = 0;
    const int TOTAL_OPS = 15000;

    for (int i = 0; i < TOTAL_OPS; ++i) {
        int val = val_dist(gen);
        int op = op_dist(gen);

        if (op == 0) { // Insert
            if (!tree.search(val)) {
                tree.insert(val);
                mirror.push_back(val);
                ins++;
            }
        } 
        else if (op == 1) { // Delete
            if (!mirror.empty()) {
                size_t idx = gen() % mirror.size();
                int to_remove = mirror[idx];
                tree.remove(to_remove);
                mirror.erase(mirror.begin() + idx);
                dels++;
            }
        } 
        else { // Search
            tree.search(val);
            searches++;
        }

        // Integrity check every 1000 ops
        if (i % 1000 == 0) {
            ASSERT_TRUE(tree.isBalanced()) << "Unbalanced at op " << i << " (Seed: " << seed << ")";
            ASSERT_TRUE(tree.isValidBST()) << "BST violation at op " << i << " (Seed: " << seed << ")";
            ASSERT_EQ(tree.size(), mirror.size()) << "Size mismatch at op " << i;
        }
    }


    PrintDiagnostics("Stochastic Stress", ins, dels, searches, seed);
    return;
}
