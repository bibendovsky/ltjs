cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_reg_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_reg_mgr STATIC)

ltjs_add_defaults(ltjs_lib_reg_mgr)

target_include_directories(
	ltjs_lib_reg_mgr
	PRIVATE
		../lith
)

target_sources(
	ltjs_lib_reg_mgr
	PRIVATE
		regmgr.h
)

target_sources(
	ltjs_lib_reg_mgr
	PRIVATE
		regmgr.cpp
)
