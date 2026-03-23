#include "VectorTest.h"
#include <cmocka.h>
#include <tree/C/vector.h>
#include <util2/C/random.h>
#include <stdlib.h>
#include <stdio.h>


/* Shadow state tracker for the fuzzer */
typedef struct {
    uint32_t expected_size;
    int64_t  expected_sum;
} FuzzState;


typedef enum {
    VECTOR_OP_PUSH_BACK,
    VECTOR_OP_POP_BACK,
    VECTOR_OP_INSERT,
    VECTOR_OP_ERASE,
    VECTOR_OP_RESIZE,
    VECTOR_OP_RESERVE,
    VECTOR_OP_SHRINK_TO_FIT,
    VECTOR_OP_CLEAR,
    VECTOR_OP_SET,
    VECTOR_OP_COUNT_MAX
} VectorOperation;

const char* vectorOpToString(VectorOperation op) {
    static const char* strBuffer[10] = {
        "VECTOR_OP_PUSH_BACK",
        "VECTOR_OP_POP_BACK",
        "VECTOR_OP_INSERT",
        "VECTOR_OP_ERASE",
        "VECTOR_OP_RESIZE",
        "VECTOR_OP_RESERVE",
        "VECTOR_OP_CLEAR",
        "VECTOR_OP_SHRINK_TO_FIT",
        "VECTOR_OP_SET",
        "VECTOR_OP_COUNT"
    };
    return strBuffer[(uint8_t)op];
}




static void test_create_and_destroy(void** state) {
	(void) state;
	GenericVector vec;


	GenericVectorCreate(&vec, sizeof(int32_t));
	assert_true(GenericVectorEmpty(&vec));
	assert_int_equal(GenericVectorSize(&vec), 0);
	assert_int_equal(GenericVectorCapacity(&vec), 0);

	GenericVectorDestroy(&vec);
	return;
}


static void test_create_with_capacity(void** state) {
	(void) state;
	GenericVector vec;

	bool err = GenericVectorCreateWithCapacity(&vec, sizeof(int32_t), 10);
	assert_false(err);

	assert_true(GenericVectorEmpty(&vec));
	assert_int_equal(GenericVectorSize(&vec), 0);
	assert_int_equal(GenericVectorCapacity(&vec), 10);
	assert_non_null(GenericVectorData(&vec));

	GenericVectorDestroy(&vec);
	return;
}


static void test_create_with_size_default_value_null(void** state) {
	(void) state;
	GenericVector vec;

    /* Test Path With DefaultValue==NULL -> Simple memset with reallocation */
	bool err = GenericVectorCreateWithSize(&vec, sizeof(int32_t), 10, NULL);
	assert_false(err);

	assert_false(GenericVectorEmpty(&vec));
	assert_int_equal(GenericVectorSize(&vec), 10);
	assert_int_equal(GenericVectorCapacity(&vec), 10);
	assert_non_null(GenericVectorData(&vec));

    for(uint32_t i = 0; i < GenericVectorSize(&vec); ++i) {
        assert_null(  *( (int32_t*)GenericVectorGet(&vec, i) )  );
    }

	GenericVectorDestroy(&vec);
	return;
}


static void test_create_with_size_default_value_nonnull(void** state) {
	(void) state;
	GenericVector vec;
    int32_t defaultVal = 5;

    /* Test Path With DefaultValue != NULL -> reallocation & memcpy */
	bool err = GenericVectorCreateWithSize(&vec, sizeof(int32_t), 10, &defaultVal);
	assert_false(err);

	assert_false(GenericVectorEmpty(&vec));
	assert_int_equal(GenericVectorSize(&vec), 10);
	assert_int_equal(GenericVectorCapacity(&vec), 10);
	assert_non_null(GenericVectorData(&vec));

    for(uint32_t i = 0; i < GenericVectorSize(&vec); ++i) {
        assert_int_equal(defaultVal, *( (int32_t*)GenericVectorGet(&vec, i) )  );
    }

	GenericVectorDestroy(&vec);
	return;
}


static void test_getters_and_setters(void** state) {
    (void)state;
	GenericVector vec, vec1;
    uint32_t vectorSize = 20;
    int32_t  valueToSet = 11;
    int32_t* valptr     = NULL;
    bool     err        = 0;

	GenericVectorCreate(&vec, sizeof(int32_t));
	assert_true(GenericVectorEmpty(&vec));
    assert_int_equal(0,    GenericVectorSize(&vec));
    assert_int_equal(0,    GenericVectorCapacity(&vec));
    assert_int_equal(NULL, GenericVectorGet(&vec, 0) ); /* m_objCount is 0 -> Get Should return NULL */


    err = GenericVectorResize(&vec, vectorSize);
    assert_false(err);
    assert_false(GenericVectorEmpty(&vec));
    assert_int_equal(vectorSize, GenericVectorSize(&vec));
    assert_true(GenericVectorSize(&vec) <= GenericVectorCapacity(&vec));


    /* Test Boundary Checked Set/Get */
    GenericVectorSet(&vec, 0, NULL); /* Should fail because of NULL valueptr */
    valptr = (int32_t*)GenericVectorGet(&vec, 0);
    assert_non_null(valptr);
    assert_int_equal(0, *valptr); /* Resize Sets Indices [0, m_objCount] to 0 */

    /* Test Boundary Checked Set/Get with value */
    GenericVectorSet(&vec, 0, &valueToSet); /* Should Succeed because of valueptr */
    valptr = (int32_t*)GenericVectorGet(&vec, 0);
    assert_non_null(valptr);
    assert_int_equal(valueToSet, *valptr);

    /* Test valid unchecked memory access with value */
    GenericVectorSetAt(&vec, vectorSize - 1, &valueToSet);
    valptr = (int32_t*)GenericVectorAt(&vec, vectorSize - 1);
    assert_non_null(valptr);
    assert_int_equal(valueToSet, *valptr);

    assert_int_equal(valptr,     GenericVectorBack(&vec));
    assert_int_equal(valptr + 1, GenericVectorEnd(&vec));
    return;
}


static void test_push_back_and_access(void** state) {
	(void) state;
	GenericVector vec;
    int32_t* front = NULL;
    int32_t* back  = NULL;
    int32_t* val   = NULL;
    bool     err   = 0;

	GenericVectorCreate(&vec, sizeof(int32_t));
	for (int32_t i = 0; i < 5; ++i) {
		err = GenericVectorPushBack(&vec, &i);
        assert_false(err);
	}

	assert_false(GenericVectorEmpty(&vec));
	assert_int_equal(GenericVectorSize(&vec), 5);
    assert_true(GenericVectorCapacity(&vec) >= GenericVectorSize(&vec));


	front = (int32_t*)GenericVectorFront(&vec);
	back  = (int32_t*)GenericVectorBack(&vec);
	assert_int_equal(*front, 0);
	assert_int_equal(*back, 4);

	for (int32_t i = 0; i < 5; ++i) {
        val = (int32_t*)GenericVectorGet(&vec, i);
		assert_non_null(val);
		assert_int_equal(*val, i);
	}


	GenericVectorDestroy(&vec);
    return;
}


static void test_pop_back(void** state) {
	(void) state;
    int32_t  val1 = 10;
    int32_t  val2 = 20;
    int32_t* back = NULL;
    bool     err  = 0;
	GenericVector vec;

	
    GenericVectorCreate(&vec, sizeof(int32_t));
	err = GenericVectorPushBack(&vec, &val1);
    assert_false(err);
	err = GenericVectorPushBack(&vec, &val2);
    assert_false(err);

	assert_int_equal(GenericVectorSize(&vec), 2);
	GenericVectorPopBack(&vec);
	assert_int_equal(GenericVectorSize(&vec), 1);

	back = (int32_t*)GenericVectorBack(&vec);
	assert_int_equal(*back, 10);
	GenericVectorPopBack(&vec);
	assert_true(GenericVectorEmpty(&vec));


	GenericVectorPopBack(&vec); /* Empty PopBack */
	assert_true(GenericVectorEmpty(&vec));

	GenericVectorDestroy(&vec);
    return;
}


static void test_insert(void** state) {
	(void) state;
	GenericVector vec;
	int32_t  val0[] = {1, 2, 3};
	int32_t  val1[] = {9, 9};
	int32_t  expected0[] = {1, 9, 9, 2, 3};
    int32_t  val2[] = {5, 4};
    int32_t  expected1[] = {1, 9, 9, 2, 3, 5, 4};
    int32_t* val = NULL;
    bool     err = 0;


    /* Insert in empty vector */
	GenericVectorCreate(&vec, sizeof(int32_t));
	err = GenericVectorInsert(&vec, 0, 3, val0);
    assert_false(err);
	assert_int_equal(GenericVectorSize(&vec), 3);

    /* Insert in the middle (index 1) */
	err = GenericVectorInsert(&vec, 
        1, 
        2, 
        val1
    );
    assert_false(err);
	assert_int_equal(GenericVectorSize(&vec), 5);
	for (int32_t i = 0; i < ( sizeof(expected0) / sizeof(expected0[0]) ); ++i) {
		val = (int32_t*)GenericVectorAt(&vec, i);
        assert_non_null(val);
		assert_int_equal(*val, expected0[i]);
	}

    /* Insert in the end */
	err = GenericVectorInsert(&vec, 
        ( GenericVectorEnd(&vec) - GenericVectorBegin(&vec) ) / sizeof(int32_t),
        2,
        val2
    );
    assert_false(err);
	for (int32_t i = 0; i < ( sizeof(expected1) / sizeof(expected1[0]) ); ++i) {
		val = (int32_t*)GenericVectorAt(&vec, i);
        assert_non_null(val);
		assert_int_equal(*val, expected1[i]);
	}


	GenericVectorDestroy(&vec);
    return;
}


static void test_erase(void** state) {
	(void) state;
	int32_t  valA[]     = { 10, 20, 30, 40, 50 };
	int32_t  expected[] = { valA[0], valA[3], valA[4] };
    int32_t* val   = NULL;
    int32_t* front = NULL;
    bool     err   = 0;

	GenericVector vec;


	GenericVectorCreate(&vec, sizeof(int32_t));
	err = GenericVectorInsert(&vec, 0, ( sizeof(valA) / sizeof(valA[0]) ), valA);
    assert_false(err);


	/* Erase middle elements (20, 30) */
	GenericVectorErase(&vec, 1, 2);
	assert_int_equal(GenericVectorSize(&vec), 3);
	for (uint32_t i = 0; i < ( sizeof(expected) / sizeof(expected[0]) ); ++i) {
		val = (int32_t*)GenericVectorGet(&vec, i);
		assert_int_equal(*val, expected[i]);
	}

    /* Attempts to erase beyond size, clamps to remaining size */
	GenericVectorErase(&vec, 1, 10);
	assert_int_equal(GenericVectorSize(&vec), 1);
	front = (int32_t*)GenericVectorFront(&vec);
	assert_int_equal(*front, valA[0]);

    /* Make sure out-of-bounds access actually returns nullptr */
	for (uint32_t i = 0; i < ( sizeof(expected) / sizeof(expected[0]) ); ++i) {
        val = GenericVectorGet(&vec, i);
        if(i == 0) {
            assert_int_equal(*val, expected[0]);
        } else {
            assert_null(val);
        }
    }


	GenericVectorDestroy(&vec);
    return;
}


static void test_swap(void** state) {
	(void) state;
	int32_t  valA   = 100;
	int32_t  valB[] = {1, 2, 3};
    int32_t* frontA = NULL;
    int32_t* frontB = NULL;
    bool     err    = 0;
	GenericVector vecA, vecB;


	GenericVectorCreate(&vecA, sizeof(int32_t));
	GenericVectorCreate(&vecB, sizeof(int32_t));
	err = GenericVectorPushBack(&vecA, &valA);
    assert_false(err);
	err = GenericVectorInsert(&vecB, 0, 3, valB);
    assert_false(err);

    /* Test Successful swap */
	GenericVectorSwap(&vecA, &vecB);
	assert_int_equal(GenericVectorSize(&vecA), 3);
	assert_int_equal(GenericVectorSize(&vecB), 1);

	frontA = (int32_t*)GenericVectorFront(&vecA);
	assert_int_equal(*frontA, 1);
	frontB = (int32_t*)GenericVectorFront(&vecB);
	assert_int_equal(*frontB, 100);

	GenericVectorDestroy(&vecA);
	GenericVectorDestroy(&vecB);
    return;
}


static void test_reserve_and_shrink(void** state) {
	(void)state;
    const uint32_t vectorCapacity      = 100;
    const uint32_t vectorCapacityBytes = vectorCapacity * sizeof(int32_t);
    int32_t valToPush = 42;
    bool    err       = 0;
	GenericVector vec;


	GenericVectorCreate(&vec, sizeof(int32_t));
	err = GenericVectorReserve(&vec, 100);
    assert_false(err);
	assert_int_equal(GenericVectorCapacity(&vec), vectorCapacity);
	assert_int_equal(GenericVectorCapacityBytes(&vec), vectorCapacityBytes);
	assert_int_equal(GenericVectorSize(&vec), 0);
	assert_int_equal(GenericVectorSizeBytes(&vec), 0);


    err = GenericVectorPushBack(&vec, &valToPush);
    assert_false(err);
	err = GenericVectorShrinkToFit(&vec);
    assert_false(err);
    
	/* Capacity drops to size, which is 1 */
	assert_int_equal(GenericVectorCapacity(&vec), 1);
	assert_int_equal(GenericVectorSize(&vec),     1);

	GenericVectorDestroy(&vec);
    return;
}


static void test_clear_and_resize(void** state) {
	(void) state;
    bool          err      = 0;
    int32_t*      valptr   = NULL;
	int32_t       vals[]   = {1, 2, 3};
    uint32_t      valsSize = ( sizeof(vals) / sizeof(vals[0]) );
    uint32_t      vecSize  = 5;
	GenericVector vec;
    
	
    
    GenericVectorCreate(&vec, sizeof(int32_t));
	err = GenericVectorInsert(&vec, 0, valsSize, vals);
	assert_false(err);
    GenericVectorClear(&vec);


	assert_true(GenericVectorEmpty(&vec));
	assert_int_equal(GenericVectorSize(&vec), 0);
	/* Capacity should remain unchanged */
	assert_true(GenericVectorCapacity(&vec) >= valsSize);

	/* Resize larger */
	err = GenericVectorResize(&vec, vecSize);
    assert_false(err);
	assert_int_equal(GenericVectorSize(&vec), 5);

	/* Check that memory is zeroed out by GenericVectorClearRange logic */
	for (uint32_t i = 0; i < vecSize; ++i) {
		valptr = (int32_t*)GenericVectorGet(&vec, i);
		assert_int_equal(*valptr, 0);
	}

	/* Resize smaller */
    GenericVectorSetAt(&vec, 0, &vals[0]);
    GenericVectorSetAt(&vec, 1, &vals[1]);
	err = GenericVectorResize(&vec, 2);
    assert_false(err);
	assert_int_equal(GenericVectorSize(&vec), 2);
	/* Check (again) that memory is zeroed out by GenericVectorClearRange logic */
	for (uint32_t i = 0; i < vecSize; ++i) {
		valptr = (int32_t*)GenericVectorAt(&vec, i);
        if(i < 2) {
            assert_int_equal(*valptr, vals[i]);    
        } else {
            assert_int_equal(*valptr, 0);
        }
	}


	GenericVectorDestroy(&vec);
    return;
}



static void VerifyInnerVectorState(
    GenericVector* vec, 
    FuzzState*     state
) {
    /* Verify Data Members, Iterators & sum of data members. */
    assert_int_equal(GenericVectorSize(vec), state->expected_size);
    assert_true(GenericVectorCapacity(vec) >= state->expected_size);
    

    if (state->expected_size > 0) {
        assert_non_null(GenericVectorData(vec));
        assert_non_null(GenericVectorFront(vec));
        assert_non_null(GenericVectorBack(vec));
        
        int64_t  sum = 0;
        int32_t* val = NULL;
        for (uint32_t i = 0; i < state->expected_size; ++i) {
            val = (int32_t*)GenericVectorGet(vec, i);
            assert_non_null(val);
            sum += *val;
        }
        assert_int_equal(sum, state->expected_sum);
    } else {
        assert_true(GenericVectorEmpty(vec));
        assert_int_equal(state->expected_sum, 0);
    }


    return;
}

static void test_fuzz_generic_vector(void **state_ptr) {
    (void)state_ptr;
    const uint32_t num_operations = 1000000;
    const uint32_t randSeed       = 42;

    GenericVector vec;
    FuzzState     state = {0, 0};
    int32_t*      valptr = NULL;
    int32_t       val    = 0;

    GenericVectorCreate(&vec, sizeof(int32_t));
    for (int32_t i = 0; i < num_operations; ++i) {
        // printf("\r\r\r\r\r\r");
        VectorOperation op = (VectorOperation)(random8u() % VECTOR_OP_COUNT_MAX);
        

        switch (op) 
        {
            case VECTOR_OP_PUSH_BACK:
            int32_t val = random32i();
            if (!GenericVectorPushBack(&vec, &val)) {
                ++state.expected_size;
                state.expected_sum += val;
            }
            break;

            case VECTOR_OP_POP_BACK:
            if (state.expected_size > 0) {
                int32_t* back_val = (int32_t*)GenericVectorBack(&vec);
                state.expected_sum -= *back_val;
                GenericVectorPopBack(&vec);
                --state.expected_size;
            }
            break;

            case VECTOR_OP_INSERT:
            uint32_t randInsertIdx = (state.expected_size == 0) ? 0 : (random32u() % (state.expected_size + 1));
            uint32_t valCount      = (random8u() % 100) + 1;
            int32_t* valueBuf      = malloc(valCount * sizeof(int32_t));
            int64_t  valToInsertSum = 0;

            if (valueBuf) 
            {
                for(uint32_t j = 0; j < valCount; ++j) {
                    valueBuf[j] = random32i();
                    valToInsertSum += valueBuf[j];
                }
                if (!GenericVectorInsert(&vec, randInsertIdx, valCount, valueBuf)) {
                    state.expected_size += valCount;
                    state.expected_sum += valToInsertSum;
                }
                free(valueBuf);
            }
            break;
            
            case VECTOR_OP_ERASE:
            if (state.expected_size > 0) 
            {
                uint32_t randIdx  = random32u() % state.expected_size;
                uint32_t valCount = (random32u() % (state.expected_size - randIdx)) + 1;
                int32_t* currVal       = NULL;
                int64_t  valToEraseSum = 0;

                for (uint32_t j = 0; j < valCount; j++) {
                    currVal = (int32_t*)GenericVectorGet(&vec, randIdx + j);
                    valToEraseSum += *currVal;
                }
                
                GenericVectorErase(&vec, randIdx, valCount);
                state.expected_size -= valCount;
                state.expected_sum -= valToEraseSum;
            }
            break;

            case VECTOR_OP_RESIZE:
            uint32_t updatedSize = random16u();
            if (!GenericVectorResize(&vec, updatedSize)) 
            {
                if (updatedSize < state.expected_size) {
                    /* Vector shrank: recalculate sum of remaining elements */
                    int64_t  new_sum = 0;
                    int32_t* currVal = NULL;
                    for (uint32_t j = 0; j < updatedSize; j++) {
                        currVal = (int32_t*)GenericVectorGet(&vec, j);
                        new_sum += *currVal;
                    }
                    state.expected_sum = new_sum;
                }
                /* Vector Grew/Stayed the same: new elements are zeroed out by GenericVectorResize */
                /* expected_sum remains exactly the same */
                state.expected_size = updatedSize;
            }
            break;

            case VECTOR_OP_RESERVE:
            uint32_t new_cap = GenericVectorCapacity(&vec) + random8u();
            if(GenericVectorReserve(&vec, new_cap)) {
                printf("GenericVectorReserve Failed(&vec, %u) Failed\n", new_cap);
            }
            break;

            case VECTOR_OP_SHRINK_TO_FIT:
            uint32_t currCap  = GenericVectorCapacity(&vec);
            uint32_t currSize = GenericVectorSize(&vec);
            if(currSize > 0) {
                if(GenericVectorShrinkToFit(&vec)) {
                    printf("GenericVectorShrinkToFit(&vec) Failed To Shrink From Capacity %u To Size %u\n", currCap, currSize);
                }
            }
            break;

            case VECTOR_OP_CLEAR:
            GenericVectorClear(&vec);
            state.expected_size = 0;
            state.expected_sum = 0;
            break;

            case VECTOR_OP_SET:
            if (state.expected_size > 0) {
                uint32_t idxToSet   = random32u() % state.expected_size;
                int32_t  valueToSet = random32i();
                int32_t  valueOld   = *(int32_t*)GenericVectorGet(&vec, idxToSet);

                GenericVectorSet(&vec, idxToSet, &valueToSet);
                state.expected_sum = state.expected_sum - valueOld + valueToSet;
            }
            break;
            
            default:
            break;
        }


        // printf("[%07u] Operation %s\n", i, vectorOpToString(op));
        // printf("%06u", i);
        VerifyInnerVectorState(&vec, &state); /* Check state after every operation */
    }


    GenericVectorDestroy(&vec);
    return;
}


int run_all_vector_tests(void)
{
	const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_create_and_destroy),
        cmocka_unit_test(test_create_with_capacity),
        cmocka_unit_test(test_create_with_size_default_value_null),
        cmocka_unit_test(test_create_with_size_default_value_nonnull),
        cmocka_unit_test(test_getters_and_setters),
        cmocka_unit_test(test_push_back_and_access),
        cmocka_unit_test(test_pop_back),
        cmocka_unit_test(test_insert),
        cmocka_unit_test(test_erase),
        cmocka_unit_test(test_swap),
        cmocka_unit_test(test_reserve_and_shrink),
        cmocka_unit_test(test_clear_and_resize),
        cmocka_unit_test(test_fuzz_generic_vector)
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}