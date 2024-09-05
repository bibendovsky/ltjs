cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_lib_reg_mgr VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../lith
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		regmgr.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		regmgr.cpp
)
