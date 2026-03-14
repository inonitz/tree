#include "AVLTreeCTest.h"
#include <util2/C/aligned_malloc.h>
#include <util2/C/ifcrash2.h>
#include <util2/C/random.h>
#include <tree/C/avl_tree.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <cmocka.h>


static FILE*    g_reportFile           = NULL;
static char*    g_massiveBuffer        = NULL;
static uint64_t g_massiveBufferCurrIdx = 0;


#define GK_STEST_TOTAL_OPS     (1 * 1000 * 1000)
#define GK_STEST_VAL_DIST_MIN  1
#define GK_STEST_VAL_DIST_MAX  100000
#define GK_MASSIVE_BUFFER_SIZE (128ull * 1024 * 1024)
#define MANUAL_TREE_SIZE       100
#define write_to_test_buffer(formatstr, ...)                                                               \
	g_massiveBufferCurrIdx += sprintf(&g_massiveBuffer[g_massiveBufferCurrIdx], formatstr, ##__VA_ARGS__); \
	ifcrashfmt(g_massiveBufferCurrIdx >= GK_MASSIVE_BUFFER_SIZE, "Report Buffer Index Reached %" PRIu64 "/%" PRIu64 " Bytes\n", g_massiveBufferCurrIdx, (uint64_t) GK_MASSIVE_BUFFER_SIZE);


static int8_t uint32_cmp(const void* a, const void* b) {
	uint32_t valA = *(const uint32_t*) a;
	uint32_t valB = *(const uint32_t*) b;

	return valA == valB ? 0 : (valA < valB) ? -1 : 1;
}


static void printTreeDataMember(
    binaryTreePrintCtx* ctx,
    const void* value
) {
	const char* formatString = "[%u]";
	uint32_t    actualVal 	 = *((uint32_t*)value);

    if(ctx->m_buf == NULL || (ctx->m_bufSize != 0 && ctx->m_bufOffset >= ctx->m_bufSize)) {
        return;
    }

	if(ctx->m_bufSize == 0) {
		fprintf((FILE*)ctx->m_buf, formatString, actualVal);
		return;
	}


	int bytesWritten = snprintf(
		(char*)ctx->m_buf + ctx->m_bufOffset, 
		ctx->m_bufSize - ctx->m_bufOffset, 
		formatString, 
		actualVal
	);
	ctx->m_bufOffset += (bytesWritten > 0) ? bytesWritten : 0;
	return;
}


static int setup_c_report_buffer(void** state) {
	(void) state;
	g_massiveBuffer = (char*) util2_aligned_malloc(GK_MASSIVE_BUFFER_SIZE, CACHE_LINE_BYTES);
	g_reportFile    = fopen("avl_test_report.txt", "w");

	assert_non_null(g_reportFile);
	assert_non_null(g_massiveBuffer);

	setvbuf(g_reportFile, NULL, _IONBF, 0);
	g_massiveBuffer[GK_MASSIVE_BUFFER_SIZE - 1] = '\0';
	return 0;
}

static int teardown_c_report_buffer(void** state) {
	(void) state;
	write_to_test_buffer("g_massiveBuffer Consumed %" PRIu64 "/%" PRIu64 " Bytes for %u Operations\n", g_massiveBufferCurrIdx, (uint64_t) GK_MASSIVE_BUFFER_SIZE, GK_STEST_TOTAL_OPS);
	if (g_reportFile) {
		fprintf(g_reportFile, "%s", g_massiveBuffer);
		fclose(g_reportFile);
	}
	util2_aligned_free(g_massiveBuffer);
	return 0;
}


/* --- Dynamic Array Implementation to replace std::vector --- */
typedef struct {
	uint32_t* data;
	size_t    size;
	size_t    capacity;
} UIntArray;

static void array_init(UIntArray* arr) {
	arr->size     = 0;
	arr->capacity = 1024;
	arr->data     = (uint32_t*) malloc(arr->capacity * sizeof(uint32_t));
	ifcrash(arr->data == NULL);
}

static void array_push_back(UIntArray* arr, uint32_t val) {
	if (arr->size >= arr->capacity) {
		arr->capacity *= 2;
		arr->data = (uint32_t*) realloc(arr->data, arr->capacity * sizeof(uint32_t));
		ifcrash(arr->data == NULL);
	}
	arr->data[arr->size++] = val;
}

static void array_erase(UIntArray* arr, size_t index) {
	if (index < arr->size) {
		memmove(&arr->data[index], &arr->data[index + 1], (arr->size - index - 1) * sizeof(uint32_t));
		arr->size--;
	}
}

static void array_destroy(UIntArray* arr) {
	if (arr->data) {
		free(arr->data);
		arr->data = NULL;
	}
	arr->size     = 0;
	arr->capacity = 0;
}
/* --------------------------------------------------------- */




static void BasicInsertionAndSearch(void** state) {
	(void)state;
	AVLTree tree;
	uint32_t vals[] = {50, 30, 70, 100};
	
	AVLTreeCreate(&tree, uint32_cmp, sizeof(uint32_t));
	assert_true(AVLTreeEmpty(&tree));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));
	assert_int_equal(AVLTreeSize(&tree), 3);
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &vals[3]));

	AVLTreeDestroy(&tree);
}


static void SingleRotationsLeftLeft(void** state) {
	(void) state;
	AVLTree tree;
	AVLTreeCreate(&tree, uint32_cmp, sizeof(uint32_t));

	uint32_t vals[] = {30, 20, 10, 40};
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));

	assert_true(AVLTreeIsBalanced(&tree));
	assert_int_equal(AVLTreeSize(&tree), 3);

	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[2]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &vals[3]));

	AVLTreeDestroy(&tree);
	return;
}


static void SingleRotationsRightRight(void** state) {
	(void) state;
	AVLTree tree;
	AVLTreeCreate(&tree, uint32_cmp, sizeof(uint32_t));

	uint32_t vals[] = {10, 20, 30, 40};
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));

	assert_true(AVLTreeIsBalanced(&tree));
	assert_int_equal(AVLTreeSize(&tree), 3);

	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[2]));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &vals[3]));

	AVLTreeDestroy(&tree);
}


static void DoubleRotationsLeftRight(void** state) {
	(void) state;
	AVLTree  tree;
	uint32_t vals[] = {30, 10, 20};

	AVLTreeCreate(&tree, uint32_cmp, sizeof(uint32_t));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));
	assert_true(AVLTreeIsBalanced(&tree));

	AVLTreeDestroy(&tree);
	return;
}


static void DoubleRotationsRightLeft(void** state) {
	(void) state;
	AVLTree  tree;
	uint32_t vals[] = {10, 30, 20};

	AVLTreeCreate(&tree, uint32_cmp, sizeof(uint32_t));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));

	assert_true(AVLTreeIsBalanced(&tree));

	AVLTreeDestroy(&tree);
}


static void DeletionRebalancing(void** state) {
	(void) state;
	AVLTree tree;
	uint32_t vals[]   = {50, 25, 75, 10, 35, 60, 90};
	size_t   num_vals = sizeof(vals) / sizeof(vals[0]);
	uint32_t val10 = 10;
	uint32_t val25 = 25;
    
	
    AVLTreeCreate(&tree, uint32_cmp, sizeof(uint32_t));
	for (size_t i = 0; i < num_vals; ++i) {
		assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[i]));
	}


	/* Remove leaf */
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeRemove2(&tree, &val10));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &val10));
	assert_true(AVLTreeIsBalanced(&tree));

	/* Remove node with two children */
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeRemove2(&tree, &val25));
	assert_true(AVLTreeIsBalanced(&tree));

	AVLTreeDestroy(&tree);
    return;
}


static void ManualVerificationInsertDeleteTest(void** state) {
	(void)state;
	AVLTree  test;
	uint32_t data[MANUAL_TREE_SIZE];
	uint32_t randIdx      = 0;
	uint32_t randValToDel = 0;
    bool     opStatus = false;
	int      c        = 0;
    

	AVLTreeCreate(&test, uint32_cmp, sizeof(uint32_t));
	for (size_t i = 0; i < MANUAL_TREE_SIZE; ++i) {
		data[i] = (uint32_t)(random32u() % 101); /* 0 to 100 */
	}


	for (size_t i = 0; i < MANUAL_TREE_SIZE; ++i) {
		uint32_t val = data[i];
		opStatus     = AVLTreeInsert(&test, &val);

		write_to_test_buffer("--- 2D Tree Visualization (Rotate head left) ---\n");
		write_to_test_buffer("[c=%3u] Insertion Of %3u -> %s\n", c, val, opStatus == BINARY_TREE_OP_SUCCESS ? "SUCCESS" : "FAILURE");

		if (g_reportFile)
			AVLTreePrint(&test, g_reportFile, 0, 0, printTreeDataMember);

		write_to_test_buffer("Post Insertion AVL Tree Valid(?) %u\n", AVLTreeIsBalanced(&test));
		write_to_test_buffer("-----------------------------------------------\n");
		++c;
	}


	write_to_test_buffer("-----------------------------------------------\n");
	write_to_test_buffer("--------------------------------------------AAA\n");
	write_to_test_buffer("-----------------------------------------------\n");
	fflush(g_reportFile);


	for (size_t i = 1; i < MANUAL_TREE_SIZE; ++i) 
	{
		randIdx      = (uint32_t) (random32u() % MANUAL_TREE_SIZE);
		randValToDel = data[randIdx];

		opStatus = AVLTreeRemove2(&test, &randValToDel);

		write_to_test_buffer("--- 2D Tree Visualization (Rotate head left) ---\n");
		write_to_test_buffer("[c=%3u] Deletion Of %3u -> %s\n", c, randValToDel, opStatus ? "SUCCESS" : "FAILURE");

		if (g_reportFile)
			AVLTreePrint(&test, g_reportFile, 0, 0, printTreeDataMember);

		write_to_test_buffer("Post Deletion AVL Tree Valid(?) %u\n", AVLTreeIsBalanced(&test));
		write_to_test_buffer("-----------------------------------------------\n");
		++c;
	}

	AVLTreeDestroy(&test);
}


static void StochasticStressTest(void** state) {
	(void) state;
	AVLTree testTree;
	AVLTreeCreate(&testTree, uint32_cmp, sizeof(uint32_t));

	UIntArray treeValueSet;
	array_init(&treeValueSet);


	CTestOperationType op          = CTEST_AVL_OPER_MAX_OP;
	uint32_t           val         = 0;
	uint32_t           tmpValue    = 0;
	uint32_t           tmpValueIdx = 0;
	bool               status      = false;

	uint32_t insertion[2]     = {0, 0};
	uint32_t deletion[2]      = {0, 0};
	uint32_t searchType[2]    = {0, 0};
	uint32_t searchRandVal[2] = {0, 0};
	uint32_t searchInSet[2]   = {0, 0};

	uint32_t* searchRandomValueOp        = &searchType[0];
	uint32_t* searchExistingValueOp      = &searchType[1];
	uint32_t* searchRandomValueSuccess   = &searchRandVal[0];
	uint32_t* searchRandomValueFailure   = &searchRandVal[1];
	uint32_t* searchExistingValueSuccess = &searchInSet[0];
	uint32_t* searchExistingValueFailure = &searchInSet[1];


	for (uint32_t i = 0; i < GK_STEST_TOTAL_OPS; ++i) {
		printf("\r\r\r\r\r\r");

		val = (random32u() % (GK_STEST_VAL_DIST_MAX - GK_STEST_VAL_DIST_MIN + 1)) + GK_STEST_VAL_DIST_MIN;
		op  = (CTestOperationType) (random32u() % CTEST_AVL_OPER_MAX_OP);

		switch (op) {
		case CTEST_AVL_OPER_INSERT_OP:
			if (AVLTreeSearch(&testTree, &val) == true) {
				continue;
			}
			status = AVLTreeInsert(&testTree, &val);
			array_push_back(&treeValueSet, val);
			++insertion[status ? 1 : 0];
			write_to_test_buffer("%07u: i %06u (%u)\n", i, val, status);
			break;

		case CTEST_AVL_OPER_DELETE_OP:
			if (treeValueSet.size == 0) {
				continue;
			}
			tmpValueIdx = random32u() % treeValueSet.size;
			tmpValue    = treeValueSet.data[tmpValueIdx];
			status      = AVLTreeRemove(&testTree, &tmpValue);
			array_erase(&treeValueSet, tmpValueIdx);
			++deletion[status ? 1 : 0];
			write_to_test_buffer("%07u: d %06u (%u)\n", i, tmpValue, status);
			break;

		case CTEST_AVL_OPER_SEARCH_RAND_OP:
			if (treeValueSet.size == 0) {
				continue;
			}
			tmpValue = val;
			status   = AVLTreeSearch(&testTree, &tmpValue);
			(*searchRandomValueOp)++;
			if (status)
				(*searchRandomValueSuccess)++;
			else
				(*searchRandomValueFailure)++;
			write_to_test_buffer("%07u: rs %06u (%u)\n", i, tmpValue, status);
			break;

		case CTEST_AVL_OPER_SEARCH_SET_OP:
			if (treeValueSet.size == 0) {
				continue;
			}
			tmpValueIdx = random32u() % treeValueSet.size;
			tmpValue    = treeValueSet.data[tmpValueIdx];
			status      = AVLTreeSearch(&testTree, &tmpValue);
			(*searchExistingValueOp)++;
			if (status)
				(*searchExistingValueSuccess)++;
			else
				(*searchExistingValueFailure)++;
			write_to_test_buffer("%07u: ss %06u (%u)\n", i, tmpValue, status);
			break;

		case CTEST_AVL_OPER_MAX_OP:
		default:
		break;
		}


		/* Integrity check every 10 ops */
		if (i % 10 == 0) {
			assert_true(AVLTreeIsBalanced(&testTree));
			assert_true(AVLTreeIsValidBST(&testTree));
			assert_int_equal(AVLTreeSize(&testTree), treeValueSet.size);
		}
		printf("%06u", i);
	}
	printf("\n");

	write_to_test_buffer("\n[==========] Stochastic Stress Diagnostics\n");
	write_to_test_buffer("             Insertions              (Success, Failure): %06u %06u\n", insertion[1], insertion[0]);
	write_to_test_buffer("             Deletions               (Success, Failure): %06u %06u\n", deletion[1], deletion[0]);
	write_to_test_buffer("             Searches                (Random, Existing): %06u %06u\n", *searchRandomValueOp, *searchExistingValueOp);
	write_to_test_buffer("             Random   Value Searches (Success, Failure): %06u %06u\n", *searchRandomValueSuccess, *searchRandomValueFailure);
	write_to_test_buffer("             Existing Value Searches (Success, Failure): %06u %06u\n", *searchExistingValueSuccess, *searchExistingValueFailure);
	write_to_test_buffer("             Final Size : %" PRIu64 "\n", (uint64_t) AVLTreeSize(&testTree));
	write_to_test_buffer("             Tree Height: %u\n", AVLTreeHeight(&testTree));

	array_destroy(&treeValueSet);
	AVLTreeDestroy(&testTree);
	return;
}


int run_all_c_avl_tree_tests() {
	const struct CMUnitTest tests[] = {
	    cmocka_unit_test(BasicInsertionAndSearch),
	    cmocka_unit_test(SingleRotationsLeftLeft),
	    cmocka_unit_test(SingleRotationsRightRight),
	    cmocka_unit_test(DoubleRotationsLeftRight),
	    cmocka_unit_test(DoubleRotationsRightLeft),
	    cmocka_unit_test(DeletionRebalancing),
	    cmocka_unit_test(ManualVerificationInsertDeleteTest),
	    cmocka_unit_test(StochasticStressTest)
	};

	return cmocka_run_group_tests(tests, setup_c_report_buffer, teardown_c_report_buffer);
}