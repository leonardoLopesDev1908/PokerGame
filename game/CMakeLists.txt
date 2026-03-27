file(GLOB GAME_SOURCES 
	"Game.cpp",
	"Player.cpp"
)

add_library(game STATIC ${GAME_SOURCES})

find_package(asio CONFIG REQUIRED)

target_include_directories(game PRIVATE ../netcommon)
target_include_directories(game PRIVATE ../evaluator)
target_include_directories(game PRIVATE ../common)

target_link_libraries(game PRIVATE evaluator)
target_link_libraries(game PRIVATE asio::asio)