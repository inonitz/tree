#include <util2/C/macro.h>
#include <util2/C/random.h>
#include <util2/C/env.h>
#include <stdio.h>
#include "StackTest.h"
#include "QueueTest.h"
#include "AVLTreeCTest.h"
#include "VectorTest.h"


int main(__unused int argc, __unused char *argv[])
{
    int result = 0;
    uint32_t seed[8] = {0};

    (void)random8u(); /* Dummy call to init RNG */
    randomGetSeed(&seed[0], 8);
    printf("Debug Test\nRandom Seed Used:\n(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n", 
        seed[0], seed[1], seed[2], seed[3],
        seed[4], seed[5], seed[6], seed[7]
    );


    util2_putenv("CMOCKA_TEST_ABORT=1"); /* Incase of SIGABRT/SIGSEGV inside c_avl_tree_tests */
    result = run_all_stack_tests(); /* util2 C RNG called first here, init here */
    result = run_all_queue_tests();
    result = run_all_c_avl_tree_tests();
    result = run_all_vector_tests();
    return result;
}
