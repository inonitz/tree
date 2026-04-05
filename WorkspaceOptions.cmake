cmake_minimum_required(VERSION 3.24)
include(CheckCXXCompilerFlag)
include(CheckIPOSupported)


set(SANITIZER_ADDRESS_INTERFACE_NAME
    workspace_cfg_address_sanitizer_interface
)
set(SANITIZER_UNDEFINED_BEHAVIOUR_INTERFACE_NAME
    workspace_cfg_ubsan_sanitizer_interface
)
set(SANITIZER_MEMORY_INTERFACE_NAME
    workspace_cfg_memory_sanitizer_interface
)
set(SANITIZER_AGGREGATE_INTERFACE_NAME
    workspace_cfg_sanitizers
)

set(LINK_TIME_OPT_INTERFACE_NAME
    workspace_cfg_lto_ipo
)

set(PROJECT_CONFIG_INTERFACE_NAME
    generic_project_cfg
)


check_cxx_compiler_flag(-fsanitize=address COMPILER_HAS_ASAN_NOT_MSVC)
check_cxx_compiler_flag(/fsanitize=address COMPILER_HAS_ASAN_MSVC)

check_cxx_compiler_flag(-fsanitize=undefined COMPILER_HAS_UBSAN_NOT_MSVC)
check_cxx_compiler_flag(/fsanitize=undefined COMPILER_HAS_UBSAN_MSVC)

check_cxx_compiler_flag(-fsanitize=memory COMPILER_HAS_MEMSAN_NOT_MSVC)
check_cxx_compiler_flag(/fsanitize=memory COMPILER_HAS_MEMSAN_MSVC)

check_ipo_supported(
    RESULT LTO_SUPPORT_CHECK_RESULT 
    OUTPUT LTO_SUPPORT_CHECK_OUTPUT
)




# Configure Interface Projects For Different Sanitizers
add_library(${SANITIZER_ADDRESS_INTERFACE_NAME} INTERFACE)
add_library(WORKSPACE_CONFIG::AddressSanitizerInterface ALIAS ${SANITIZER_ADDRESS_INTERFACE_NAME})

if(COMPILER_HAS_ASAN_NOT_MSVC)
    target_compile_options(${SANITIZER_ADDRESS_INTERFACE_NAME} INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${SANITIZER_ADDRESS_INTERFACE_NAME} INTERFACE -fsanitize=address)

elseif(MSVC AND COMPILER_HAS_ASAN_MSVC)
    target_compile_options(${SANITIZER_ADDRESS_INTERFACE_NAME} INTERFACE /fsanitize=address /Zi /Oy-)
    target_link_options(${SANITIZER_ADDRESS_INTERFACE_NAME} INTERFACE /fsanitize=address)

else()
    message(WARNING "Address Sanitizer (-fsanitize=address OR /fsanitize=address) not supported by this compiler\n")
endif()


add_library(${SANITIZER_UNDEFINED_BEHAVIOUR_INTERFACE_NAME} INTERFACE)
add_library(WORKSPACE_CONFIG::UBSanInterface ALIAS ${SANITIZER_UNDEFINED_BEHAVIOUR_INTERFACE_NAME})

if(COMPILER_HAS_UBSAN_NOT_MSVC)
    target_compile_options(${SANITIZER_UNDEFINED_BEHAVIOUR_INTERFACE_NAME} INTERFACE -fsanitize=undefined)
    target_link_options(${SANITIZER_UNDEFINED_BEHAVIOUR_INTERFACE_NAME} INTERFACE -fsanitize=undefined)

elseif(MSVC AND COMPILER_HAS_UBSAN_MSVC)
    target_compile_options(${SANITIZER_UNDEFINED_BEHAVIOUR_INTERFACE_NAME} INTERFACE /fsanitize=undefined)
    target_link_options(${SANITIZER_UNDEFINED_BEHAVIOUR_INTERFACE_NAME} INTERFACE /fsanitize=undefined)

else()
    message(WARNING "Undefined Behaviour Sanitizer (-fsanitize=undefined OR /fsanitize=undefined) not supported by this compiler\n")
endif()


add_library(${SANITIZER_MEMORY_INTERFACE_NAME} INTERFACE)
add_library(WORKSPACE_CONFIG::MemorySanitizerInterface ALIAS ${SANITIZER_MEMORY_INTERFACE_NAME})

if(COMPILER_HAS_MEMSAN_NOT_MSVC)
    target_compile_options(${SANITIZER_MEMORY_INTERFACE_NAME} INTERFACE -fno-omit-frame-pointer -fsanitize=memory)
    target_link_options(${SANITIZER_MEMORY_INTERFACE_NAME} INTERFACE -fsanitize=memory)

elseif(MSVC AND COMPILER_HAS_MEMSAN_MSVC)
    target_compile_options(${SANITIZER_MEMORY_INTERFACE_NAME} INTERFACE /fsanitize=memory /Zi /Oy-)
    target_link_options(${SANITIZER_MEMORY_INTERFACE_NAME} INTERFACE /fsanitize=memory)

else()
    message(WARNING "Memory Sanitizer (-fsanitize=memory OR /fsanitize=memory) not supported by this compiler\n")
endif()


# declare_optional_interface_library_different_sanitizers()
function(DECLARE_OPTIONAL_INTERFACE_LIBRARY_DIFFERENT_SANITIZERS)
    # Final Sanitizer Interface
    add_library(${SANITIZER_AGGREGATE_INTERFACE_NAME} INTERFACE)
    add_library(WORKSPACE_CONFIG::Sanitizers ALIAS ${SANITIZER_AGGREGATE_INTERFACE_NAME})

    target_link_libraries(${SANITIZER_AGGREGATE_INTERFACE_NAME} INTERFACE 
        $<$<BOOL:${ENABLE_SANITIZER_ADDRESS}>:WORKSPACE_CONFIG::AddressSanitizerInterface>
        $<$<BOOL:${ENABLE_SANITIZER_UNDEFINED}>:WORKSPACE_CONFIG::UBSanInterface>
        $<$<BOOL:${ENABLE_SANITIZER_MEMORY}>:WORKSPACE_CONFIG::MemorySanitizerInterface>
    )
endfunction()


# declare_optional_interface_library_lto()
function(DECLARE_OPTIONAL_INTERFACE_LIBRARY_LTO)
    # Configure Interface Project for Link-Time/Interprocedural Optimization
    add_library(${LINK_TIME_OPT_INTERFACE_NAME} INTERFACE)
    add_library(WORKSPACE_CONFIG::LTO ALIAS ${LINK_TIME_OPT_INTERFACE_NAME})

    set_target_properties(${LINK_TIME_OPT_INTERFACE_NAME} PROPERTIES 
        INTERFACE_INTERPROCEDURAL_OPTIMIZATION "$<AND:
            $<BOOL:${ENABLE_LINK_TIME_OPTIMIZATION}>,
            $<BOOL:${LTO_SUPPORT_CHECK_RESULT}>
        >"
    )
    if(NOT LTO_SUPPORT_CHECK_RESULT)
        message(WARNING "IPO / LTO Not Supported:\nMessage:${LTO_SUPPORT_CHECK_OUTPUT}\n")
    endif()
endfunction()


# Project Configuration that should be used
# declare_optional_interface_default_project_configuration()
function(DECLARE_OPTIONAL_INTERFACE_DEFAULT_PROJECT_CONFIGURATION)
    add_library(${PROJECT_CONFIG_INTERFACE_NAME} INTERFACE)
    add_library(WORKSPACE_CONFIG::ProjectDefaultConfig ALIAS ${PROJECT_CONFIG_INTERFACE_NAME})

    target_link_libraries(${PROJECT_CONFIG_INTERFACE_NAME} INTERFACE
        WORKSPACE_CONFIG::Sanitizers
        WORKSPACE_CONFIG::LTO
    )

    target_compile_options(${PROJECT_CONFIG_INTERFACE_NAME} INTERFACE
        $<
            $<OR:
            $<C_COMPILER_ID:GNU>, $<C_COMPILER_ID:Clang>, $<C_COMPILER_ID:AppleClang>,
            $<CXX_COMPILER_ID:GNU>, $<CXX_COMPILER_ID:Clang>, $<CXX_COMPILER_ID:AppleClang>>:
            
            $< $<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:
                $<$<CONFIG:Debug,RelWithDebInfo>:-Werror>
                -Wall
                -Wextra
                -Wshadow
                -Wfloat-equal
                -Wconversion
                -Wformat=2
                -pedantic
                -pedantic-errors
                -fstrict-aliasing
                -fcolor-diagnostics
            >

            $<$<COMPILE_LANGUAGE:CXX>:
                -Wold-style-cast
                -Wnon-virtual-dtor
                -Woverloaded-virtual
            >

            $<$<COMPILE_LANGUAGE:C>:
                -Wmissing-prototypes
                -Wstrict-prototypes
            >
        >

        $<
            $<OR:
            $<C_COMPILER_ID:MSVC>, $<CXX_COMPILER_ID:MSVC>>:
            /W4
            $<$<CONFIG:Debug,RelWithDebInfo>:/WX>        
            $<$<COMPILE_LANGUAGE:CXX>:/permissive->
        >
    )

    set_target_properties(${PROJECT_CONFIG_INTERFACE_NAME} PROPERTIES
        INTERFACE_C_STANDARD                11
        INTERFACE_CXX_STANDARD              17
        INTERFACE_CXX_STANDARD_REQUIRED     ON
        INTERFACE_CXX_EXTENSIONS            OFF    # Disables compiler-specific extensions 
        INTERFACE_POSITION_INDEPENDENT_CODE ON     # Cross-platform -fPIC
        INTERFACE_CXX_VISIBILITY_PRESET     hidden # Cross-platform -fvisibility=hidden
        INTERFACE_VISIBILITY_INLINES_HIDDEN ON     # Cross-platform -fvisibility-inlines-hidden
    )
endfunction()
