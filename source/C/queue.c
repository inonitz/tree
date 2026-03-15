#include <tree/C/queue.h>
#include <stdlib.h>
#include <string.h>


static uint8_t GenericQueueGrow(
    GenericQueue* toReAlloc,
    uint32_t      newObjectCount
);


uint8_t GenericQueueCreate(
    GenericQueue* toCreate,
    uint32_t      objSize,
    uint32_t      capacity
) {
    toCreate->m_objSize     = objSize;
    toCreate->m_objCount    = 0;
    toCreate->m_maxObjCount = capacity == 0 ? 1 : capacity;
    toCreate->m_head        = 0;
    toCreate->m_tail        = 0;
    toCreate->m_buffer = (uint8_t*)malloc((uint64_t)toCreate->m_maxObjCount * toCreate->m_objSize);
    return toCreate->m_buffer == NULL ? 1 : 0;
}


void GenericQueueDestroy(GenericQueue* toDestroy) {
    free(toDestroy->m_buffer);
    memset(toDestroy, 0x00, sizeof(GenericQueue));
    return;
}


uint8_t GenericQueuePush(
    GenericQueue* toModify, 
    void*         objAddr
) {
    uint8_t  failStatus = 0;
    uint8_t* destptr    = NULL;
    
    /* Queue is full */
    if(toModify->m_objCount == toModify->m_maxObjCount) 
    {
        uint32_t newMaxSize = toModify->m_maxObjCount == 1 ? 
            3 : (3 * toModify->m_maxObjCount) / 2;

        failStatus = GenericQueueGrow(toModify, newMaxSize);
        if(failStatus) {
            return 1;
        }
    }


    destptr = &toModify->m_buffer[ (uint64_t)toModify->m_objSize * toModify->m_tail ];
    memcpy(destptr, objAddr, toModify->m_objSize);

    toModify->m_tail = (toModify->m_tail + 1) % toModify->m_maxObjCount;
    ++toModify->m_objCount;
    return 0;
}


uint8_t GenericQueueFront(GenericQueue const* toRead, void* objAddrToWriteTo) {
    if(GenericQueueEmpty(toRead)) {
        return 1;
    }

    void* srcptr = &toRead->m_buffer[ (uint64_t)toRead->m_objSize * toRead->m_head ];
    memcpy(objAddrToWriteTo, srcptr, toRead->m_objSize);
    return 0;
}


void GenericQueuePop(GenericQueue* toModify) {
    if(GenericQueueEmpty(toModify)) {
        return;
    }

    void* objectToReset = &toModify->m_buffer[ (uint64_t)toModify->m_objSize * toModify->m_head ];
    memset(objectToReset, 0x00, toModify->m_objSize);

    toModify->m_head = (toModify->m_head + 1) % toModify->m_maxObjCount;
    --toModify->m_objCount;
    return;
}


uint8_t GenericQueueEmpty(GenericQueue const* toRead) {
    return toRead->m_objCount == 0;
}


uint32_t GenericQueueSize(GenericQueue const* toRead) {
    return toRead->m_objCount;
}


static uint8_t GenericQueueGrow(
    GenericQueue* q,
    uint32_t      newObjectCount
) {
    uint8_t* newBuffer = (uint8_t*)malloc( (uint64_t)newObjectCount * q->m_objSize);
    if (newBuffer == NULL) {
        return 1;
    }


    if (q->m_head < q->m_tail) {
        /* Case 1: Data is already contiguous (no wrap-around) */
        memcpy(
            newBuffer, 
            &q->m_buffer[q->m_head * q->m_objSize], 
            (uint64_t)q->m_objCount * q->m_objSize
        );
    } 
    else {
        /* Case 2: Data is wrapped */


        /* Part A: From head to the physical end of the old buffer */
        uint64_t firstPartCount = q->m_maxObjCount - q->m_head;
        memcpy(
            newBuffer, 
            &q->m_buffer[q->m_head * q->m_objSize], 
            firstPartCount * q->m_objSize
        );

        /* Part B: From the start of the old buffer to tail */
        uint64_t secondPartCount = q->m_tail;
        memcpy(
            &newBuffer[firstPartCount * q->m_objSize], 
            q->m_buffer, 
            secondPartCount * q->m_objSize
        );
    }


    free(q->m_buffer);
    q->m_buffer      = newBuffer;
    q->m_maxObjCount = newObjectCount;
    q->m_head        = 0;
    q->m_tail        = q->m_objCount;
    return 0;
}