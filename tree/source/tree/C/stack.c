#include <tree/C/stack.h>
#include <stdlib.h>
#include <string.h>


/* 
    Thanks to:
    https://stackoverflow.com/questions/2796639/implementation-of-ceil-function-in-c
*/
#define CEIL_POSITIVE(X) ( (X) - ((uint32_t)X) > 0 ? (X + 1) : X  )
    

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
    toCreate->m_maxObjCount  = firstObjCount == 0 ? 1 : firstObjCount;
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