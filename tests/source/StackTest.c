#include "StackTest.h"
#include <cmocka.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <tree/C/stack.h>


typedef struct {
    int id;
    double value;
} TestData;


typedef struct {
    uint32_t m_id;
    uint32_t m_val;
} FuzzPayload;


typedef enum fuzzOperation {
    FUZZ_STACK_OPERATION_PUSH = 0,
    FUZZ_STACK_OPERATION_POP  = 1,
    FUZZ_STACK_OPERATION_TOP  = 2,
    FUZZ_STACK_OPERATION_MAX  = 3
} FuzzOp;


static inline void verify_stack_creation(
    GenericStack* toCreate,
    uint32_t      objSizeInBytes,
    uint32_t      maximumInitialAmountOfObjects
) {
    uint8_t status = 0;
    status = GenericStackCreate(toCreate, objSizeInBytes, maximumInitialAmountOfObjects);


    assert_int_equal(status, 0);
    assert_int_equal(toCreate->m_maxObjCount,
        maximumInitialAmountOfObjects == 0 ?
            1 : maximumInitialAmountOfObjects
    );
    assert_int_equal(GenericStackEmpty(toCreate), 1);
    assert_int_equal(GenericStackSize(toCreate),  0);
    return;
}




static void Stack_test_create(void **state) {
    (void)state;
    GenericStack stack;
    verify_stack_creation(&stack, sizeof(TestData), 2);


    assert_int_equal(GenericStackEmpty(&stack), 1);

    GenericStackDestroy(&stack);
    assert_null(stack.m_buffer);
    return;
}


static void Stack_test_single_push(void **state) {
    (void) state;
    uint8_t      status = 0;
    TestData     td1 = {1, 3.14};
    GenericStack stack;

    verify_stack_creation(&stack, sizeof(TestData), 2);
    assert_int_equal(GenericStackPush(&stack, &td1), 0);
    assert_int_equal(GenericStackEmpty(&stack), 0);

    GenericStackDestroy(&stack);
    return;
}


static void Stack_test_top_element(void **state) {
    (void) state;
    GenericStack stack;
    TestData td1    = {1, 3.14};
    TestData td_out = {0, 0.0};

    verify_stack_creation(&stack, sizeof(TestData), 2);
    assert_int_equal(GenericStackPush(&stack, &td1),   0);
    assert_int_equal(GenericStackTop(&stack, &td_out), 0);
    assert_int_equal(td_out.id, 1);

    GenericStackDestroy(&stack);
    return;
}


static void Stack_test_dynamic_growth(void **state) {
    (void) state;
    // void*    stackBufChanged = NULL;
    uint8_t  status = 0;
    TestData td1 = {1, 3.14};
    TestData td2 = {2, 6.28};
    TestData td3 = {3, 9.42};
    GenericStack stack;

    verify_stack_creation(&stack, sizeof(TestData), 128);
    assert_non_null(stack.m_buffer);
    assert_int_equal(stack.m_objSize, sizeof(TestData));
    // stackBufChanged = stack.m_buffer;

    assert_false(GenericStackPush(&stack, &td1));
    assert_false(GenericStackPush(&stack, &td2));

    /* Pushing a 3rd element triggers GenericStackGrow */
    assert_false(GenericStackPush(&stack, &td3));
    // assert_int_not_equal(stackBufChanged, stack.m_buffer);

    GenericStackDestroy(&stack);
    assert_null(stack.m_buffer);
    return;
}


static void Stack_test_pop_is_valid(void **state) {
    (void)state;
    GenericStack stack;
    TestData td1 = {1, 3.14};
    TestData td2 = {2, 6.28};
    TestData td_out;

    verify_stack_creation(&stack, sizeof(TestData), 2);
    assert_int_equal(GenericStackPush(&stack, &td1), 0);
    assert_int_equal(GenericStackPush(&stack, &td2), 0);

    GenericStackPop(&stack);
    assert_int_equal(stack.m_objCount,    1);
    assert_int_equal(stack.m_maxObjCount, 2);
    assert_int_equal(GenericStackTop(&stack, &td_out), 0);
    assert_int_equal(td_out.id, 1); /* Because td2 was popped, the top should now be td1 */

    GenericStackDestroy(&stack);
    return;
}


static void Stack_test_pop_after_empty(void **state) {
    (void)state;
    GenericStack stack;
    TestData td1 = {1, 3.14};

    verify_stack_creation(&stack, sizeof(TestData), 2);

    assert_int_equal(GenericStackPush(&stack, &td1), 0);

    /* Popping the lone element from the stack, should be empty now */
    GenericStackPop(&stack);
    assert_int_equal(GenericStackEmpty(&stack), 1);

    /* Edge case: Popping an already empty stack, should do nothing */
    GenericStackPop(&stack);
    assert_int_equal(GenericStackEmpty(&stack), 1);

    GenericStackDestroy(&stack);
    return;
}


static void Stack_test_destroy_valid(void **state) {
    (void)state;
    GenericStack stack;
    GenericStack emptyStack;

    GenericStackCreate(&stack, sizeof(TestData), (uint32_t)rand() % 10);
    GenericStackCreate(&emptyStack, 1, 1);

    GenericStackDestroy(&stack);
    GenericStackDestroy(&emptyStack);

    assert_int_equal(0, memcmp(&stack, &emptyStack, sizeof(GenericStack)));
    return;
}


static void Stack_test_fuzz_operations(void **state) {
    (void)state;
    static const uint64_t kGroundTruthBufMaxSize   = 4 * 1024 * 1024;
    static const uint64_t kMaximumRandomOperations = 4 * 1024 * 1024;
    FuzzPayload* groundTruthBuf = malloc(sizeof(FuzzPayload) * kGroundTruthBufMaxSize);
    uint32_t     groundTruthCnt = 0;
    FuzzOp       op             = FUZZ_STACK_OPERATION_MAX;
    FuzzPayload  tmpVal         = {};
    GenericStack stack;


    srand(0);
    verify_stack_creation(&stack, sizeof(FuzzPayload), 10);
    for (uint32_t i = 0; i < kMaximumRandomOperations; ++i)
    {
        op = rand() % FUZZ_STACK_OPERATION_MAX;

        switch(op) {
            case FUZZ_STACK_OPERATION_PUSH:
            tmpVal = (FuzzPayload){
                i,
                rand() % UINT32_MAX
            };
            groundTruthBuf[groundTruthCnt] = tmpVal;
            ++groundTruthCnt;

            assert_int_equal(GenericStackPush(&stack, &tmpVal), 0);
            break;

            case FUZZ_STACK_OPERATION_POP:
            if(groundTruthCnt > 0) {
                GenericStackPop(&stack);
                --groundTruthCnt;
            }
            break;

            case FUZZ_STACK_OPERATION_TOP:
            if (groundTruthCnt > 0) {
                assert_int_equal(GenericStackTop(&stack, &tmpVal), 0);
                assert_int_equal(tmpVal.m_id,  groundTruthBuf[groundTruthCnt - 1].m_id);
                assert_int_equal(tmpVal.m_val, groundTruthBuf[groundTruthCnt - 1].m_val);

            }
            break;

            default:
            break;
        }


        assert_int_equal(stack.m_objCount, groundTruthCnt);
    }


    free(groundTruthBuf);
    GenericStackDestroy(&stack);
    return;
}



int run_all_stack_tests() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(Stack_test_create),
        cmocka_unit_test(Stack_test_single_push),
        cmocka_unit_test(Stack_test_top_element),
        cmocka_unit_test(Stack_test_dynamic_growth),
        cmocka_unit_test(Stack_test_pop_is_valid),
        cmocka_unit_test(Stack_test_pop_after_empty),
        cmocka_unit_test(Stack_test_destroy_valid),
        cmocka_unit_test(Stack_test_fuzz_operations)
    };
    return cmocka_run_group_tests_name("Stack", tests, NULL, NULL);
}
