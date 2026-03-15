cmake_minimum_required(VERSION 3.14)


option(ENABLE_SANITIZER_ADDRESS 
    "Enable AddressSanitizer For This Library" 
    OFF
)

option(ENABLE_SANITIZER_UNDEFINED 
    "Enable UndefinedBehaviorSanitizer For This Library" 
    OFF
)

option(ENABLE_SANITIZER_MEMORY
    "Enable MemorySanitizer For This Library" 
    OFF
)


add_library(workspace_cfg_sanitizers INTERFACE)
add_library(WORKSPACE_CONFIG::Sanitizers ALIAS workspace_cfg_sanitizers)
if(${ENABLE_SANITIZER_ADDRESS})
    target_compile_options(workspace_cfg_sanitizers INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(workspace_cfg_sanitizers INTERFACE -fsanitize=address)
endif()
if(${ENABLE_SANITIZER_UNDEFINED})
    target_compile_options(workspace_cfg_sanitizers INTERFACE -fsanitize=undefined)
    target_link_options(workspace_cfg_sanitizers INTERFACE -fsanitize=undefined)
endif()
if(${ENABLE_SANITIZER_MEMORY})
    target_compile_options(workspace_cfg_sanitizers INTERFACE -fno-omit-frame-pointer -fsanitize=memory)
    target_link_options(workspace_cfg_sanitizers INTERFACE -fno-omit-frame-pointer -fsanitize=memory)
endif()
