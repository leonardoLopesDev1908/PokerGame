add_library(net_deque INTERFACE)

target_include_directories(net_deque INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
if(MSVC)
    add_compile_options(/utf-8)
endif()