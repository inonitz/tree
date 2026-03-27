#include "AVLTreeCTest.h"
#include <util2/C/aligned_malloc.h>
#include <util2/C/debugbreak.h>
#include <util2/C/ifcrash2.h>
#include <util2/C/random.h>
#include <tree/C/avl_tree.h>
#include <cmocka.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


#define GK_STEST_TOTAL_OPS     (1 * 1000 * 1000)
#define GK_STEST_VAL_DIST_MIN  1
#define GK_STEST_VAL_DIST_MAX  2000000
#define GK_MASSIVE_BUFFER_SIZE (128ull * 1024 * 1024)
#define MANUAL_TREE_SIZE       100


/* Global Test Logging State */
static FILE*    g_reportFile           = NULL;
static char*    g_massiveBuffer        = NULL;
static uint64_t g_massiveBufferCurrIdx = 0;


static void   log_test(uint8_t directlyToFile, const char* formatstr, ...);
static int8_t uint32_cmp(const void* a, const void* b);
static void   printTreeDataMember(
    binaryTreePrintCtx* ctx,
    const void* value
);
static int setup_c_report_buffer(void** state);
static int teardown_c_report_buffer(void** state);




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




static void AVLTreeBasicInsertionAndSearch(void** state) {
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


static void AVLTreeSingleRotationsLeftLeft(void** state) {
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


static void AVLTreeSingleRotationsRightRight(void** state) {
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


static void AVLTreeDoubleRotationsLeftRight(void** state) {
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


static void AVLTreeDoubleRotationsRightLeft(void** state) {
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


static void AVLTreeDeletionRebalancing(void** state) {
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
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeRemove(&tree, &val10));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &val10));
	assert_true(AVLTreeIsBalanced(&tree));

	/* Remove node with two children */
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeRemove(&tree, &val25));
	assert_true(AVLTreeIsBalanced(&tree));

	AVLTreeDestroy(&tree);
    return;
}


static void AVLTreeManualVerificationInsertDeleteTest(void** state) {
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

		log_test(0, "--- 2D Tree Visualization (Rotate head left) ---\n");
		log_test(0, "[c=%3u] Insertion Of %3u -> %s\n", c, val, opStatus == BINARY_TREE_OP_SUCCESS ? "SUCCESS" : "FAILURE");

		if (g_reportFile)
			AVLTreePrint(&test, g_reportFile, 0, 0, printTreeDataMember);

		log_test(0, "Post Insertion AVL Tree Valid(?) %u\n", AVLTreeIsBalanced(&test));
		log_test(0, "-----------------------------------------------\n");
		++c;
	}


	log_test(0, "-------------------------------------------------\n");
	log_test(0, "  ----- ManualInsertionDeletionTestMidway -----  \n");
	log_test(0, "-------------------------------------------------\n");
	fflush(g_reportFile);


	for (size_t i = 1; i < MANUAL_TREE_SIZE; ++i)
	{
		randIdx      = (uint32_t) (random32u() % MANUAL_TREE_SIZE);
		randValToDel = data[randIdx];

		opStatus = AVLTreeRemove(&test, &randValToDel);

		log_test(0, "--- 2D Tree Visualization (Rotate head left) ---\n");
		log_test(0, "[c=%3u] Deletion Of %3u -> %s\n", c, randValToDel, opStatus ? "SUCCESS" : "FAILURE");

		if (g_reportFile)
			AVLTreePrint(&test, g_reportFile, 0, 0, printTreeDataMember);

		log_test(0, "Post Deletion AVL Tree Valid(?) %u\n", AVLTreeIsBalanced(&test));
		log_test(0, "-----------------------------------------------\n");
		++c;
	}

	log_test(0, "-------------------------------------------------\n");
	log_test(0, "-------- ManualInsertionDeletionTestEnd --------\n");
	log_test(0, "-------------------------------------------------\n");
	AVLTreeDestroy(&test);
	return;
}


static void AVLTreeRandomOperationsFuzzStressTest(void** state) {
	(void) state;
	AVLTree testTree;
	AVLTreeCreate(&testTree, uint32_cmp, sizeof(uint32_t));

	UIntArray treeValueSet;
	array_init(&treeValueSet);

	uint32_t randomValueBufSize = GK_STEST_TOTAL_OPS;
	uint32_t* randomValueBuf    = malloc(sizeof(uint32_t) * randomValueBufSize);

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


	f64 tmp = 0;
	for(u32 i = 0; i < randomValueBufSize; ++i) {
		tmp = random64f();
		tmp = tmp * (GK_STEST_VAL_DIST_MAX - GK_STEST_VAL_DIST_MIN) + GK_STEST_VAL_DIST_MIN;
		randomValueBuf[i] = (u32)tmp;
	}


	for (uint32_t i = 0; i < GK_STEST_TOTAL_OPS; ++i) {
		// printf("\r\r\r\r\r\r");
		val = randomValueBuf[i];
		op  = (CTestOperationType) (random32u() % CTEST_AVL_OPER_MAX_OP);

		switch (op)
		{
		case CTEST_AVL_OPER_INSERT_OP:
			if (AVLTreeSearch(&testTree, &val) == BINARY_TREE_BOOL_FALSE) {
				status = AVLTreeInsert(&testTree, &val);
				array_push_back(&treeValueSet, val);
				++insertion[status == BINARY_TREE_OP_SUCCESS ? 1 : 0];
				log_test(0, "%07u: i %06u (%u)\n", i, val, status);

				AVLTreePrint(&testTree, g_massiveBuffer, 1, GK_MASSIVE_BUFFER_SIZE - g_massiveBufferCurrIdx, printTreeDataMember);
			}
			break;

		case CTEST_AVL_OPER_DELETE_OP:
			if (treeValueSet.size != 0) {
				tmpValueIdx = random32u() % treeValueSet.size;
				tmpValue    = treeValueSet.data[tmpValueIdx];
				status      = AVLTreeRemove(&testTree, &tmpValue);
				array_erase(&treeValueSet, tmpValueIdx);
				++deletion[status == BINARY_TREE_OP_SUCCESS ? 1 : 0];
				log_test(0, "%07u: d %06u (%u)\n", i, tmpValue, status);

				AVLTreePrint(&testTree, g_massiveBuffer, 1, GK_MASSIVE_BUFFER_SIZE - g_massiveBufferCurrIdx, printTreeDataMember);
			}
			break;

		case CTEST_AVL_OPER_SEARCH_RAND_OP:
			tmpValue = val;
			status   = AVLTreeSearch(&testTree, &tmpValue);
			(*searchRandomValueOp)++;
			(*searchRandomValueSuccess) += (status == BINARY_TREE_BOOL_TRUE);
			(*searchRandomValueFailure) += (status == BINARY_TREE_BOOL_FALSE);

			log_test(0, "%07u: rs %06u (%u)\n", i, tmpValue, status);
			break;

		case CTEST_AVL_OPER_SEARCH_SET_OP:
			if(treeValueSet.size != 0) {
				tmpValueIdx = random32u() % treeValueSet.size;
				tmpValue    = treeValueSet.data[tmpValueIdx];
				status      = AVLTreeSearch(&testTree, &tmpValue);
				(*searchExistingValueOp)++;
				(*searchExistingValueSuccess) += (status == BINARY_TREE_BOOL_TRUE);
				(*searchExistingValueFailure) += (status == BINARY_TREE_BOOL_FALSE);
				log_test(0, "%07u: ss %06u (%u)\n", i, tmpValue, status);
			}
			break;

		case CTEST_AVL_OPER_MAX_OP:
		default:
		break;
		}


		/* Integrity check every 10 ops */
		if (i % 10 == 0) {
			assert_true(AVLTreeIsBalanced(&testTree));
			assert_int_equal(AVLTreeSize(&testTree), treeValueSet.size);
		}
		// printf("%06u", i);
	}

	printf("\n");
	log_test(0, "\n[==========] Stochastic Stress Diagnostics\n");
	log_test(0, "             Insertions              (Success, Failure): %06u %06u\n", insertion[1], insertion[0]);
	log_test(0, "             Deletions               (Success, Failure): %06u %06u\n", deletion[1], deletion[0]);
	log_test(0, "             Searches                (Random, Existing): %06u %06u\n", *searchRandomValueOp, *searchExistingValueOp);
	log_test(0, "             Random   Value Searches (Success, Failure): %06u %06u\n", *searchRandomValueSuccess, *searchRandomValueFailure);
	log_test(0, "             Existing Value Searches (Success, Failure): %06u %06u\n", *searchExistingValueSuccess, *searchExistingValueFailure);
	log_test(0, "             Final Size : %" PRIu64 "\n", (uint64_t) AVLTreeSize(&testTree));
	log_test(0, "             Tree Height: %u\n", AVLTreeHeight(&testTree));

	free(randomValueBuf);
	array_destroy(&treeValueSet);
	AVLTreeDestroy(&testTree);
	return;
}


int run_all_c_avl_tree_tests() {
	const struct CMUnitTest tests[] = {
	    cmocka_unit_test(AVLTreeBasicInsertionAndSearch),
	    cmocka_unit_test(AVLTreeSingleRotationsLeftLeft),
	    cmocka_unit_test(AVLTreeSingleRotationsRightRight),
	    cmocka_unit_test(AVLTreeDoubleRotationsLeftRight),
	    cmocka_unit_test(AVLTreeDoubleRotationsRightLeft),
	    cmocka_unit_test(AVLTreeDeletionRebalancing),
	    cmocka_unit_test(AVLTreeManualVerificationInsertDeleteTest),
	    cmocka_unit_test(AVLTreeRandomOperationsFuzzStressTest)
	};

	return cmocka_run_group_tests_name("AVLTree", tests, setup_c_report_buffer, teardown_c_report_buffer);
}




void log_test(uint8_t directlyToFile, const char* formatstr, ...) {
    va_list args;
    va_start(args, formatstr);

	if(directlyToFile) {
		vfprintf(g_reportFile, formatstr, args);
		va_end(args);
		return;
	}


    u64 bytesWritten = vsnprintf(
		&g_massiveBuffer[g_massiveBufferCurrIdx],
		GK_MASSIVE_BUFFER_SIZE - g_massiveBufferCurrIdx,
		formatstr,
		args
	);
    va_end(args);

    g_massiveBufferCurrIdx += (bytesWritten > 0) ? bytesWritten : 0;
    ifcrashfmt(g_massiveBufferCurrIdx >= GK_MASSIVE_BUFFER_SIZE,
        "Report Buffer Index Reached %" PRIu64 "/%" PRIu64 " Bytes\n",
        g_massiveBufferCurrIdx,
        GK_MASSIVE_BUFFER_SIZE
    );
	return;
}


int8_t uint32_cmp(const void* a, const void* b) {
	uint32_t valA = *(const uint32_t*) a;
	uint32_t valB = *(const uint32_t*) b;

	return valA == valB ? 0 : (valA < valB) ? -1 : 1;
}


void printTreeDataMember(
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


int setup_c_report_buffer(void** state) {
	(void) state;
	g_massiveBuffer = (char*) util2_aligned_malloc(GK_MASSIVE_BUFFER_SIZE, CACHE_LINE_BYTES);
	g_reportFile    = fopen("avl_test_report.txt", "w");

	assert_non_null(g_reportFile);
	assert_non_null(g_massiveBuffer);

	setvbuf(g_reportFile, NULL, _IONBF, 0);
	g_massiveBuffer[GK_MASSIVE_BUFFER_SIZE - 1] = '\0';
	return 0;
}

int teardown_c_report_buffer(void** state) {
	(void) state;
	log_test(0, "g_massiveBuffer Consumed %" PRIu64 "/%" PRIu64 " Bytes for %u Operations\n", g_massiveBufferCurrIdx, (uint64_t) GK_MASSIVE_BUFFER_SIZE, GK_STEST_TOTAL_OPS);
	if (g_reportFile) {
		fprintf(g_reportFile, "%s", g_massiveBuffer);
		fclose(g_reportFile);
	}
	util2_aligned_free(g_massiveBuffer);
	return 0;
}
