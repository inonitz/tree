cmake_minimum_required(VERSION 3.14)
project(test_treelib
    VERSION 0.0.1
    DESCRIPTION "C/C++ Programs to Test Functionality of tree"
    LANGUAGES C CXX
)

include(ProcessorCount)
include(CMakeListsGTest.cmake)
include(CMakeListsCMocka.cmake)


set(NPROC 0)
ProcessorCount(NPROC)
math(EXPR NPROC "${NPROC}/2")


add_custom_target(${PROJECT_NAME}_run_all
    COMMAND ${CMAKE_CTEST_COMMAND}
        -L unit_test
        --verbose -j${NPROC}
        --progress
        --output-on-failure
        --test-output-size-passed 262144
    USES_TERMINAL
    COMMENT "Running unit tests in parallel..."
)
