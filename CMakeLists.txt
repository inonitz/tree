cmake_minimum_required (VERSION 3.24)
project(
    workspace
    VERSION 0.00.1
    DESCRIPTION "Various Types Of Trees :)"
    LANGUAGES ASM C CXX
)


# Functions/Utilities that should run in the top-most of the file
include(SubmoduleUpdate.cmake)
include(UseCCache.cmake)
include(OutputDir.cmake)
include(WorkspaceOptions.cmake)


# Workspace/Project-Wide Specific Configurations
update_submodules_on_cmake_configure("dependencies")
use_ccache_program_if_exists()
define_aggregate_output_artifact_directory()


# Workspace/Project-Wide Options
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

option(ENABLE_LINK_TIME_OPTIMIZATION
    "Enable Link-Time/Interprocedural Optimization (LTO/IPO)"
    OFF
)

OPTION(TREELIB_BUILD_TESTS
    "Create Build, Run Targets for Testing of the whole tree project"
    OFF
)


if(ENABLE_SANITIZER_ADDRESS AND ENABLE_SANITIZER_MEMORY)
    message(FATAL_ERROR "ASan and MSan are mutually exclusive. Please choose only one\n")
endif()


# Declare interface libraries 
declare_optional_interface_library_different_sanitizers()
declare_optional_interface_library_lto()
declare_optional_interface_default_project_configuration()


# The actual Project Content
add_subdirectory(tree)
# Thank you to:
# https://github.com/cld-gh/cpp-with-gtest-example
# https://stackoverflow.com/questions/50468620/what-does-enable-testing-do-in-cmake
if(${TREELIB_BUILD_TESTS})
    enable_testing()
    add_subdirectory(dependencies/util2)
    add_subdirectory(dependencies/googletest)
    add_subdirectory(dependencies/benchmark)
    add_subdirectory(dependencies/cmocka)
    add_subdirectory(benchmarks)
    add_subdirectory(tests)
endif()
