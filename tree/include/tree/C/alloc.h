#ifndef __TREELIB_MEMORY_ALLOCATOR_DEFINITION_HEADER__
#define __TREELIB_MEMORY_ALLOCATOR_DEFINITION_HEADER__
#include <stdint.h>
#include <stdlib.h>


#ifndef treelibMallocTypeExplicit
#   define treelibMallocTypeExplicit(Type) \
        (  (Type*)(malloc(sizeof(Type)))  )
#endif /* treelibMallocTypeExplicit */

#ifndef treelibMallocBytesExplicit
#   define treelibMallocBytesExplicit(Type, ObjSize, Size) \
    (  (Type*)malloc( ((uint64_t)Size) * (ObjSize) )  )
#endif /* treelibMallocBytesExplicit */

#ifndef treelibMallocBufferExplicit
#   define treelibMallocBufferExplicit(Type, Size) \
    treelibMallocBytesExplicit(Type, sizeof(Type), Size)
#endif /* treelibMallocBufferExplicit */


#ifndef treelibReallocBytesExplicit
#   define treelibReallocBytesExplicit(Type, OriginalBuffer, ObjSize, Size) \
    (  (Type*)realloc(OriginalBuffer, ((uint64_t)Size) * (ObjSize) )  )
#endif /* treelibMallocBytesExplicit */

#ifndef treelibFreeTypeExplicit
#   define treelibFreeTypeExplicit(Pointer) \
        (  free( (void*)(Pointer) )  )
#endif /* treelibFreeTypeExplicit */


#endif /* __TREELIB_MEMORY_ALLOCATOR_DEFINITION_HEADER__ */
