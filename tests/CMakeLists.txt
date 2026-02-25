cmake_minimum_required(VERSION 3.14)
include(GoogleTest)


project(Testingtree
    VERSION 0.0.1
    DESCRIPTION "C/C++ Programs to Test Functionality of tree"
    LANGUAGES C CXX
)


set(SOURCES
    source/main.cpp
    source/AVLTree.cpp
    source/AVLTree.hpp
    source/binaryTreePrettyPrint.cpp
    source/binaryTreePrettyPrint.hpp

)


# Thank you very much to:
# https://gbaudic.github.io/2021/07/14/using_sdl2_with_gtest.html
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    TREELIB::treelib
    util2
    GTest::gtest_main
)
gtest_discover_tests(${PROJECT_NAME})


# The minimum C/C++ versions required for the library
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)


add_custom_target(run_${PROJECT_NAME}
    COMMAND ./${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY}
)


