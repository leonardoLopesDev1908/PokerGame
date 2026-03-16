
add_executable (Client "NetClient.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Client PROPERTY CXX_STANDARD 20)
endif()

find_package(asio CONFIG REQUIRED)

target_include_directories(Client PRIVATE ../netcommon)
target_include_directories(Client PRIVATE ../common)
target_link_libraries(Client PRIVATE asio::asio)
