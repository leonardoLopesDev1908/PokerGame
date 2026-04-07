add_executable(Server "src/Server.cpp" "src/AppServer.cpp" "include/Server.h")

target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Server PRIVATE game)
target_link_libraries(Server PRIVATE net_common)
