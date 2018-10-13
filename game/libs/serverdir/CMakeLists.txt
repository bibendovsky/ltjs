cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_server_dir VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_server_dir SHARED)

ltjs_add_defaults(ltjs_server_dir)

if (MSVC)
	set_target_properties(
		ltjs_server_dir
		PROPERTIES
			LINK_FLAGS /SUBSYSTEM:WINDOWS
	)
endif ()

set_target_properties(
	ltjs_server_dir
	PROPERTIES
		OUTPUT_NAME ltjs_serverdir
		PREFIX ""
)

target_include_directories(
	ltjs_server_dir
	PRIVATE
		../../../engine/sdk/inc
)

target_sources(
	ltjs_server_dir
	PRIVATE
		iserverdir.h
		stdafx.h
)

target_sources(
	ltjs_server_dir
	PRIVATE
		cserverdir_titan.cpp
		stdafx.cpp
)

install(
	TARGETS
	ltjs_server_dir
	RUNTIME DESTINATION "."
)
