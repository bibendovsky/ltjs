cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_lt_mem VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_lt_mem STATIC)

ltjs_add_defaults(ltjs_lib_lt_mem stdafx.h)

target_include_directories(
	ltjs_lib_lt_mem
	PRIVATE
		../../../libs/stdlith
		../../sdk/inc
		../../runtime/shared/src
		../../runtime/kernel/src
		../../runtime/kernel/mem/src
)

target_sources(
	ltjs_lib_lt_mem
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

target_sources(
	ltjs_lib_lt_mem
	PRIVATE
		ltmem.cpp
		ltmemdebug.cpp
		ltmemheap.cpp
		ltmemstats.cpp
		ltmemtrack.cpp
		stdafx.cpp
)

if (MSVC)
	set_source_files_properties(
		stdafx.cpp
		PROPERTIES
			# Precompiled Header (Create)
			COMPILE_FLAGS -Yc
)
endif ()
