
add_executable (NetClient "NetClient.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NetClient PROPERTY CXX_STANDARD 20)
endif()

find_package(asio CONFIG REQUIRED)

target_include_directories(NetClient PRIVATE ../NetCommon)
target_link_libraries(NetClient PRIVATE asio::asio)
