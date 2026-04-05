#include "AVLTreeCTest.h"
#include <util2/C/aligned_malloc.h>
#include <util2/C/debugbreak.h>
#include <util2/C/ifcrash2.h>
#include <util2/C/mt19937ii.h>
#include <util2/C/random.h>
#include <tree/C/alloc.h>
#include <tree/C/binary_tree.h>
#include <tree/C/avl_tree.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>
#include <inttypes.h>


#define GK_TEST_TOTAL_OPS     (1 * 1000 * 1000)
#define GK_TEST_VAL_DIST_MIN  (1)
#define GK_TEST_VAL_DIST_MAX  (2000000)
#define GK_MASSIVE_BUFFER_SIZE (128ull * 1024 * 1024)
#define GK_MANUAL_TREE_SIZE    (100)
#define LOG_OPTION_DIRECTLY_TO_FILE 0
#define LOG_OPTION_REDIRECT_CACHE_BUFFER 1
#define LOG_OPTION_CANCEL_OPERATION 2
#define LOG_OPTION_GLOBAL_DEFAULT 0xFF


/* Global Test Logging State */
static CTestLoggingOption_t g_logOption 	   	   = CTEST_LOG_OPTION_CANCEL_OPERATION;
static FILE*    			g_reportFile           = NULL;
static char*    			g_massiveBuffer        = NULL;
static uint64_t 			g_massiveBufferCurrIdx = 0;


static void   log_test(CTestLoggingOption_t option, const char* formatstr, ...);
void 		  log_test_treeprint(AVLTree* tree, CTestLoggingOption_t option);
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
	uint64_t    size;
	uint64_t    capacity;
} UIntArray;

static void array_init(UIntArray* arr) {
	arr->size     = 0;
	arr->capacity = 1024;
	arr->data 	  = treelibMallocBufferExplicit(uint32_t, arr->capacity);
	ifcrash(arr->data == NULL);
	return;
}

static void array_push_back(UIntArray* arr, uint32_t val) {
	if (arr->size >= arr->capacity) {
		arr->capacity *= 2;
		arr->data = treelibReallocBytesExplicit(uint32_t, arr->data, sizeof(uint32_t), arr->capacity);
		ifcrash(arr->data == NULL);
	}
	arr->data[arr->size++] = val;
	return;
}

static void array_erase(UIntArray* arr, uint64_t index) {
	ifcrash(index >= arr->size);

	memmove(&arr->data[index], &arr->data[index + 1], (arr->size - index - 1) * sizeof(uint32_t));
	arr->size--;
	return;
}

static void array_destroy(UIntArray* arr) {
	if (arr->data) {
		treelibFreeTypeExplicit(arr->data);
		arr->data = NULL;
	}
	arr->size     = 0;
	arr->capacity = 0;
}
/* --------------------------------------------------------- */




static void AVLTreeCreateEmptyCopyTest(void **state) {
    (void)state;
    AVLTree src, dst;
	binaryTreeResult_t res = BINARY_TREE_OP_SUCCESS;
    AVLTreeCreate(&src, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    AVLTreeCreate(&dst, uint32_cmp, NULL, NULL, sizeof(uint32_t));

    assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeCreateCopy(&src, &dst));
    assert_int_equal(AVLTreeSize(&dst), 0);
    assert_null(dst.m_root);
    
    AVLTreeDestroy(&src);
    AVLTreeDestroy(&dst);
	return;
}


static void AVLTreeCreatePopulatedCopyTest(void **state) {
    (void)state;
    AVLTree src, dst;
    uint32_t v1 = 10, v2 = 20, v3 = 5;
	binaryTreeResult_t res = BINARY_TREE_OP_SUCCESS;
    AVLTreeCreate(&src, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    AVLTreeCreate(&dst, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    
    assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&src, &v1) );
    assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&src, &v2) );
    assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&src, &v3) );
    assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeCreateCopy(&src, &dst));
    assert_int_equal(AVLTreeSize(&dst), AVLTreeSize(&src));
	assert_int_equal(0, uint32_cmp(src.m_root->m_data, dst.m_root->m_data));
	assert_int_equal(0, uint32_cmp(src.m_root->m_left->m_data, dst.m_root->m_left->m_data));
	assert_int_equal(0, uint32_cmp(src.m_root->m_right->m_data, dst.m_root->m_right->m_data));
    assert_non_null(dst.m_root);
    
    AVLTreeDestroy(&src);
    AVLTreeDestroy(&dst);
	return;
}


static void AVLTreeBasicInsertionAndSearchTest(void** state) {
	(void)state;
	AVLTree tree;
	uint32_t vals[] = {50, 30, 70, 100};

	AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	assert_true(AVLTreeEmpty(&tree));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));
	assert_int_equal(AVLTreeSize(&tree), 3);
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &vals[3]));

	AVLTreeDestroy(&tree);
	return;
}


static void AVLTreeSingleRotationsLeftLeftTest(void** state) {
	(void) state;
	AVLTree tree;
	AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));

	uint32_t vals[] = {30, 20, 10, 40};
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));

	binaryTreeStatusPair_t treestate = AVLTreeIsBalanced(&tree);
	assert_int_equal(treestate.m_bool, BINARY_TREE_BOOL_TRUE);
	assert_int_equal(treestate.m_op, BINARY_TREE_OP_SUCCESS);
	assert_int_equal(AVLTreeSize(&tree), 3);

	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[2]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &vals[3]));

	AVLTreeDestroy(&tree);
	return;
}


static void AVLTreeSingleRotationsRightRightTest(void** state) {
	(void) state;
	AVLTree tree;
	AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));

	uint32_t vals[] = {10, 20, 30, 40};
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));

	binaryTreeStatusPair_t treestate = AVLTreeIsBalanced(&tree);
	assert_int_equal(treestate.m_bool, BINARY_TREE_BOOL_TRUE);
	assert_int_equal(treestate.m_op, BINARY_TREE_OP_SUCCESS);
	assert_int_equal(AVLTreeSize(&tree), 3);

	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_BOOL_TRUE, AVLTreeSearch(&tree, &vals[2]));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &vals[3]));

	AVLTreeDestroy(&tree);
	return;
}


static void AVLTreeDoubleRotationsLeftRightTest(void** state) {
	(void) state;
	AVLTree  tree;
	uint32_t vals[] = {30, 10, 20};

	AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));
	binaryTreeStatusPair_t treestate = AVLTreeIsBalanced(&tree);
	assert_int_equal(treestate.m_bool, BINARY_TREE_BOOL_TRUE);
	assert_int_equal(treestate.m_op, BINARY_TREE_OP_SUCCESS);

	AVLTreeDestroy(&tree);
	return;
}


static void AVLTreeDoubleRotationsRightLeftTest(void** state) {
	(void) state;
	AVLTree  tree;
	uint32_t vals[] = {10, 30, 20};

	AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[0]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[1]));
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[2]));

	binaryTreeStatusPair_t treestate = AVLTreeIsBalanced(&tree);
	assert_int_equal(treestate.m_bool, BINARY_TREE_BOOL_TRUE);
	assert_int_equal(treestate.m_op, BINARY_TREE_OP_SUCCESS);

	AVLTreeDestroy(&tree);
	return;
}


static void AVLTreeDeletionRebalancingTest(void** state) {
	(void) state;
	binaryTreeStatusPair_t treestate;
	AVLTree tree;
	uint32_t vals[]   = {50, 25, 75, 10, 35, 60, 90};
	uint64_t   num_vals = sizeof(vals) / sizeof(vals[0]);
	uint32_t val10 = 10;
	uint32_t val25 = 25;


    AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	for (uint64_t i = 0; i < num_vals; ++i) {
		assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &vals[i]));
	}


	/* Remove leaf */
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeRemove(&tree, &val10));
	assert_int_equal(BINARY_TREE_BOOL_FALSE, AVLTreeSearch(&tree, &val10));
	treestate = AVLTreeIsBalanced(&tree);
	assert_int_equal(treestate.m_bool, BINARY_TREE_BOOL_TRUE);
	assert_int_equal(treestate.m_op, BINARY_TREE_OP_SUCCESS);

	/* Remove node with two children */
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeRemove(&tree, &val25));
	treestate = AVLTreeIsBalanced(&tree);
	assert_int_equal(treestate.m_bool, BINARY_TREE_BOOL_TRUE);
	assert_int_equal(treestate.m_op, BINARY_TREE_OP_SUCCESS);

	AVLTreeDestroy(&tree);
    return;
}


static void AVLTreeCompareTreesEqualTest(void **state) {
    (void)state;
    AVLTree  treeA;
	AVLTree  treeB;
	uint32_t vals[]   = {50, 25, 75, 10, 35, 60, 90};
	uint64_t num_vals = sizeof(vals) / sizeof(vals[0]);
	binaryTreeStatusPair_t res;
    

    AVLTreeCreate(&treeA, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    AVLTreeCreate(&treeB, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	for (uint64_t i = 0; i < num_vals; ++i) {
		assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&treeA, &vals[i]));
		assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&treeB, &vals[i]));
	}
    
	res = AVLTreeCompare(&treeA, &treeB, uint32_cmp);
    assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
    assert_int_equal(res.m_bool, BINARY_TREE_BOOL_TRUE);
    
    AVLTreeDestroy(&treeA);
    AVLTreeDestroy(&treeB);
	return;
}


static void AVLTreeCompareTreesEqualFromCopyTest(void **state) {
    (void)state;
    AVLTree  treeA;
	AVLTree  treeB;
	uint32_t vals[]   = {50, 25, 75, 10, 35, 60, 90};
	uint64_t num_vals = sizeof(vals) / sizeof(vals[0]);
    binaryTreeStatusPair_t res;


	AVLTreeCreate(&treeA, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    AVLTreeCreate(&treeB, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	for (uint64_t i = 0; i < num_vals; ++i) {
		assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&treeA, &vals[i]));
	}
	assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeCreateCopy(&treeA, &treeB));
    
    res = AVLTreeCompare(&treeA, &treeB, uint32_cmp);
    assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
    assert_int_equal(res.m_bool, BINARY_TREE_BOOL_TRUE);
    
    AVLTreeDestroy(&treeA);
    AVLTreeDestroy(&treeB);
	return;
}


static void AVLTreeCompareTreesInequalEmptyEdgeCaseTest(void **state) {
    (void)state;
    AVLTree treeA, treeB;
    uint32_t v1 = 10;
	binaryTreeStatusPair_t res;


    AVLTreeCreate(&treeA, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    AVLTreeCreate(&treeB, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&treeA, &v1));

    res = AVLTreeCompare(&treeA, &treeB, uint32_cmp);
	assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
    assert_int_equal(res.m_bool, BINARY_TREE_BOOL_FALSE);
    
    AVLTreeDestroy(&treeA);
    AVLTreeDestroy(&treeB);
	return;
}


static void AVLTreeCompareTreesInequalFullTreeTest(void **state) {
    (void)state;
    AVLTree  treeA;
	AVLTree  treeB;
	uint32_t vals[]   = {50, 25, 75, 10, 35, 60, 90};
	uint64_t num_vals = sizeof(vals) / sizeof(vals[0]);
    binaryTreeStatusPair_t res;


	AVLTreeCreate(&treeA, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    AVLTreeCreate(&treeB, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	for (uint64_t i = 0; i < num_vals; ++i) {
		assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&treeA, &vals[i]));
	}
	vals[num_vals - 1] = 100;
	for (uint64_t i = 0; i < num_vals; ++i) {
		assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&treeB, &vals[i]));
	}
    
    res = AVLTreeCompare(&treeA, &treeB, uint32_cmp);
    assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
    assert_int_equal(res.m_bool, BINARY_TREE_BOOL_FALSE);
    
    AVLTreeDestroy(&treeA);
    AVLTreeDestroy(&treeB);
	return;
}


static void AVLTreeVerifyBalanceFactorTest(void **state) {
    (void)state;
    AVLTree  treeA;
	uint32_t vals[]   = {50, 25, 75, 10, 35, 60, 90};
	uint64_t num_vals = sizeof(vals) / sizeof(vals[0]);
    binaryTreeStatusPair_t res;


	AVLTreeCreate(&treeA, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	for (uint64_t i = 0; i < num_vals; ++i) {
		assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&treeA, &vals[i]));
	}

	/* Tree of Height 2 with 2 full nodes per parent. balance factor is 0 across all nodes */
    res = AVLTreeIsBalanced(&treeA);
    assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
    assert_int_equal(res.m_bool, BINARY_TREE_BOOL_TRUE);
    

	/* Randomly choose a node to mess up, check that it's being detected */
	for(uint32_t i = 0; i < 10; ++i) {
		binaryTreeNode* parent 	   = NULL;
		int8_t 			oldBalance = 0;

		for(binaryTreeNode* currNode = treeA.m_root; currNode != NULL; ) {
			parent = currNode;
			currNode = (random8i() > 0) ? currNode->m_left : currNode->m_right;
		}
		
		oldBalance = parent->m_balance;
		parent->m_balance = 2;
		res = AVLTreeIsBalanced(&treeA);
		assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
		assert_int_equal(res.m_bool, BINARY_TREE_BOOL_FALSE);
		parent->m_balance = oldBalance;
	}


    AVLTreeDestroy(&treeA);
	return;
}


static void AVLTreeVerifyIsValidBST(void **state) {
    (void)state;
    AVLTree tree;
	uint32_t rootVal = 50, leftVal = 25, rightVal = 75;
	uint32_t oopsyVal = 60;
    binaryTreeNode* oopsyNode = NULL;
	binaryTreeStatusPair_t res;


	oopsyNode = treelibMallocTypeExplicit(binaryTreeNode);
    AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    binaryTreeNodeCreate(oopsyNode, &oopsyVal, sizeof(uint32_t), NULL);

    /* 
		Construct a small tree:
            50
           /  \
          25   75
    */
    AVLTreeInsert(&tree, &rootVal);
    AVLTreeInsert(&tree, &leftVal);
    AVLTreeInsert(&tree, &rightVal);

    /* 
		Attach 60 as the right child of 25
		Per Binary Search Tree rules, 60 
		should not be in the left subtree of 50.
    */
    tree.m_root->m_left->m_right = oopsyNode;
    ++tree.m_nodeCount;
    res = AVLTreeIsValidBST(&tree);

	assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
	assert_int_equal(res.m_bool, BINARY_TREE_BOOL_FALSE);

    tree.m_root->m_left->m_right = NULL;
	binaryTreeNodeDestroy(oopsyNode, NULL);
    treelibFreeTypeExplicit(oopsyNode);
    AVLTreeDestroy(&tree);
	return;
}


static void AVLTreeVerifyIsValidBSTEmpty(void **state) {
    (void)state;
    AVLTree tree;
    binaryTreeStatusPair_t res;


    AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    res = AVLTreeIsValidBST(&tree);
    assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
    assert_int_equal(res.m_bool, BINARY_TREE_BOOL_TRUE);
    

    AVLTreeDestroy(&tree);
	return;
}


static void AVLTreeVerifyIsValidBSTDuplicateCheck(void **state) {
    (void)state;
    AVLTree tree;
    uint32_t val = 10;
	binaryTreeNode* dupNode = NULL;
	binaryTreeStatusPair_t res;

	AVLTreeCreate(&tree, uint32_cmp, NULL, NULL, sizeof(uint32_t));
    assert_int_equal(BINARY_TREE_OP_SUCCESS, AVLTreeInsert(&tree, &val));

	dupNode = treelibMallocTypeExplicit(binaryTreeNode);
    assert_int_equal(BINARY_TREE_OP_SUCCESS, 
		binaryTreeNodeCreate(dupNode, &val, sizeof(uint32_t), NULL)
	);


    /* Manually force a duplicate node into the tree */
    tree.m_root->m_left = dupNode;
    res = AVLTreeIsValidBST(&tree);
    
    /* BSTs must be strictly increasing; duplicates fail the > check */
    assert_int_equal(res.m_op, BINARY_TREE_OP_SUCCESS);
    assert_int_equal(res.m_bool, BINARY_TREE_BOOL_FALSE);
    
    tree.m_root->m_left = NULL;
	binaryTreeNodeDestroy(dupNode, NULL);
    treelibFreeTypeExplicit(dupNode);
    AVLTreeDestroy(&tree);
	return;
}


static void AVLTreeManualVerificationInsertDeleteTest(void** state) {
	(void)state;
	binaryTreeStatusPair_t opState;
	AVLTree  test;
	uint32_t data[GK_MANUAL_TREE_SIZE];
	uint32_t randIdx      = 0;
	uint32_t randValToDel = 0;
	int      c        	  = 0;


	AVLTreeCreate(&test, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	for (uint64_t i = 0; i < GK_MANUAL_TREE_SIZE; ++i) {
		data[i] = (random32u() % 101); /* 0 to 100 */
	}


	for (uint64_t i = 0; i < GK_MANUAL_TREE_SIZE; ++i) {
		uint32_t val = data[i];
		opState.m_op = AVLTreeInsert(&test, &val);

		log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "--- 2D Tree Visualization (Rotate head left) ---\n");
		log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "[c=%3u] Insertion Of %3u -> %s\n", c, val, opState.m_op == BINARY_TREE_OP_SUCCESS ? "SUCCESS" : "FAILURE");

		if (g_reportFile)
			AVLTreePrint(&test, g_reportFile, 0, 0, printTreeDataMember);

		opState = AVLTreeIsBalanced(&test);
		log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "Post Insertion AVL Tree Balanced ? (%s, %s)\n", 
			opState.m_op   == BINARY_TREE_OP_SUCCESS ? "SUCCESS" : "FAILURE",
			opState.m_bool == BINARY_TREE_BOOL_TRUE  ? "TRUE "   : "FALSE"
		);
		log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "-----------------------------------------------\n");
		++c;
	}


	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "-------------------------------------------------\n");
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "  ----- ManualInsertionDeletionTestMidway -----  \n");
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "-------------------------------------------------\n");
	fflush(g_reportFile);


	for (uint64_t i = 1; i < GK_MANUAL_TREE_SIZE; ++i)
	{
		randIdx      = (random32u() % GK_MANUAL_TREE_SIZE);
		randValToDel = data[randIdx];

		opState.m_op = AVLTreeRemove(&test, &randValToDel);

		log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "--- 2D Tree Visualization (Rotate head left) ---\n");
		log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "[c=%3u] Deletion Of %3u -> %s\n", c, randValToDel, opState.m_op == BINARY_TREE_OP_SUCCESS ? "SUCCESS" : "FAILURE");

		if (g_reportFile)
			AVLTreePrint(&test, g_reportFile, 0, 0, printTreeDataMember);

		opState = AVLTreeIsBalanced(&test);
		log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "Post Deletion AVL Tree Balanced ? (%s, %s)\n", 
			opState.m_op   == BINARY_TREE_OP_SUCCESS ? "SUCCESS" : "FAILURE",
			opState.m_bool == BINARY_TREE_BOOL_TRUE  ? "TRUE "   : "FALSE"
		);
		log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "-----------------------------------------------\n");
		++c;
	}

	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "-------------------------------------------------\n");
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "-------- ManualInsertionDeletionTestEnd --------\n");
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "-------------------------------------------------\n");
	AVLTreeDestroy(&test);
	return;
}




static void AVLTreeRandomOperationsFuzzStressTest(void** state) {
	(void)state;
	AVLTree treeA;
	AVLTree treeB;
	AVLTree* testTree;
	UIntArray treeValueSet;

	uint32_t randomValueBufSize = GK_TEST_TOTAL_OPS;
	uint32_t* randomValueBuf    = NULL;

	CTestOperationType op          = CTEST_AVL_OPER_MAX_OP;
	uint32_t           val         = 0;
	uint32_t           tmpValue    = 0;
	uint32_t           tmpValueIdx = 0;
	binaryTreeStatusPair_t status;

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


	AVLTreeCreate(&treeA, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	AVLTreeCreate(&treeB, uint32_cmp, NULL, NULL, sizeof(uint32_t));
	array_init(&treeValueSet);
	randomValueBuf = treelibMallocBufferExplicit(uint32_t, randomValueBufSize);
	

	f64 tmp = 0;
	for(u32 i = 0; i < randomValueBufSize; ++i) {
		tmp = random64f();
		tmp = tmp * (GK_TEST_VAL_DIST_MAX - GK_TEST_VAL_DIST_MIN) + GK_TEST_VAL_DIST_MIN;
		randomValueBuf[i] = (u32)tmp;
	}


	printf("AVLTreeCTest Begin\n");
	testTree = &treeA;
	for (uint32_t i = 0; i < GK_TEST_TOTAL_OPS; ++i) {
		// printf("\r\r\r\r\r\r");
		status = (binaryTreeStatusPair_t){
			BINARY_TREE_OP_SUCCESS,
			BINARY_TREE_BOOL_TRUE
		};
		val = randomValueBuf[i];
		op  = (CTestOperationType) (random32u() % CTEST_AVL_OPER_MAX_OP);

		switch (op)
		{
		case CTEST_AVL_OPER_INSERT_OP:
			if (AVLTreeSearch(testTree, &val) == BINARY_TREE_BOOL_FALSE) {
				log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "%07u: i %06u", i, val);

				status.m_op = AVLTreeInsert(testTree, &val);
				array_push_back(&treeValueSet, val);
				++insertion[status.m_op == BINARY_TREE_OP_SUCCESS ? 1 : 0];

				log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, " (%s)\n",
					status.m_op == BINARY_TREE_OP_SUCCESS ? "OP_SUCCESS" : "OP_FAILURE"
				);
				log_test_treeprint(testTree, CTEST_LOG_OPTION_GLOBAL_DEFAULT);
			}
			break;

		case CTEST_AVL_OPER_DELETE_OP:
			if (treeValueSet.size != 0) {
				tmpValueIdx = random32u() % treeValueSet.size;
				tmpValue    = treeValueSet.data[tmpValueIdx];
				log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "%07u: d %06u", i, tmpValue);

				status.m_op = AVLTreeRemove(testTree, &tmpValue);
				array_erase(&treeValueSet, tmpValueIdx);
				++deletion[status.m_op == BINARY_TREE_OP_SUCCESS ? 1 : 0];
				
				log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, " (%s)\n", 
					status.m_op == BINARY_TREE_OP_SUCCESS ? "OP_SUCCESS" : "OP_FAILURE"
				);
				log_test_treeprint(testTree, CTEST_LOG_OPTION_GLOBAL_DEFAULT);
			}
			break;

		case CTEST_AVL_OPER_SEARCH_RAND_OP:
			tmpValue = val;
			log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "%07u: rs %06u", i, tmpValue);

			status = (binaryTreeStatusPair_t){
				BINARY_TREE_OP_SUCCESS,
				AVLTreeSearch(testTree, &tmpValue)
			};
			(*searchRandomValueOp)++;
			(*searchRandomValueSuccess) += (status.m_bool == BINARY_TREE_BOOL_TRUE);
			(*searchRandomValueFailure) += (status.m_bool == BINARY_TREE_BOOL_FALSE);

			log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, " (%s)\n",
				status.m_bool == BINARY_TREE_BOOL_TRUE ? "TRUE" : "FALSE"
			);
			break;

		case CTEST_AVL_OPER_SEARCH_SET_OP:
			if(treeValueSet.size != 0) {
				tmpValueIdx = random32u() % treeValueSet.size;
				tmpValue    = treeValueSet.data[tmpValueIdx];
				log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "%07u: ss %06u", i, tmpValue);

				status = (binaryTreeStatusPair_t){
					BINARY_TREE_OP_SUCCESS,
					AVLTreeSearch(testTree, &tmpValue)
				};
				(*searchExistingValueOp)++;
				(*searchExistingValueSuccess) += (status.m_bool == BINARY_TREE_BOOL_TRUE);
				(*searchExistingValueFailure) += (status.m_bool == BINARY_TREE_BOOL_FALSE);

				log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, " (%s)\n", 
					status.m_bool == BINARY_TREE_BOOL_TRUE ? "TRUE" : "FALSE"
				);
			}
			break;

		case CTEST_AVL_OPER_COPY_OP:
		break;
			AVLTree* copyTo = (testTree == &treeA) ? &treeB : &treeA;
			log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "%07u: swapTo 0x%p", i, (void*)copyTo);
			
			status.m_op = AVLTreeCreateCopy(testTree, copyTo);
			AVLTreeDestroy(testTree);
			testTree = copyTo;
			
			log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, " (%s)\n",
				status.m_op == BINARY_TREE_OP_SUCCESS ? "OP_SUCCESS" : "OP_FAILURE"
			);
		break;
		case CTEST_AVL_OPER_MAX_OP:
		default:
		break;
		}


		/* Integrity check every 10 ops */
		if (i % 10 == 0) {
			status = AVLTreeIsBalanced(testTree);
			assert_int_equal(status.m_op,   BINARY_TREE_OP_SUCCESS);
			assert_int_equal(status.m_bool, BINARY_TREE_BOOL_TRUE);
			assert_int_equal(AVLTreeSize(testTree), treeValueSet.size);
		}
		// printf("%06u", i);
	}


	printf("AVLTreeCTest End\n");
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "\n[==========] Stochastic Stress Diagnostics\n");
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "             Insertions              (Success, Failure): %06u %06u\n", insertion[1], insertion[0]);
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "             Deletions               (Success, Failure): %06u %06u\n", deletion[1], deletion[0]);
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "             Searches                (Random, Existing): %06u %06u\n", *searchRandomValueOp, *searchExistingValueOp);
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "             Random   Value Searches (Success, Failure): %06u %06u\n", *searchRandomValueSuccess, *searchRandomValueFailure);
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "             Existing Value Searches (Success, Failure): %06u %06u\n", *searchExistingValueSuccess, *searchExistingValueFailure);
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "             Final Size : %" PRIu64 "\n", (uint64_t) AVLTreeSize(testTree));
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, "             Tree Height: %u\n", AVLTreeHeight(testTree));

	treelibFreeTypeExplicit(randomValueBuf);
	array_destroy(&treeValueSet);
	AVLTreeDestroy(testTree);
	return;
}


int run_all_c_avl_tree_tests() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(AVLTreeCreateEmptyCopyTest),
		cmocka_unit_test(AVLTreeCreatePopulatedCopyTest),
		cmocka_unit_test(AVLTreeBasicInsertionAndSearchTest),
		cmocka_unit_test(AVLTreeSingleRotationsLeftLeftTest),
		cmocka_unit_test(AVLTreeSingleRotationsRightRightTest),
		cmocka_unit_test(AVLTreeDoubleRotationsLeftRightTest),
		cmocka_unit_test(AVLTreeDoubleRotationsRightLeftTest),
		cmocka_unit_test(AVLTreeDeletionRebalancingTest),
		cmocka_unit_test(AVLTreeCompareTreesEqualTest),
		cmocka_unit_test(AVLTreeCompareTreesEqualFromCopyTest),
		cmocka_unit_test(AVLTreeCompareTreesInequalEmptyEdgeCaseTest),
		cmocka_unit_test(AVLTreeCompareTreesInequalFullTreeTest),
		cmocka_unit_test(AVLTreeVerifyBalanceFactorTest),
		cmocka_unit_test(AVLTreeVerifyIsValidBST),
		cmocka_unit_test(AVLTreeVerifyIsValidBSTEmpty),
		cmocka_unit_test(AVLTreeVerifyIsValidBSTDuplicateCheck),
		cmocka_unit_test(AVLTreeManualVerificationInsertDeleteTest),
		cmocka_unit_test(AVLTreeRandomOperationsFuzzStressTest),
	};

	return cmocka_run_group_tests_name("AVLTree", tests, setup_c_report_buffer, teardown_c_report_buffer);
}




void log_test(CTestLoggingOption_t option, const char* formatstr, ...) {
	option = (option == CTEST_LOG_OPTION_GLOBAL_DEFAULT) ? g_logOption : option;
	if(option == CTEST_LOG_OPTION_CANCEL_OPERATION || option == CTEST_LOG_OPTION_MAX) {
		return;
	}
	
	
	va_list args;
    va_start(args, formatstr);
	if(option == CTEST_LOG_OPTION_DIRECTLY_TO_FILE) {
		vfprintf(g_reportFile, formatstr, args);
		va_end(args);
		return;
	}

	/* Only option left is CTEST_LOG_OPTION_REDIRECT_CACHE_BUFFER */
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


void log_test_treeprint(AVLTree* tree, CTestLoggingOption_t option) {
	option = (option == CTEST_LOG_OPTION_GLOBAL_DEFAULT) ? g_logOption : option;
	if(option == CTEST_LOG_OPTION_CANCEL_OPERATION || option == CTEST_LOG_OPTION_MAX) {
		return;
	}

	if(option == CTEST_LOG_OPTION_DIRECTLY_TO_FILE) {
		AVLTreePrint(tree, g_reportFile, 0, 0, printTreeDataMember);
		return;
	}
	AVLTreePrint(tree, 
		g_massiveBuffer, 1, 
		GK_MASSIVE_BUFFER_SIZE - g_massiveBufferCurrIdx, 
		printTreeDataMember
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
	log_test(CTEST_LOG_OPTION_GLOBAL_DEFAULT, 
		"g_massiveBuffer Consumed %" PRIu64 "/%" PRIu64 " Bytes for %u Operations\n", 
		g_massiveBufferCurrIdx, 
		(uint64_t)GK_MASSIVE_BUFFER_SIZE, 
		GK_TEST_TOTAL_OPS
	);
	if (g_reportFile) {
		fprintf(g_reportFile, "%s", g_massiveBuffer);
		fclose(g_reportFile);
	}
	util2_aligned_free(g_massiveBuffer);
	return 0;
}
