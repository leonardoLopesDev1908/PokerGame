add_library(net_common INTERFACE)

find_package(asio CONFIG REQUIRED)

target_include_directories(net_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(net_common INTERFACE asio::asio)
target_precompile_headers(net_common INTERFACE net_common.h)