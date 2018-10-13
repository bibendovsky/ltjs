cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_bute_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_bute_mgr STATIC)

ltjs_add_defaults(ltjs_lib_bute_mgr stdafx.h)

target_compile_definitions(
	ltjs_lib_bute_mgr
	PRIVATE _NOMFC)

if (WIN32)
	target_compile_definitions(
		ltjs_lib_bute_mgr
		PRIVATE WINVER=0x500)
endif ()

target_include_directories(
	ltjs_lib_bute_mgr
	PRIVATE
		../cryptmgr
		../lith
		../mfcstub
		../stdlith
)

target_sources(
	ltjs_lib_bute_mgr
	PRIVATE
		arange.h
		avector.h
		butemgr.h
		stdafx.h
)

target_sources(
	ltjs_lib_bute_mgr
	PRIVATE
		arange.cpp
		avector.cpp
		butemgr.cpp
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
