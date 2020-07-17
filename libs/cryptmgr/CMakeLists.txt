cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_crypt_mgr VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_sources (
	${PROJECT_NAME}
	PRIVATE
		blowfish.h
		cryptmgr.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		blowfish.cpp
		cryptmgr.cpp
)
