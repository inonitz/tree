#ifndef __SIMPLE_LINEAR_CIRCULAR_QUEUE_C_DEFINITION_HEADER__
#define __SIMPLE_LINEAR_CIRCULAR_QUEUE_C_DEFINITION_HEADER__
#include <tree/C/treelib_extern.h>
#include <tree/C/treelib_api.h>
#include <stdint.h>


TREELIB_EXTERNC_DECL_BEGIN


typedef struct {
    uint8_t* m_buffer;
    uint32_t m_objSize;
    uint32_t m_objCount;
    uint32_t m_maxObjCount;
    uint32_t m_head; // Index of the first element
    uint32_t m_tail; // Index where the next element will be pushed
} GenericQueue;


uint8_t  TREELIB_API GenericQueueCreate(
    GenericQueue* toCreate, 
    uint32_t      objSizeInBytes, 
    uint32_t      maximumInitialAmountOfObjects
);
void     TREELIB_API GenericQueueDestroy(GenericQueue*       toDestroy);
uint8_t  TREELIB_API GenericQueuePush   (GenericQueue*       toModify, void* objAddr);
uint8_t  TREELIB_API GenericQueueFront  (GenericQueue const* toRead,   void* objAddrToWriteTo);
void     TREELIB_API GenericQueuePop    (GenericQueue*       toModify);
uint8_t  TREELIB_API GenericQueueEmpty  (GenericQueue const* toRead);
uint32_t TREELIB_API GenericQueueSize   (GenericQueue const* toRead);


TREELIB_EXTERNC_DECL_END


#endif /* __SIMPLE_LINEAR_CIRCULAR_QUEUE_C_DEFINITION_HEADER__ */
