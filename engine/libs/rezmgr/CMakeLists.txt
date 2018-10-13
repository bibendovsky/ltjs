cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_rez_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_rez_mgr STATIC)

ltjs_add_defaults(ltjs_lib_rez_mgr)

target_include_directories(
	ltjs_lib_rez_mgr
	PRIVATE
		../../../libs/lith
		../../sdk/inc
)

target_sources(
	ltjs_lib_rez_mgr
	PRIVATE
		rezfile.h
		rezhash.h
		rezmgr.h
		reztypes.h
		rezutils.h
)

target_sources(
	ltjs_lib_rez_mgr
	PRIVATE
		rezfile.cpp
		rezhash.cpp
		rezmgr.cpp
		rezutils.cpp
)
