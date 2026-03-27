#ifndef __DYNAMIC_POOL_ALLOCATOR_DEFINITION_HEADER__
#define __DYNAMIC_POOL_ALLOCATOR_DEFINITION_HEADER__
#include <tree/C/treelib_extern.h>
#include <tree/C/treelib_api.h>
#include <stdbool.h>
#include <stdint.h>


TREELIB_EXTERNC_DECL_BEGIN


typedef int32_t poolNodeIndex_t;


typedef struct __dynamic_pool_allocator_node_type_v0 {
    int32_t         m_indexInBuf;
    poolNodeIndex_t m_nextNodeIdx;
} genericDynamicPoolNode;


typedef genericDynamicPoolNode poolNode_t;


typedef struct __dynamic_vector_like_pool_allocator_type_v0 {
    uint8_t*        m_buf;
    poolNode_t*     m_freeList;
    poolNodeIndex_t m_availableIdx;
    uint32_t m_elemSize;
    uint32_t m_elemCount;
    uint32_t m_elemCountMax;
} GenericDynamicPool;


bool TREELIB_API GenericDynamicPoolCreate(
    GenericDynamicPool* toCreate, 
    uint32_t            objSizeBytes, 
    uint32_t            initialAmountOfObjects
);
void TREELIB_API GenericDynamicPoolDestroy(GenericDynamicPool* toDestroy);

void* TREELIB_API GenericDynamicPoolAllocate(GenericDynamicPool* toModify);
void  TREELIB_API GenericDynamicPoolFree(GenericDynamicPool* toModify, void* blockToFree);

uint32_t TREELIB_API GenericDynamicPoolBlockSize(GenericDynamicPool const* toRead);
uint32_t TREELIB_API GenericDynamicPoolFreeBlocks(GenericDynamicPool const* toRead);
uint32_t TREELIB_API GenericDynamicPoolTotalBlocks(GenericDynamicPool const* toRead);
uint64_t TREELIB_API GenericDynamicPoolTotalBytes(GenericDynamicPool const* toRead);


TREELIB_EXTERNC_DECL_END


#endif /* __DYNAMIC_POOL_ALLOCATOR_DEFINITION_HEADER__ */
