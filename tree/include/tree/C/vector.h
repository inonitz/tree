#ifndef __DYNAMIC_ARRAY_LIKE_CPP_VECTOR_DEFINITION_HEADER__
#define __DYNAMIC_ARRAY_LIKE_CPP_VECTOR_DEFINITION_HEADER__
#include <tree/C/treelib_extern.h>
#include <tree/C/treelib_api.h>
#include <stdint.h>
#include <stdbool.h>


TREELIB_EXTERNC_DECL_BEGIN


typedef struct __dynamic_array_cpp_vector_equivalent_type_v0 {
    uint8_t* m_buf;
    uint32_t m_objSize;
    uint32_t m_objCount;
    uint32_t m_objCapacity;
    uint8_t  m_reserved[4];
} GenericVector;


void TREELIB_API GenericVectorCreate(
    GenericVector* toCreate, 
    uint32_t       elementSizeBytes
);
bool TREELIB_API GenericVectorCreateWithCapacity(
    GenericVector* toCreate, 
    uint32_t       elementSizeBytes, 
    uint32_t       maxElements
);
bool TREELIB_API GenericVectorCreateWithSize(
    GenericVector* toCreate, 
    uint32_t       elementSizeBytes, 
    uint32_t       initialElementCount,
    const void*    defaultValue
);
void TREELIB_API GenericVectorDestroy(GenericVector* toDestroy);


void* TREELIB_API GenericVectorAt (GenericVector const* toRead, uint32_t index);
void* TREELIB_API GenericVectorGet(GenericVector const* toRead, uint32_t index);
void  TREELIB_API GenericVectorSetAt(GenericVector* toModify, uint32_t index, void const* val);
void  TREELIB_API GenericVectorSet  (GenericVector* toModify, uint32_t index, void const* val);
void* TREELIB_API GenericVectorFront(GenericVector const* toRead);
void* TREELIB_API GenericVectorBack (GenericVector const* toRead);
void* TREELIB_API GenericVectorData (GenericVector const* toRead);

uint8_t* TREELIB_API GenericVectorBegin(GenericVector const* toRead);
uint8_t* TREELIB_API GenericVectorEnd  (GenericVector const* toRead);

bool     TREELIB_API GenericVectorEmpty(GenericVector const* toRead);
uint32_t TREELIB_API GenericVectorSize    (GenericVector const* toRead);
uint64_t TREELIB_API GenericVectorSizeBytes(GenericVector const* toRead);
uint32_t TREELIB_API GenericVectorCapacity(GenericVector const* toRead);
uint64_t TREELIB_API GenericVectorCapacityBytes(GenericVector const* toRead);
bool     TREELIB_API GenericVectorReserve    (GenericVector* toMaybeRealloc, uint32_t numElems);
bool     TREELIB_API GenericVectorShrinkToFit(GenericVector* toModify);


void TREELIB_API GenericVectorClear(GenericVector* toModify);
bool TREELIB_API GenericVectorInsert(
    GenericVector* toModify, 
    uint32_t       index, 
    uint32_t       objCount,
    void const*    values
);
void TREELIB_API GenericVectorErase(
    GenericVector* toModify, 
    uint32_t       index, 
    uint32_t       objCount
);
bool TREELIB_API GenericVectorPushBack(
    GenericVector* toModify, 
    void const*    val
);
void TREELIB_API GenericVectorPopBack(GenericVector* toModify);
bool TREELIB_API GenericVectorResize(GenericVector* toModify, uint32_t newNumOfElements);
void TREELIB_API GenericVectorSwap(
    GenericVector* vecA, 
    GenericVector* vecB
);


TREELIB_EXTERNC_DECL_END


#endif /* __DYNAMIC_ARRAY_LIKE_CPP_VECTOR_DEFINITION_HEADER__ */
