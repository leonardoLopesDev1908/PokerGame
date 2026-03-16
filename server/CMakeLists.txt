add_executable(Server "NetServer.cpp")

find_package(asio CONFIG REQUIRED)

target_include_directories(Server PRIVATE ../netcommon)
target_include_directories(Server PRIVATE ../common)
target_link_libraries(Server PRIVATE asio::asio)