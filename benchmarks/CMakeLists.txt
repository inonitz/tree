cmake_minimum_required(VERSION 3.14)
include(GoogleTest)
include(ProcessorCount)


project(benchmark_treelib
    VERSION 0.0.1
    DESCRIPTION "C/C++ Programs to Measure Performance of Various tree Implementations & Friends"
    LANGUAGES C CXX
)


set(SOURCES
    source/AVLTreeCBench.cpp
    source/AVLTreeBench.cpp
    source/AVLTreeGenericBench.cpp
    source/AVLTreeGenericRecursiveBench.cpp
    source/FlatAVLTreeGenericBench.cpp
    source/StdSetBench.cpp
    source/StdUnorderedMapBench.cpp
)

set(SOURCES_BENCHMARK_MAIN
    source/main.cpp
)

set(BENCHMARK_NAMES
    ""
)

set(NPROC 0)
ProcessorCount(NPROC)
math(EXPR NPROC "${NPROC}/2")


add_executable(${PROJECT_NAME} ${SOURCES} ${SOURCES_BENCHMARK_MAIN})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    TREELIB::treelib
    UTIL2::util2
    benchmark::benchmark
    # benchmark::benchmark_main
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)


# Define a benchmark for each specific .cpp file, s.t they can be run concurrently
# (Even if that defeats the point of benchmarking, this shit takes a minimum of 2 Hours!)
foreach(SOURCE_PATH ${SOURCES})
    get_filename_component(FILE_BASE ${SOURCE_PATH} NAME_WLE)
    list(APPEND BENCHMARK_NAMES ${FILE_BASE})
endforeach()
foreach(BENCH_NAME ${BENCHMARK_NAMES})
    add_test(
        NAME "bench_${BENCH_NAME}"
        COMMAND ./${PROJECT_NAME}
            "--benchmark_out=results_${BENCH_NAME}.json"
            "--benchmark_out_format=json"
            "--benchmark_filter=BM_${BENCH_NAME}*"
        WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
    )
    set_tests_properties("bench_${BENCH_NAME}" PROPERTIES
        PROCESSOR_AFFINITY ON
        LABELS "benchmark"
        # RUN_SERIAL TRUE
    )
endforeach()




add_custom_target(run_${PROJECT_NAME}
    COMMAND ${CMAKE_CTEST_COMMAND}
        -L benchmark
        --verbose -j${NPROC}
        --progress
        --output-on-failure
        --test-output-size-passed 262144
    USES_TERMINAL
    COMMENT "Running unit tests in parallel..."
)
add_custom_target(debug_${PROJECT_NAME}
    COMMAND ./${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
    USES_TERMINAL
    COMMENT "Running Benchmarking Debug Target..."
)
