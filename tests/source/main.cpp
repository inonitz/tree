#include <random>
#include <util2/C/ifcrash2.h>
#include "AVLTreeTest.hpp"




int main(int argc, char *argv[]) { 
    constexpr u64 treeSize = 100;
    constexpr u64 k_massiveBufferSize = 128 * 1024 * 1024;
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
    

    g_massiveBuffer = __rcast(char*, malloc(k_massiveBufferSize));
    g_reportFile    = fopen("report.txt", "w");
    ifcrash(g_reportFile == nullptr || g_massiveBuffer == nullptr);
    
    g_massiveBuffer[k_massiveBufferSize - 1] = '\0';
    // setbuf(g_reportFile, NULL);
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

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;

    int result = RUN_ALL_TESTS();  // Store the results in a variable



    (void)fprintf(g_reportFile, "%s", g_massiveBuffer);
    fclose(g_reportFile);
    free(g_massiveBuffer);
    return result;  // Return the result, as required by google test
}
