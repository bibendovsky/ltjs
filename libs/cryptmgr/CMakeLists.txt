cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_crypt_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_crypt_mgr STATIC)

ltjs_add_defaults(ltjs_lib_crypt_mgr)

target_sources(
	ltjs_lib_crypt_mgr
	PRIVATE
		blowfish.h
		cryptmgr.h
)

target_sources(
	ltjs_lib_crypt_mgr
	PRIVATE
		blowfish.cpp
		cryptmgr.cpp
)
