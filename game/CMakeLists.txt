file(GLOB GAME_SOURCES 
	"Game.cpp"
)

add_library(game STATIC ${GAME_SOURCES})

target_include_directories(game PRIVATE ../netcommon)
target_include_directories(game PRIVATE ../evaluator)
target_include_directories(game PRIVATE ../common)

target_link_libraries(game PRIVATE evaluator)