#include <tree/C/stack.h>
#include <stdlib.h>
#include <string.h>


static uint8_t GenericStackGrow(
    GenericStack* toReAlloc,
    uint32_t      newObjectCount
);


uint8_t GenericStackCreate (
    GenericStack* toCreate,
    uint32_t      objectSizeInBytes,
    uint32_t      firstObjCount
) {
    toCreate->m_objSize  = objectSizeInBytes;
    toCreate->m_objCount = 0;
    toCreate->m_maxObjCount  = objectSizeInBytes * firstObjCount;
    toCreate->m_buffer = (uint8_t*)malloc( (uint64_t)toCreate->m_maxObjCount * toCreate->m_objSize);
    return toCreate->m_buffer == NULL ? 1 : 0;
}

void GenericStackDestroy(GenericStack* toDestroy) {
    toDestroy->m_objSize     = 0;
    toDestroy->m_objCount    = 0;
    toDestroy->m_maxObjCount = 0;
    free(toDestroy->m_buffer);
    toDestroy->m_buffer      = NULL;
    return;
}



uint8_t GenericStackPush(
    GenericStack* toModify, 
    void* objAddr
) {
    uint8_t  failStatus = 0;
    uint8_t* destptr    = NULL;


    if(toModify->m_objCount == toModify->m_maxObjCount) { /* Stack is full */
        failStatus = GenericStackGrow(toModify, (3 * toModify->m_maxObjCount) / 2);
        if(failStatus) {
            return 1;
        }
    }


    destptr = &toModify->m_buffer[ (uint64_t)toModify->m_objSize * toModify->m_objCount ];
    memcpy(destptr, objAddr, toModify->m_objSize);
    ++toModify->m_objCount;
    return 0;
}


void GenericStackTop(
    GenericStack const* toRead,  
    void* objAddrToWriteTo
) {
    uint8_t* srcptr = &toRead->m_buffer[ (uint64_t)toRead->m_objSize * toRead->m_objCount ];

    memcpy(objAddrToWriteTo, srcptr, toRead->m_objSize);
    return;
}


void GenericStackPop(GenericStack* toModify) {
    if(GenericStackEmpty(toModify)) {
        return;
    }
    --toModify->m_objCount;
}


uint8_t GenericStackEmpty(GenericStack const* toRead) {
    return toRead->m_objCount == 0;
}



static uint8_t GenericStackGrow(
    GenericStack* toReAlloc,
    uint32_t      newObjectCount
) {
    uint8_t* newBuffer = (uint8_t*)malloc( (uint64_t)newObjectCount * toReAlloc->m_objSize);

    if(newBuffer == NULL) {
        return 1;
    }


    memcpy(newBuffer, toReAlloc->m_buffer, (uint64_t)toReAlloc->m_objCount * toReAlloc->m_objSize);
    free(toReAlloc->m_buffer);

    toReAlloc->m_buffer      = newBuffer;
    toReAlloc->m_maxObjCount = newObjectCount;
    return 0;
}