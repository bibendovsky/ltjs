cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_lib_lt_mem VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../../../libs/stdlith
		../../sdk/inc
		../../runtime/shared/src
		../../runtime/kernel/src
		../../runtime/kernel/mem/src
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		generalheap.h
		generalheapgroup.h
		lilfixedheap.h
		lilfixedheapgroup.h
		../../sdk/inc/ltmem.h
		ltmemdebug.h
		ltmemheap.h
		ltmemtrack.h
		stdafx.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		ltmem.cpp
		ltmemdebug.cpp
		ltmemheap.cpp
		ltmemstats.cpp
		ltmemtrack.cpp
		stdafx.cpp
)
