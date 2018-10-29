cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_server_dir VERSION 0.0.1 LANGUAGES CXX)

option(LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

include(ltjs_common)

add_library(ltjs_server_dir SHARED)

ltjs_add_defaults(ltjs_server_dir)

set(LTJS_TMP_LINKER_FLAGS "")

if (MSVC)
	set(LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

	if (NOT LTJS_USE_SAFE_SEH)
		set(LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()
endif ()

set_target_properties(
	ltjs_server_dir
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

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
