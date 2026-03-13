add_library(common INTERFACE)

target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
if(MSVC)
    add_compile_options(/utf-8)
endif()
