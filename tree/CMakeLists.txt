cmake_minimum_required(VERSION 3.24)
project(treelib 
    VERSION 0.0.1
    DESCRIPTION "Various Implementations of an AVL Tree"
    LANGUAGES C CXX
)


set(SOURCES
    source/tree/C/avl_tree.c
    source/tree/C/binary_tree.c
    source/tree/C/queue.c
    source/tree/C/stack.c
    source/tree/C/vector.c
)

set(PUBLIC_HEADERS
    include/tree/C/avl_tree.h
    include/tree/C/binary_tree.h
    include/tree/C/compare_func.h
    include/tree/C/construct_func.h
    include/tree/C/op_result.h
    include/tree/C/print_func.h
    include/tree/C/queue.h
    include/tree/C/stack.h
    include/tree/C/treelib_api.h
    include/tree/C/treelib_extern.h
    include/tree/C/vector.h

    include/tree/AVLTree.hpp
    include/tree/AVLTreeImpl.hpp

    include/tree/FlatAVLTree.hpp
    include/tree/FlatAVLTreeImpl.hpp
)

set(PRIVATE_HEADERS
    include/tree/internal/AVLTreeGeneric.hpp
    include/tree/internal/AVLTreeGeneric.tpp
    include/tree/internal/AVLTreeRotateState.hpp

    include/tree/internal/binaryTreeGeneric.hpp
    include/tree/internal/binaryTreeGeneric.tpp
    
    include/tree/internal/FlatAVLGenericAccessor.hpp
    include/tree/internal/FlatAVLTreeGeneric.hpp
    include/tree/internal/FlatAVLTreeGeneric.tpp
    include/tree/internal/FlatAVLTreeGenericIterators.hpp
    include/tree/internal/floatingPoint.hpp

    include/tree/internal/treelib_util.hpp
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
target_sources(${PROJECT_NAME} PUBLIC ${PUBLIC_HEADERS})
target_sources(${PROJECT_NAME} INTERFACE 
    FILE_SET project_internal_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES ${PRIVATE_HEADERS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    WORKSPACE_CONFIG::ProjectDefaultConfig
)

# The minimum C/C++ versions required for the library
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)
set_source_files_properties(${PRIVATE_HEADERS} PROPERTIES LANGUAGE CXX)
set_target_properties(${PROJECT_NAME} PROPERTIES 
    VERIFY_INTERFACE_HEADER_SETS ON
)
if(BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        C_VISIBILITY_PRESET       hidden
        CXX_VISIBILITY_PRESET     hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

