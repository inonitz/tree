#include <stdlib.h>
#include <tree/C/dynamic_pool.h>
#include <string.h>


#ifndef FREELIST_NODE
#   define FREELIST_NODE(struct_ptr, index) (struct_ptr)->m_freeList[index]
#endif /* FREELIST_NODE */

#ifndef FREELIST_AVAILABLE_NODE
#   define FREELIST_AVAILABLE_NODE(struct_ptr) FREELIST_NODE(struct_ptr, (struct_ptr)->m_availableIdx)
#endif /* FREELIST_AVAILABLE_NODE */

#ifndef POOL_BUFFER_AT
#   define POOL_BUFFER_AT(struct_ptr, index) (struct_ptr)->m_buf[ (uint64_t)((struct_ptr)->m_elemSize) * (index) ]
#endif /* POOL_BUFFER_AT */

#ifndef BLOCK_OCCUPIED
#   define BLOCK_OCCUPIED(struct_ptr, index) ( FREELIST_NODE(struct_ptr, index).m_indexInBuf < 0 )
#endif /* BLOCK_OCCUPIED */

#ifndef BLOCK_INDEX_FROM_POINTER
#   define BLOCK_INDEX_FROM_POINTER(struct_ptr, freed_ptr) ( (uint8_t*)(struct_ptr)->m_buf ) - ((uint8_t*)freed_ptr)
#endif /* BLOCK_INDEX_FROM_POINTER */


bool GenericDynamicPoolExpand(
    genericDynamicPool* toModify, 
    uint32_t            newCapacity
);




bool GenericDynamicPoolCreate(
    genericDynamicPool* toCreate, 
    uint32_t            objSizeBytes, 
    uint32_t            initialAmountOfObjects
) {
    memset(toCreate, 0x00, sizeof(genericDynamicPool));
    
    toCreate->m_elemSize = objSizeBytes;
    if(initialAmountOfObjects == 0) {
        return 1;
    }

    toCreate->m_elemCount    = 0;
    toCreate->m_elemCountMax = initialAmountOfObjects;
    toCreate->m_buf      = malloc(GenericDynamicPoolTotalBytes(toCreate));
    toCreate->m_freeList = malloc(sizeof(poolNode_t) * toCreate->m_elemCountMax);
    if(toCreate->m_buf == NULL || toCreate->m_freeList == NULL) {
        if(toCreate->m_buf) {
            free(toCreate->m_buf);
        }
        if(toCreate->m_freeList) {
            free(toCreate->m_freeList);
        }
        return 1;
    }

    for(uint32_t i = 0; i < toCreate->m_elemCountMax - 1; ++i) {
        toCreate->m_freeList[i] = (poolNode_t){
            .m_indexInBuf  = i + 1,
            .m_nextNodeIdx = i + 1
        };
    }
    toCreate->m_freeList[ toCreate->m_elemCountMax - 1 ] = (poolNode_t){ toCreate->m_elemCountMax, (poolNodeIndex_t)-1 };
    toCreate->m_availableIdx = 0;
    return 0;
}

void GenericDynamicPoolDestroy(genericDynamicPool* toDestroy) {

    if(toDestroy->m_buf != NULL) {
        free(toDestroy->m_buf);
    }
    if(toDestroy->m_freeList != NULL) {
        free(toDestroy->m_freeList);
    }
    memset(toDestroy, 0x00, sizeof(genericDynamicPool));
    return;
}


void* GenericDynamicPoolAllocate(genericDynamicPool* toModify) {
    if(GenericDynamicPoolFreeBlocks(toModify) == 0) {
        bool err = GenericDynamicPoolExpand(toModify, toModify->m_elemCountMax * 3 / 2);
        if(err) {
            return NULL;
        }
    }


    poolNode_t* availableNode = &FREELIST_AVAILABLE_NODE(toModify);
    uint8_t*    valueToAlloc  = &POOL_BUFFER_AT(toModify, availableNode->m_indexInBuf - 1); 
    availableNode->m_indexInBuf *= -1; /* now occupied */

    toModify->m_availableIdx = availableNode->m_nextNodeIdx;
    ++toModify->m_elemCount;
    return valueToAlloc;
}

void GenericDynamicPoolFree(genericDynamicPool* toModify, void* blockToFree) {
    uint32_t blockIdx = BLOCK_INDEX_FROM_POINTER(toModify, blockToFree);

    if(false
        || toModify->m_elemCount == 0          /* Cannot Free if we haven't allocated */
        || blockToFree == NULL                 /* Invalid pointer */
        || (blockIdx >= toModify->m_elemCount) /* Out-of-bounds pointer/Invalid pointer */
        || !BLOCK_OCCUPIED(toModify, blockIdx) /* Attempt to Free an-already free block */
    ) {
        return;
    }


    FREELIST_NODE(toModify, blockIdx).m_indexInBuf *= -1;
    FREELIST_NODE(toModify, blockIdx).m_nextNodeIdx *= toModify->m_availableIdx;
    toModify->m_availableIdx = blockIdx;
    --toModify->m_elemCount;


#ifndef NDEBUG
    /* Clear out Memory Of allocated block on debug */
    memset(
        &POOL_BUFFER_AT(toModify, blockIdx - 1),
        0x00,
        GenericDynamicPoolBlockSize(toModify)
    );
#endif
    return;
}


uint32_t GenericDynamicPoolBlockSize(genericDynamicPool const* toRead) {
    return toRead->m_elemSize;
}

uint32_t GenericDynamicPoolFreeBlocks(genericDynamicPool const* toRead) {
    return toRead->m_elemCountMax - toRead->m_elemCount;
}

uint32_t GenericDynamicPoolTotalBlocks(genericDynamicPool const* toRead) {
    return toRead->m_elemCountMax;
}

uint64_t GenericDynamicPoolTotalBytes(genericDynamicPool const* toRead) {
    return (uint64_t)GenericDynamicPoolBlockSize(toRead) * GenericDynamicPoolTotalBlocks(toRead);
}




bool GenericDynamicPoolExpand(
    genericDynamicPool* toModify, 
    uint32_t            newCapacity
) {
    /* 
        Care must be taken to ensure this function is called 
        ONLY when the existing pool is full.
        Otherwise, the last node in the pool will point to a NULL index,
        which, on allocation, will set m_availableIdx=NULL.
        
        The buffer would have resized to accomodate additional allocations,
        but the indices would be messed up.
    */
    if(newCapacity <= toModify->m_elemCountMax) {
        return 1;
    }

    uint8_t*    bBuf  = realloc(toModify->m_buf, (uint64_t)toModify->m_elemSize * newCapacity);
    poolNode_t* flBuf = realloc(toModify->m_freeList, sizeof(poolNode_t) * newCapacity);
    if(bBuf == NULL || flBuf == NULL) {
        if(bBuf) {
            free(bBuf);
        }
        if(flBuf) {
            free(flBuf);
        }
        return 1;
    }


    toModify->m_buf      = bBuf;
    toModify->m_freeList = flBuf;
    toModify->m_elemCountMax = newCapacity;
    for(uint32_t i = toModify->m_elemCount; i < newCapacity - 1; ++i) {
        toModify->m_freeList[i] = (poolNode_t){
            .m_indexInBuf  = i + 1,
            .m_nextNodeIdx = i + 1
        };
    }
    toModify->m_freeList[ newCapacity - 1 ] = (poolNode_t){ newCapacity, (poolNodeIndex_t)-1 };
    /* 
        the function is called when the pool is full, i.e m_availableIdx = -1
        because -1 is an invalid index (intentionally so) we need to reset it
        The first available block in the free-list is now in the newly allocated space.
        So, we assign m_availableIdx to the newly freed space (index).
    */
    toModify->m_availableIdx = toModify->m_elemCount;
    return 0;
}