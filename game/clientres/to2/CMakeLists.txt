cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_to2_client_res VERSION 0.0.1 LANGUAGES CXX)

option(LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

include(ltjs_common)

add_library(ltjs_to2_client_res SHARED)

ltjs_add_defaults(ltjs_to2_client_res)

set_target_properties(
	ltjs_to2_client_res
	PROPERTIES
		OUTPUT_NAME ltjs_cres
		PREFIX ""
)

set(LTJS_TMP_LINKER_FLAGS "")

if (MSVC AND NOT LTJS_USE_SAFE_SEH)
	if (NOT LTJS_USE_SAFE_SEH)
		set(LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()
endif ()

set_target_properties(
	ltjs_to2_client_res
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

target_include_directories(
	ltjs_to2_client_res
	PRIVATE
		../shared
		../shared/lang/en
		../../shared/lang/en
)

target_sources(
	ltjs_to2_client_res
	PRIVATE
		clientres.h
		../shared/clientresshared.h
)

target_sources(
	ltjs_to2_client_res
	PRIVATE
		lang/en/clientres.rc
		main.cpp
)

install(
	TARGETS
	ltjs_to2_client_res
	RUNTIME DESTINATION "./Game"
)
