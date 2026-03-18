file(GLOB EVALUATOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/omp/*.cpp
)

add_library(evaluator STATIC ${EVALUATOR_SOURCES})

target_include_directories(evaluator PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/omp
	${CMAKE_CURRENT_SOURCE_DIR}/libdivide
)


