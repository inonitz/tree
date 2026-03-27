#include <stdlib.h>
#include <tree/C/queue.h>
#include <cmocka.h>


typedef struct {
    int id;
} QueueData;


typedef struct {
    uint32_t m_id;
    uint32_t m_val;
} FuzzPayload;


typedef enum fuzzOperation {
    FUZZ_QUEUE_OPERATION_ENQUEUE = 0,
    FUZZ_QUEUE_OPERATION_DEQUEUE = 1,
    FUZZ_QUEUE_OPERATION_FRONT   = 2,
    FUZZ_STACK_OPERATION_MAX     = 3
} FuzzOp;


static inline void verify_queue_creation(
    GenericQueue* toCreate,
    uint32_t      objSizeInBytes,
    uint32_t      maximumInitialAmountOfObjects
) {
    uint8_t status = 0;
    status = GenericQueueCreate(toCreate, objSizeInBytes, maximumInitialAmountOfObjects);


    assert_int_equal(status, 0);
    assert_int_equal(toCreate->m_maxObjCount,
        maximumInitialAmountOfObjects == 0 ?
            1 : maximumInitialAmountOfObjects
    );
    assert_int_equal(GenericQueueEmpty(toCreate), 1);
    assert_int_equal(GenericQueueSize(toCreate),  0);

    return;
}


static void Queue_test_create(void **state) {
    (void)state;
    GenericQueue q;

    verify_queue_creation(&q, sizeof(QueueData), 0);

    GenericQueueDestroy(&q);
    return;
}


static void Queue_test_basic_operations(void **state) {
    (void)state;
    GenericQueue q;
    QueueData d1 = {10};
    QueueData d_out = {0};


    verify_queue_creation(&q, sizeof(QueueData), 2);

    assert_int_equal(GenericQueuePush(&q, &d1), 0);
    assert_int_equal(GenericQueueSize(&q), 1);
    assert_int_equal(GenericQueueEmpty(&q), 0);

    GenericQueueFront(&q, &d_out);
    assert_int_equal(d_out.id, 10);

    GenericQueuePop(&q);
    assert_int_equal(GenericQueueEmpty(&q), 1);

    GenericQueueDestroy(&q);
    return;
}


static void Queue_test_circular_wrap(void **state) {
    (void) state;
    GenericQueue q;
    QueueData d1 = {1}, d2 = {2}, d3 = {3}, d4 = {4};
    QueueData d_out;

    verify_queue_creation(&q, sizeof(QueueData), 3);

    /* Queue is now full: [1, 2, 3], head=0, tail=0 */
    assert_int_equal(GenericQueuePush(&q, &d1), 0);
    assert_int_equal(GenericQueuePush(&q, &d2), 0);
    assert_int_equal(GenericQueuePush(&q, &d3), 0);

    /* Pop 1: [_, 2, 3], head=1, tail=0 */
    GenericQueuePop(&q);

    /* Push 4 at tail 0: [4, 2, 3], head=1, tail=1 */
    assert_int_equal(GenericQueuePush(&q, &d4), 0);

    /* Front should be 2 */
    GenericQueueFront(&q, &d_out);
    assert_int_equal(d_out.id, 2);

    GenericQueueDestroy(&q);
    return;
}


static void Queue_test_growth_from_capacity_one(void **state) {
    (void)state;
    GenericQueue q;
    uint8_t   status = 0;
    QueueData d1 = {1}, d2 = {2};


    verify_queue_creation(&q, sizeof(QueueData), 1);

    assert_int_equal(GenericQueuePush(&q, &d1), 0);

    status = GenericQueuePush(&q, &d2);
    /* When q.m_maxObjCount == 1, resize should change to it to 3 */
    assert_int_equal(status, 0);
    assert_int_equal(q.m_maxObjCount, 3);

    GenericQueueDestroy(&q);
    return;
}


static void Queue_test_growth_while_wrapped(void **state) {
    (void)state;
    GenericQueue q;
    QueueData d1 = {1}, d2 = {2}, d3 = {3}, d4 = {4};
    QueueData d_out;

    /* Create with capacity 2 */
    verify_queue_creation(&q, sizeof(QueueData), 2);

    GenericQueuePush(&q, &d1);
    GenericQueuePush(&q, &d2); /* Full: [1, 2], head=0, tail=0 */
    GenericQueuePop(&q);       /* Pop 1: [_, 2], head=1, tail=0 */
    GenericQueuePush(&q, &d3); /* Full again: [3, 2], head=1, tail=1 (Wrapped) */

    /* Triggers GenericQueueGrow on wrapped data */
    GenericQueuePush(&q, &d4);

    /* Verify data was contiguous-ified correctly */
    GenericQueueFront(&q, &d_out);
    assert_int_equal(d_out.id, 2);
    GenericQueuePop(&q);

    GenericQueueFront(&q, &d_out);
    assert_int_equal(d_out.id, 3);

    GenericQueueDestroy(&q);
    return;
}


static void Queue_test_empty_edge_cases(void **state) {
    (void) state;
    GenericQueue q;
    QueueData d_out = {99}; /* Canary value */

    GenericQueueCreate(&q, sizeof(QueueData), 5);

    /* Popping empty queue should do nothing */
    GenericQueuePop(&q);
    assert_int_equal(GenericQueueEmpty(&q), 1);

    /* Front on empty queue should leave d_out unmodified */
    GenericQueueFront(&q, &d_out);
    assert_int_equal(d_out.id, 99);

    GenericQueueDestroy(&q);
    return;
}


static void Queue_test_fuzz_operations(void **state) {
    (void)state;
    static const uint64_t kGroundTruthBufMaxSize   = 4 * 1024 * 1024;
    static const uint64_t kMaximumRandomOperations = 4 * 1024 * 1024;
    FuzzPayload* groundTruthBuf  = malloc(sizeof(FuzzPayload) * kGroundTruthBufMaxSize);
    uint32_t     groundTruthCnt  = 0;
    uint32_t     groundTruthHead = 0;
    uint32_t     groundTruthTail = 0;
    FuzzOp       op              = FUZZ_STACK_OPERATION_MAX;
    FuzzPayload  tmpVal          = {};
    GenericQueue queue;


    srand(0);
    verify_queue_creation(&queue, sizeof(FuzzPayload), 10);
    for (uint32_t i = 0; i < kMaximumRandomOperations; ++i)
    {
        op = rand() % FUZZ_STACK_OPERATION_MAX;

        switch(op) {
            case FUZZ_QUEUE_OPERATION_ENQUEUE:
            tmpVal = (FuzzPayload){
                i,
                rand() % UINT32_MAX
            };
            groundTruthBuf[groundTruthTail % kGroundTruthBufMaxSize] = tmpVal;
            ++groundTruthTail;
            ++groundTruthCnt;

            assert_int_equal(GenericQueuePush(&queue, &tmpVal), 0);
            break;

            case FUZZ_QUEUE_OPERATION_DEQUEUE:
            if(groundTruthCnt > 0) {
                GenericQueuePop(&queue);
                ++groundTruthHead;
                --groundTruthCnt;
            }
            break;

            case FUZZ_QUEUE_OPERATION_FRONT:
            if (groundTruthCnt > 0) {
                assert_int_equal(GenericQueueFront(&queue, &tmpVal), 0);
                assert_int_equal(tmpVal.m_id,  groundTruthBuf[groundTruthHead % kGroundTruthBufMaxSize].m_id);
                assert_int_equal(tmpVal.m_val, groundTruthBuf[groundTruthHead % kGroundTruthBufMaxSize].m_val);
            }
            break;

            default:
            break;
        }


        assert_int_equal(GenericQueueSize(&queue), groundTruthCnt);
        assert_true(queue.m_maxObjCount >= queue.m_objCount);
        assert_true(queue.m_head < queue.m_maxObjCount);
        assert_true(queue.m_tail < queue.m_maxObjCount);
        assert_int_equal(GenericQueueEmpty(&queue), (groundTruthCnt == 0));
    }


    free(groundTruthBuf);
    GenericQueueDestroy(&queue);
    return;
}



int run_all_queue_tests() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(Queue_test_create),
        cmocka_unit_test(Queue_test_basic_operations),
        cmocka_unit_test(Queue_test_circular_wrap),
        cmocka_unit_test(Queue_test_growth_from_capacity_one),
        cmocka_unit_test(Queue_test_growth_while_wrapped),
        cmocka_unit_test(Queue_test_empty_edge_cases),
        cmocka_unit_test(Queue_test_fuzz_operations)
    };

    return cmocka_run_group_tests_name("Queue", tests, NULL, NULL);
}
