file(GLOB GAME_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(game STATIC ${GAME_SOURCES})

target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(game PUBLIC evaluator)
target_link_libraries(game PUBLIC net_common)
target_link_libraries(game PUBLIC common)