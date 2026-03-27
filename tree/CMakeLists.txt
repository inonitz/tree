cmake_minimum_required(VERSION 3.14)
project(treelib 
    VERSION 0.0.1
    DESCRIPTION "Implementation of various Tree Structures"
    LANGUAGES C CXX
)


set(SOURCES
    source/tmp/AVLTreeDraft.cpp
    source/tmp/binaryTreeDraft.cpp

    source/tree/C/avl_tree.c
    source/tree/C/binary_tree.c
    source/tree/C/queue.c
    source/tree/C/stack.c
    source/tree/C/vector.c


    source/tree/internal/bitmap.cpp
)

set(HEADERS
    include/tmp/AVLTreeDraft.hpp
    include/tmp/binaryTreeDraft.hpp

    include/tree/C/avl_tree.h
    include/tree/C/binary_tree.h
    include/tree/C/compare_func.h
    include/tree/C/op_result.h
    include/tree/C/print_func.h
    include/tree/C/queue.h
    include/tree/C/stack.h
    include/tree/C/treelib_api.h
    include/tree/C/treelib_extern.h
    include/tree/C/vector.h

    include/tree/internal/AVLTreeGeneric.hpp
    include/tree/internal/AVLTreeGeneric.tpp
    include/tree/internal/AVLTreeRotateState.hpp

    include/tree/internal/binaryTreeGeneric.hpp
    include/tree/internal/binaryTreeGeneric.tpp
    

    include/tree/internal/FlatAVLTreeGeneric.hpp
    include/tree/internal/FlatAVLTreeGeneric.tpp

    include/tree/internal/treelib_util.hpp

    include/tree/AVLTree.hpp
    include/tree/AVLTreeImpl.hpp
)


if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TREELIB_EXPORTS)
else()
    add_library(${PROJECT_NAME} STATIC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TREELIB_STATIC_DEFINE)
endif()

add_library(TREELIB::treelib ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    UTIL2::util2
)

# The minimum C/C++ versions required for the library
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)
if(BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        C_VISIBILITY_PRESET       hidden
        CXX_VISIBILITY_PRESET     hidden

        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

