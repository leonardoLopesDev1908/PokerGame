add_executable(NetServer "NetServer.cpp")

find_package(asio CONFIG REQUIRED)

target_include_directories(NetServer PRIVATE ../NetCommon)
target_link_libraries(NetServer PRIVATE asio::asio)