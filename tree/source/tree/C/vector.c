#include <tree/C/vector.h>
#include <stdlib.h>
#include <string.h>


void GenericVectorClearRange(uint8_t* begin, uint8_t* end);
bool GenericVectorChangeCapacity(GenericVector* toModify, uint32_t newCapacity);


void GenericVectorCreate(GenericVector* toCreate, uint32_t elementSizeBytes) {
    memset(toCreate, 0x00, sizeof(GenericVector));
    toCreate->m_objSize = elementSizeBytes;
    return;
}

bool GenericVectorCreateWithCapacity(GenericVector* toCreate, uint32_t elementSizeBytes, uint32_t capacity) {
    memset(toCreate, 0x00, sizeof(GenericVector));

    toCreate->m_buf = malloc(elementSizeBytes * capacity);
    if(toCreate->m_buf == NULL) {
        return 1;
    }

    toCreate->m_objSize     = elementSizeBytes;
    toCreate->m_objCapacity = capacity;
	return 0;
}

bool GenericVectorCreateWithSize(
    GenericVector* toCreate, 
    uint32_t       elementSizeBytes, 
    uint32_t       initialElementCount,
    const void*    defaultValue
) {
    bool failStatus = 0;

    GenericVectorCreate(toCreate, elementSizeBytes);
    failStatus = GenericVectorChangeCapacity(toCreate, initialElementCount);
    if(failStatus) {
        return 1;
    }
    toCreate->m_objCount = initialElementCount;


    uint8_t* dest = toCreate->m_buf;
    if(defaultValue == NULL) {
        memset(dest, 0x00, GenericVectorSizeBytes(toCreate));
    } else {
        for(uint32_t i = 0; i < initialElementCount; ++i) {
            memcpy(dest, defaultValue, elementSizeBytes);
            dest += elementSizeBytes;
        }
    }
    return 0;
}

void GenericVectorDestroy(GenericVector* toDestroy) {
    if(toDestroy->m_buf) {
        free(toDestroy->m_buf);
    }
    memset(toDestroy, 0x00, sizeof(GenericVector));
    return;
}


void* GenericVectorAt(GenericVector const* toRead, uint32_t index) {
	return &toRead->m_buf[(uint64_t)toRead->m_objSize * index];
}

void* GenericVectorGet(GenericVector const* toRead, uint32_t index) {
    if(  GenericVectorEmpty(toRead) || (index >= toRead->m_objCount) ) {
        return NULL;
    }
    return GenericVectorAt(toRead, index); 
}

void GenericVectorSetAt(GenericVector* toModify, uint32_t index, void const* val) {
    memcpy(GenericVectorAt(toModify, index), val, toModify->m_objSize);
    return;
}
void GenericVectorSet(GenericVector* toModify, uint32_t index, void const* val) {
    void* destptr = GenericVectorGet(toModify, index);
    if(destptr == NULL || val == NULL) {
        return;
    }
    memcpy(destptr, val, toModify->m_objSize);
    return;
}

void* GenericVectorFront(GenericVector const* toRead) {
	return toRead->m_buf;
}

void* GenericVectorBack(GenericVector const* toRead) {
    if(GenericVectorEmpty(toRead)) {
        return NULL;
    }
	return &toRead->m_buf[(uint64_t)toRead->m_objSize * (toRead->m_objCount - 1)];
}

void* GenericVectorData(GenericVector const* toRead) {
	return GenericVectorFront(toRead);
}


uint8_t* GenericVectorBegin(GenericVector const* toRead) {
	return toRead->m_buf;
}

uint8_t* GenericVectorEnd(GenericVector const* toRead) {
	return &toRead->m_buf[(uint64_t)toRead->m_objSize * toRead->m_objCount];
}


bool GenericVectorEmpty(GenericVector const* toRead) {
	return toRead->m_objCount == 0 || toRead->m_buf == NULL;
}

uint32_t GenericVectorSize(GenericVector const* toRead) {
	return toRead->m_objCount;
}

uint64_t GenericVectorSizeBytes(GenericVector const* toRead) {
	return (uint64_t)toRead->m_objSize * toRead->m_objCount;
}

uint32_t GenericVectorCapacity(GenericVector const* toRead) {
	return toRead->m_objCapacity;
}

uint64_t GenericVectorCapacityBytes(GenericVector const* toRead) {
	return (uint64_t)toRead->m_objSize * toRead->m_objCapacity;
}

bool GenericVectorReserve(GenericVector* toModify, uint32_t numElems) {
    /* Nothing to do if numElems <= m_objCapacity */
	if(numElems <= toModify->m_objCapacity) {
        return 0;
    }
    return GenericVectorChangeCapacity(toModify, numElems);
}

bool GenericVectorShrinkToFit(GenericVector* toModify) {
    return GenericVectorChangeCapacity(toModify, toModify->m_objCount);
}


void GenericVectorClear(GenericVector* toModify) {
    toModify->m_objCount = 0;
    return;
}

bool GenericVectorInsert(
    GenericVector* toModify, 
    uint32_t       index, 
    uint32_t       objCount,
    void const*    values
) {
    bool failStatus = GenericVectorReserve(toModify, toModify->m_objCount + objCount);
    if(failStatus) {
        return 1;
    }


    bool elementsAreAppended = 
        GenericVectorEmpty(toModify) 
        || 
        ( (toModify->m_objCount != 0) && (index == toModify->m_objCount) );
    
    if(!elementsAreAppended) {
        uint8_t* objectsSrc     = GenericVectorAt(toModify, index);
        uint8_t* objectsNewDest = GenericVectorAt(toModify, index + objCount);
        uint64_t bytesToMove    = (uint64_t)toModify->m_objSize * (toModify->m_objCount - index);
        memmove(objectsNewDest, objectsSrc, bytesToMove);
    }

    uint8_t* valuesDst = GenericVectorAt(toModify, index);
    memcpy(valuesDst, values, (uint64_t)toModify->m_objSize * objCount);
    toModify->m_objCount += objCount;
    return 0;
}

void GenericVectorErase(
    GenericVector* toModify, 
    uint32_t       index, 
    uint32_t       objCount
) {
    if(GenericVectorEmpty(toModify) || index >= toModify->m_objCount) {
        return;
    }
    
    
    /* Clamp objCount if it exceeds the number of elements available from index */
    objCount = (index + objCount > toModify->m_objCount) ? 
        toModify->m_objCount - index 
        : 
        objCount;


    uint32_t elemsLeftToMove = (toModify->m_objCount - index) - objCount;
    if (elemsLeftToMove > 0) {
        uint8_t* dest = (uint8_t*)GenericVectorAt(toModify, index);
        uint8_t* src  = (uint8_t*)GenericVectorAt(toModify, index + objCount);
        uint64_t bytesToMove = (uint64_t)toModify->m_objSize * elemsLeftToMove;

        memmove(dest, src, bytesToMove);
    }


    toModify->m_objCount -= objCount;
    return;
}

bool GenericVectorPushBack(GenericVector* toModify, void const* val) {
    if(toModify->m_objCount == toModify->m_objCapacity) 
    {
        bool tmpBool = (toModify->m_objCapacity == 1) || (toModify->m_objCapacity == 0);
        const uint32_t newMaxSize = tmpBool ? 
            3 : (3 * toModify->m_objCapacity) / 2;
        
        tmpBool = GenericVectorChangeCapacity(toModify, newMaxSize);
        if(tmpBool) {
            return 1;
        }
    }


    memcpy(GenericVectorEnd(toModify), val, toModify->m_objSize);
    ++toModify->m_objCount;
    return 0;
}

void GenericVectorPopBack(GenericVector* toModify) {
    if(GenericVectorEmpty(toModify)) {
        return;
    }

    --toModify->m_objCount;
    return;
}

bool GenericVectorResize(GenericVector* toModify, uint32_t newSize) {
    /* Buffer doesn't change. we shrink m_objCount and clear the elements. */
	if(newSize <= toModify->m_objCount) {
        GenericVectorClearRange(GenericVectorAt(toModify, newSize), GenericVectorEnd(toModify));
        toModify->m_objCount = newSize;
        return 0;
    }

    /* Buffer still large enough to accommodate reallocation. */
    if(newSize <= toModify->m_objCapacity) {
        /* newSize > m_objCount */
        GenericVectorClearRange(GenericVectorEnd(toModify), GenericVectorAt(toModify, newSize));
        toModify->m_objCount = newSize;
        return 0;
    }

    /* newSize > current Capacity. We Reserve a new Capacity (& Set==Size). */
    bool failStatus = GenericVectorChangeCapacity(toModify, newSize);
    if(failStatus) {
        return 1;
    }

    /* Clear all newly allocated elements. */
    GenericVectorClearRange(
        GenericVectorEnd(toModify), 
        &toModify->m_buf[GenericVectorCapacityBytes(toModify)]
    );
    toModify->m_objCount = toModify->m_objCapacity;
    return 0;
}

void GenericVectorSwap(GenericVector* vecA, GenericVector* vecB) {
    GenericVector tmp = {};
    memcpy(&tmp, vecA, sizeof(GenericVector)); /* Move A->Tmp */
    memcpy(vecA, vecB, sizeof(GenericVector)); /* Move B->A   */
    memcpy(vecB, &tmp, sizeof(GenericVector)); /* Move A->B   */
    return;
}




void GenericVectorClearRange(uint8_t* begin, uint8_t* end) {
    const uint64_t bytesToClear = (end - begin);
    memset(begin, 0x00, bytesToClear);
    return;
}


bool GenericVectorChangeCapacity(GenericVector* toModify, uint32_t numElems)
{
    if(numElems == 0) {
        uint32_t objSize = toModify->m_objSize;
        GenericVectorDestroy(toModify);
        toModify->m_objSize = objSize;
        return 0;
    }
    uint64_t newBufBytes = (uint64_t)toModify->m_objSize * numElems;
    uint8_t* newBuf      = realloc(toModify->m_buf, newBufBytes);

    toModify->m_buf         = (newBuf != NULL) ? newBuf   : toModify->m_buf;
    toModify->m_objCapacity = (newBuf != NULL) ? numElems : toModify->m_objCapacity;
    return newBuf == NULL;
}
