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
    toCreate->m_maxObjCount = firstObjCount == 0 ? 1 : firstObjCount;
    toCreate->m_buffer = (uint8_t*)malloc( (uint64_t)toCreate->m_maxObjCount * toCreate->m_objSize);
    return toCreate->m_buffer == NULL ? 1 : 0;
}

void GenericStackDestroy(GenericStack* toDestroy) {
    free(toDestroy->m_buffer);
    memset(toDestroy, 0x00, sizeof(GenericStack));
    return;
}



uint8_t GenericStackPush(
    GenericStack* toModify, 
    void* objAddr
) {
    uint8_t  failStatus = 0;
    uint8_t* destptr    = NULL;


    if(toModify->m_objCount == toModify->m_maxObjCount) { /* Stack is full */
        uint32_t newMaxSize = toModify->m_maxObjCount == 1 ? 
            3 : (3 * toModify->m_maxObjCount) / 2;

        failStatus = GenericStackGrow(toModify, newMaxSize);
        if(failStatus) {
            return 1;
        }
    }


    destptr = &toModify->m_buffer[ (uint64_t)toModify->m_objSize * toModify->m_objCount ];
    memcpy(destptr, objAddr, toModify->m_objSize);
    ++toModify->m_objCount;
    return 0;
}


uint8_t GenericStackTop(
    GenericStack const* toRead,  
    void* objAddrToWriteTo
) {
    if(GenericStackEmpty(toRead)) {
        return 1;
    }


    uint8_t* srcptr = &toRead->m_buffer[ (uint64_t)toRead->m_objSize * (toRead->m_objCount - 1) ];
    memcpy(objAddrToWriteTo, srcptr, toRead->m_objSize);
    return 0;
}


void GenericStackPop(GenericStack* toModify) {
    if(GenericStackEmpty(toModify)) {
        return;
    }


    --toModify->m_objCount;
    return;
}


uint8_t GenericStackEmpty(GenericStack const* toRead) {
    return toRead->m_objCount == 0;
}

uint32_t GenericStackSize(GenericStack const* toRead) {
    return toRead->m_objCount;
}


static uint8_t GenericStackGrow(
    GenericStack* toReAlloc,
    uint32_t      newObjectCount
) {
    uint8_t* newBuffer = (uint8_t*)realloc(toReAlloc->m_buffer, (uint64_t)newObjectCount * toReAlloc->m_objSize);
    toReAlloc->m_buffer      = (newBuffer != NULL) ? newBuffer      : toReAlloc->m_buffer;
    toReAlloc->m_maxObjCount = (newBuffer != NULL) ? newObjectCount : toReAlloc->m_maxObjCount;
    return newBuffer == NULL;
}
