cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		include/ltjs_exception.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		src/ltjs_exception.cpp
)
