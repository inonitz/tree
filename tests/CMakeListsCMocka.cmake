cmake_minimum_required(VERSION 3.14)

set(CMOCKA_TEST_TARGET_NAME
    ${PROJECT_NAME}_cmocka
)

set(CMOCKA_SOURCES
    source/main.c
    source/AVLTreeCTest.c
    source/QueueTest.c
    source/StackTest.c
    source/VectorTest.c
)

set(CMOCKA_HEADERS
    include/AVLTreeCTest.h
    include/QueueTest.h
    include/StackTest.h
    include/VectorTest.h
)

set(CMOCKA_STRUCTURES_TEST_GROUP
    "AVLTree"
    "Queue"
    "Stack"
    "Vector"
)


add_executable(${CMOCKA_TEST_TARGET_NAME})
target_include_directories(${CMOCKA_TEST_TARGET_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_sources(${CMOCKA_TEST_TARGET_NAME} PRIVATE ${CMOCKA_SOURCES})
target_sources(${CMOCKA_TEST_TARGET_NAME} PRIVATE ${CMOCKA_HEADERS})
target_link_libraries(
    ${CMOCKA_TEST_TARGET_NAME} PRIVATE
    TREELIB::treelib
    UTIL2::util2
    cmocka::cmocka
    WORKSPACE_CONFIG::Sanitizers
)
target_compile_definitions(${CMOCKA_TEST_TARGET_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS=1
)
# target_compile_definitions(${CMOCKA_TEST_TARGET_NAME} PRIVATE
#     $<$<BOOL:${BUILD_SHARED_LIBS}>:GTEST_LINKED_AS_SHARED_LIBRARY=1>
# )
target_compile_features(${CMOCKA_TEST_TARGET_NAME} PUBLIC c_std_11)


# define all tests, running the same main.c, under different env vars
foreach(group ${CMOCKA_STRUCTURES_TEST_GROUP})
    add_test(
        NAME "CMocka_${group}"
        COMMAND ./${CMOCKA_TEST_TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
    )
    # TEST_FILTER only filters by test name, not the group
    set_tests_properties("CMocka_${group}" PROPERTIES
        ENVIRONMENT "CMOCKA_TEST_FILTER=${group}*"
        LABELS "unit_test"
    )
endforeach()


# Incase I need to debug specific tests
add_custom_target(debug_${CMOCKA_TEST_TARGET_NAME}
    COMMAND ./${CMOCKA_TEST_TARGET_NAME}
    DEPENDS ${CMOCKA_TEST_TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
    USES_TERMINAL
)
