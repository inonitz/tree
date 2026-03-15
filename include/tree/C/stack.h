#ifndef __SIMPLE_LINEAR_STACK_C_DEFINITION_HEADER__
#define __SIMPLE_LINEAR_STACK_C_DEFINITION_HEADER__
#include <tree/C/treelib_api.h>
#include <stdint.h>


typedef struct __linear_stack_definition {
    uint8_t* m_buffer;
    uint32_t m_objSize;
    uint32_t m_objCount;
    uint32_t m_maxObjCount;
} GenericStack;


uint8_t TREELIB_API GenericStackCreate (
    GenericStack* toCreate,
    uint32_t      objectSizeInBytes,
    uint32_t      maximumInitialAmountOfObjects
);
void     TREELIB_API GenericStackDestroy(GenericStack*       toDestroy);
uint8_t  TREELIB_API GenericStackPush   (GenericStack*       toModify, void* objAddr);
uint8_t  TREELIB_API GenericStackTop    (GenericStack const* toRead,   void* objAddrToWriteTo);
void     TREELIB_API GenericStackPop    (GenericStack*       toModify);
uint8_t  TREELIB_API GenericStackEmpty  (GenericStack const* toRead);
uint32_t TREELIB_API GenericStackSize   (GenericStack const* toRead);

#endif /* __SIMPLE_LINEAR_STACK_C_DEFINITION_HEADER__ */
