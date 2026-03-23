#ifndef __BINARY_TREE_PRETTY_PRINT_GENERIC_VALUE_DEFINITION_HEADER__
#define __BINARY_TREE_PRETTY_PRINT_GENERIC_VALUE_DEFINITION_HEADER__
#include <stdint.h>


typedef struct binaryTreeNodeVariablePrintingContext {
    void*    m_buf;
    uint64_t m_bufSize; /* if == 0 -> FILE* */
    uint64_t m_bufOffset;
} binaryTreePrintCtx;


/* This function is responsible to increment the m_bufOffset/fail accordingly. */
typedef void (*binaryTreeNodeDataPrinterFunc)(
    binaryTreePrintCtx* ctx,
    const void* value
);


#endif /* __BINARY_TREE_PRETTY_PRINT_GENERIC_VALUE_DEFINITION_HEADER__ */
