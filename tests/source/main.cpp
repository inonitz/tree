#include <random>
#include <gtest/gtest.h>
#include <util2/C/ifcrash2.h>
#include "AVLTree.hpp"


std::vector<int> generate_random_data(size_t size) {
    std::vector<int> data(size);
    std::random_device rd;  // Non-deterministic seed
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine
    std::uniform_int_distribution<> distrib(1, 100);

    for (int& i : data) {
        i = distrib(gen);
    }
    return data;
}


void printTree2D(FILE* outputFile, binaryTree* root, int space) {
    constexpr auto kCOUNT = 5;
    
    if (root == NULL) {
        return;
    }
    space += kCOUNT;
    
    
    printTree2D(outputFile, root->m_right, space);
    fprintf(outputFile, "\n\n\n%*s%d (%u, %d)\n", space - kCOUNT, "", root->m_data, root->m_height, root->m_bf);
    printTree2D(outputFile, root->m_left, space);
    return;
}



int main(int argc, char *argv[]) {
    constexpr auto treeSize = 50;
    

    FILE*              reportFile;
    std::random_device rd;
    std::mt19937       gen(rd());
    std::uniform_int_distribution<> distrib(0, 100);
    std::uniform_int_distribution<> distribIndices(0, treeSize - 1);

    AVLTree test{};
    bool    opStatus = false;
    
    
    auto data = generate_random_data(treeSize);
    

    reportFile = fopen("report.txt", "w");
    setbuf(reportFile, NULL);
    ifcrash(reportFile == nullptr);
    // status = test.insertValue(100);
    // printf(" ----------------Inserting %3u -> %s----------", 100, status ? "SUCCESS" : "FAILURE");
    // printTree2D(test.m_root, 0);
    // status = test.insertValue(0);
    // printf("-----------------------------------------------\n");
    // printf(" ----------------Inserting %3u -> %s----------", 0, status ? "SUCCESS" : "FAILURE");
    // printTree2D(test.m_root, 0);
    // status = test.insertValue(2);
    // printf("-----------------------------------------------\n");
    // printf(" ----------------Inserting %3u -> %s----------", 2, status ? "SUCCESS" : "FAILURE");
    // printTree2D(test.m_root, 0);
    // status = test.insertValue(2);
    // printf("-----------------------------------------------\n");
    // printf(" ----------------Inserting %3u -> %s----------", 2, status ? "SUCCESS" : "FAILURE");
    // printTree2D(test.m_root, 0);
    // status = test.insertValue(5);
    // printf("-----------------------------------------------\n");
    // printf(" ----------------Inserting %3u -> %s----------", 5, status ? "SUCCESS" : "FAILURE");
    // printTree2D(test.m_root, 0);
    // status =  test.insertValue(2);
    // printf("-----------------------------------------------\n");
    // printf(" ----------------Inserting %3u -> %s----------", 2, status ? "SUCCESS" : "FAILURE");
    // printTree2D(test.m_root, 0);
    // status = test.insertValue(7);
    // printf("-----------------------------------------------\n");
    // printf(" ----------------Inserting %3u -> %s----------", 7, status ? "SUCCESS" : "FAILURE");
    // printTree2D(test.m_root, 0);


    int c = 0;

    for (auto& val : data) {
        opStatus = test.insertValue(val);

        fprintf(reportFile, "--- 2D Tree Visualization (Rotate head left) ---\n");
        fprintf(reportFile, "[c=%3u] Insertion Of %3u -> %s\n", c, val, opStatus ? "SUCCESS" : "FAILURE");
        printTree2D(reportFile, test.m_root, 0);
        fprintf(reportFile, "Post Insertion AVL Tree Valid(?) %u\n", binaryTree::isValidAVL(test.m_root));
        fprintf(reportFile, "-----------------------------------------------\n");
        ++c;

        if(c % 10 == 0) {
            (void(0)); /* for debugging points */
        }
    }

    fprintf(reportFile, "-----------------------------------------------\n");
    fprintf(reportFile, "--------------------------------------------AAA\n");
    fprintf(reportFile, "-----------------------------------------------\n");
    for (auto& val : data) {
        auto& randValToDel = data[ distribIndices(gen) ];
        opStatus = test.deleteValue(randValToDel);
        
        fprintf(reportFile, "--- 2D Tree Visualization (Rotate head left) ---\n");
        fprintf(reportFile, "[c=%3u] Deletion Of %3u -> %s\n", c, randValToDel, opStatus ? "SUCCESS" : "FAILURE");
        printTree2D(reportFile, test.m_root, 0);
        fprintf(reportFile, "Post Deletion AVL Tree Valid(?) %u\n", binaryTree::isValidAVL(test.m_root));
        fprintf(reportFile, "-----------------------------------------------\n");
        ++c;

        if(c % 10 == 0) {
            (void(0)); /* for debugging points */
        }
    }

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();  // Store the results in a variable



    fclose(reportFile);
    return result;  // Return the result, as required by google test
}
