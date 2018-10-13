cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_info VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_info STATIC)

ltjs_add_defaults(ltjs_lib_info)

target_include_directories(
	ltjs_lib_info
	PRIVATE
		.
		../../../sdk/inc
		../../../sdk/inc/libltinfo
		../../kernel/src
		../../shared/src
)

target_sources(
	ltjs_lib_info
	PRIVATE
		../../../sdk/inc/libltinfo/iltoutputredir.h
		../../../sdk/inc/libltinfo.h
		outputredir.h
)

target_sources(
	ltjs_lib_info
	PRIVATE
		outputredir.cpp
		sys/win32/outputredir_win32.cpp
)
