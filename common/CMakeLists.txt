add_library(common INTERFACE)

target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(common INTERFACE ../game)

if(MSVC)
    target_compile_options(common INTERFACE utf-8)
endif()