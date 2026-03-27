cmake_minimum_required(VERSION 3.14)
include(GoogleTest)

set(GOOGLE_TEST_TARGET_NAME
    ${PROJECT_NAME}_gtest
)

set(GOOGLE_TEST_DEBUG_TARGET_NAME
    ${PROJECT_NAME}_gtest_serial
)


set(GTEST_SOURCES
    source/AVLTreeTest.cpp
    source/AVLTreeGenericRecursiveTest.cpp
    source/AVLTreeGenericTest.cpp
    source/AVLTreeTest.cpp
    source/FlatAVLTreeGenericTest.cpp
)

set(GTEST_HEADERS
    include/AVLTreeGenericRecursiveTest.hpp
    include/AVLTreeGenericTest.hpp
    include/AVLTreeTest.hpp
    include/FlatAVLTreeGenericTest.hpp
)

set(GTEST_DEBUG_MAIN
    source/main.cpp
)


# Build Target for Tests, uses gtest_main for executing the tests.
# This target is run using ctest, which means that they execute concurrently
# to speed up testing time
add_executable(${GOOGLE_TEST_TARGET_NAME})
target_include_directories(${GOOGLE_TEST_TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_sources(${GOOGLE_TEST_TARGET_NAME} PRIVATE ${GTEST_SOURCES})
target_sources(${GOOGLE_TEST_TARGET_NAME} PRIVATE ${GTEST_HEADERS})
target_link_libraries(
    ${GOOGLE_TEST_TARGET_NAME} PRIVATE
    TREELIB::treelib
    UTIL2::util2
    GTest::gtest
    GTest::gtest_main
    WORKSPACE_CONFIG::Sanitizers
)
target_compile_definitions(${GOOGLE_TEST_TARGET_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS=1
)
# target_compile_definitions(${GOOGLE_TEST_TARGET_NAME} PRIVATE
#     $<$<BOOL:${BUILD_SHARED_LIBS}>:GTEST_LINKED_AS_SHARED_LIBRARY=1>
# )
target_compile_features(${GOOGLE_TEST_TARGET_NAME} PUBLIC cxx_std_17)
target_compile_features(${GOOGLE_TEST_TARGET_NAME} PUBLIC c_std_11)


# Build Target for Debugging, uses a main.cpp for executing the tests serially.
add_executable(${GOOGLE_TEST_DEBUG_TARGET_NAME})
target_include_directories(${GOOGLE_TEST_DEBUG_TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_sources(${GOOGLE_TEST_DEBUG_TARGET_NAME} PRIVATE ${GTEST_SOURCES})
target_sources(${GOOGLE_TEST_DEBUG_TARGET_NAME} PRIVATE ${GTEST_HEADERS})
target_sources(${GOOGLE_TEST_DEBUG_TARGET_NAME} PRIVATE ${GTEST_DEBUG_MAIN})
target_link_libraries(
    ${GOOGLE_TEST_DEBUG_TARGET_NAME} PRIVATE
    TREELIB::treelib
    UTIL2::util2
    GTest::gtest
    WORKSPACE_CONFIG::Sanitizers
)
target_compile_definitions(${GOOGLE_TEST_DEBUG_TARGET_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS=1
)
# target_compile_definitions(${GOOGLE_TEST_TARGET_NAME} PRIVATE
#     $<$<BOOL:${BUILD_SHARED_LIBS}>:GTEST_LINKED_AS_SHARED_LIBRARY=1>
# )
target_compile_features(${GOOGLE_TEST_DEBUG_TARGET_NAME} PUBLIC cxx_std_17)
target_compile_features(${GOOGLE_TEST_DEBUG_TARGET_NAME} PUBLIC c_std_11)




# Detect All Written Tests using the GoogleTest Module
gtest_discover_tests(${GOOGLE_TEST_TARGET_NAME}
    TEST_LIST DISCOVERED_GTESTS
    PROPERTIES
    LABELS "unit_test"
    ENVIRONMENT "GTEST_COLOR=yes"
)

# Add a Serially running Debug Target Incase I need to debug specific/newly registered tests
add_custom_target(debug_${PROJECT_NAME}
    COMMAND ./${GOOGLE_TEST_DEBUG_TARGET_NAME}
    DEPENDS ${GOOGLE_TEST_DEBUG_TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
    USES_TERMINAL
)
