cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_rez_mgr VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../../../libs/lith
		../../sdk/inc
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		rezfile.h
		rezhash.h
		rezmgr.h
		reztypes.h
		rezutils.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		rezfile.cpp
		rezhash.cpp
		rezmgr.cpp
		rezutils.cpp
)
