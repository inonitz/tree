cmake_minimum_required(VERSION 3.14)
include(GoogleTest)


project(benchmark_treelib
    VERSION 0.0.1
    DESCRIPTION "C/C++ Programs to Measure Performance of Various tree Structures"
    LANGUAGES C CXX
)


set(SOURCES
    source/main.cpp
    source/AVLTreeCBench.cpp
    source/AVLTreeBench.cpp
    source/AVLTreeGenericBench.cpp
    source/AVLTreeGenericRecursiveBench.cpp
    source/StdSetBench.cpp
    source/UnorderedMapBench.cpp
)


add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    TREELIB::treelib
    UTIL2::util2
    benchmark::benchmark
    benchmark::benchmark_main
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)


add_custom_target(run_${PROJECT_NAME}
    COMMAND ./${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)


add_test(
    NAME bench_avltree_cpp
    COMMAND ./${PROJECT_NAME} --benchmark_out=results_avltree_cpp.json --benchmark_out_format=json --benchmark_filter=BM_AVLTree*
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)

add_test(
    NAME bench_avltree_cpp_generic
    COMMAND ./${PROJECT_NAME} --benchmark_out=results_avltree_cpp_generic.json --benchmark_out_format=json --benchmark_filter=BM_GenericAVLTree* 
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)

add_test(
    NAME bench_avltree_cpp_generic_recursive
    COMMAND ./${PROJECT_NAME} --benchmark_out=results_avltree_cpp_generic_recursive.json --benchmark_out_format=json --benchmark_filter=BM_GenericRecursiveAVLTree* 
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)

add_test(
    NAME bench_avltree_c_generic
    COMMAND ./${PROJECT_NAME} --benchmark_out=results_avltree_c_generic.json --benchmark_out_format=json --benchmark_filter=BM_CAVLTree* 
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)

add_test(
    NAME bench_std_set_generic
    COMMAND ./${PROJECT_NAME} --benchmark_out=results_std_set_generic.json --benchmark_out_format=json --benchmark_filter=BM_StdSet*
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)

add_test(
    NAME bench_std_unordered_set
    COMMAND ./${PROJECT_NAME} --benchmark_out=results_std_unordered_set.json --benchmark_out_format=json --benchmark_filter=BM_StdUnorderedMap*
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)

