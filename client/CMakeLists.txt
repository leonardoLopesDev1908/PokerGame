add_executable (Client 
		"src/Client.cpp" 
		"include/Client.h" 
		"src/AppClient.cpp"
)

target_include_directories(Client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Client PUBLIC net_common)
target_link_libraries(Client PUBLIC common)
