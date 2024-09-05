cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_lib_info VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../../sdk/inc
		../../../sdk/inc/libltinfo
		../../kernel/src
		../../shared/src
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../sdk/inc/libltinfo/iltoutputredir.h
		../../../sdk/inc/libltinfo.h
		outputredir.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		outputredir.cpp
		sys/win32/outputredir_win32.cpp
)
