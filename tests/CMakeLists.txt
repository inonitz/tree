cmake_minimum_required(VERSION 3.14)
project(test_treelib
    VERSION 0.0.1
    DESCRIPTION "C/C++ Programs to Test Functionality of tree"
    LANGUAGES C CXX
)


set(SOURCES
    source/AVLTreeCTest.c
    source/AVLTreeTest.cpp
    source/AVLTreeGenericRecursiveTest.cpp
    source/AVLTreeGenericTest.cpp
    source/AVLTreeTest.cpp
    source/main.cpp
    source/QueueTest.c
    source/StackTest.c
    source/VectorTest.c
)


set(HEADERS
    include/AVLTreeCTest.h
    include/AVLTreeTest.hpp
    include/AVLTreeGenericRecursiveTest.hpp
    include/AVLTreeGenericTest.hpp
    include/QueueTest.h
    include/StackTest.h
    include/VectorTest.h
)




# Thank you very much to:
# https://gbaudic.github.io/2021/07/14/using_sdl2_with_gtest.html
add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${HEADERS})
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    TREELIB::treelib
    UTIL2::util2
    GTest::gtest_main
    cmocka::cmocka
    WORKSPACE_CONFIG::Sanitizers
)
# gtest_discover_tests(${PROJECT_NAME})
# add_test(NAME ${PROJECT_NAME} command ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS=1
)

# target_compile_definitions(${PROJECT_NAME} PRIVATE 
#     $<$<BOOL:${BUILD_SHARED_LIBS}>:GTEST_LINKED_AS_SHARED_LIBRARY=1>
# )
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)


add_custom_target(run_${PROJECT_NAME}
    COMMAND ./${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)


