
cmake_minimum_required (VERSION 3.15..4.0)
project(
    workspace
    VERSION 0.00.1
    DESCRIPTION "Various Types Of Trees :)"
    LANGUAGES ASM C
)
set(EXECUTABLE_NAME ${PROJECT_NAME})


# Taken from: https://cliutils.gitlab.io/modern-cmake/chapters/projects/submodule.html
function(UPDATE_SUBMODULES_ON_CMAKE_CONFIGURE SUBMODULE_CMAKE_TXT_PATH)
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
        # Update submodules as needed
        option(GIT_SUBMODULE "Check submodules during build" ON)
        if(GIT_SUBMODULE)
            message(STATUS "Submodule update") # see: https://stackoverflow.com/a/21195182
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE GIT_SUBMOD_RESULT
            )
            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                message(FATAL_ERROR "git submodule update --init --recursive --remote failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
            endif()
        endif()
    endif()

    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/${SUBMODULE_CMAKE_TXT_PATH})
        message(${PROJECT_SOURCE_DIR}/${SUBMODULE_CMAKE_TXT_PATH})
        message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
    endif()
endfunction() 


update_submodules_on_cmake_configure("dependencies/benchmark/CMakeLists.txt")
update_submodules_on_cmake_configure("dependencies/cmocka/CMakeLists.txt")
update_submodules_on_cmake_configure("dependencies/googletest/CMakeLists.txt")
update_submodules_on_cmake_configure("dependencies/util2/CMakeLists.txt")


option(ENABLE_SANITIZER_ADDRESS 
    "Enable AddressSanitizer" 
    OFF
)

option(ENABLE_SANITIZER_UNDEFINED 
    "Enable UndefinedBehaviorSanitizer" 
    OFF
)

option(ENABLE_SANITIZER_MEMORY
    "Enable MemorySanitizer" 
    OFF
)

OPTION(TREELIB_BUILD_TESTS
    "Create Build, Run Targets for Testing of tree" 
    OFF
)




# Using <config> will generate in a multi-generator-dependant way, which is undesirable
# I'd rather choose it myself using a script.
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

# makes sure all dynamic/static/executable objects sit in the same place
set(CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CUSTOM_BUILD_OUTPUT_DIRECTORY})

# prevent installing to system directories. 
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_EXTENSIONS          OFF)
if(NOT DEFINED CMAKE_C_STANDARD)
    set(CMAKE_C_STANDARD 11)
endif()
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

# default build type unless user defined
if(NOT DEFINED CMAKE_BUILD_TYPE)
    # set(cmake_build_type_undefined 1)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "CMake build type" FORCE)
endif()


find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_ASM_COMPILER_LAUNCHER  "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER    "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER  "${CCACHE_PROGRAM}")
    # set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()




add_library(workspace_cfg_sanitizers INTERFACE)
add_library(WORKSPACE_CONFIG::Sanitizers ALIAS workspace_cfg_sanitizers)
if(${ENABLE_SANITIZER_ADDRESS})
    # Interface-Target Specific
    target_compile_options(workspace_cfg_sanitizers INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(workspace_cfg_sanitizers INTERFACE -fsanitize=address)

    # Whole project Specific
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    if(MSVC)
        add_compile_options(/Zi /fsanitize=address)
        add_link_options(/INCREMENTAL:NO)
    endif()
endif()

if(${ENABLE_SANITIZER_UNDEFINED})
    # Interface-Target Specific
    target_compile_options(workspace_cfg_sanitizers INTERFACE -fsanitize=undefined)
    target_link_options(workspace_cfg_sanitizers INTERFACE -fsanitize=undefined)

    # Whole project Specific
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    if(MSVC)
        add_compile_options(/Zi /fsanitize=address)
        add_link_options(/INCREMENTAL:NO)
    endif()
endif()

if(${ENABLE_SANITIZER_MEMORY})
    target_compile_options(workspace_cfg_sanitizers INTERFACE -fno-omit-frame-pointer -fsanitize=memory)
    target_link_options(workspace_cfg_sanitizers INTERFACE -fno-omit-frame-pointer -fsanitize=memory)
endif()




add_subdirectory(tree)
# Thank you to:
# https://github.com/cld-gh/cpp-with-gtest-example
# https://stackoverflow.com/questions/50468620/what-does-enable-testing-do-in-cmake
if(${TREELIB_BUILD_TESTS})
    # enable_testing()
    add_subdirectory(tests)
    add_subdirectory(benchmarks)
    add_subdirectory(dependencies/util2)
    add_subdirectory(dependencies/googletest)
    add_subdirectory(dependencies/benchmark)
    add_subdirectory(dependencies/cmocka)
endif()

