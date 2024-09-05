cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_snd_drv_nul VERSION 0.0.1 LANGUAGES CXX)

option (LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

include (ltjs_common)

add_library (${PROJECT_NAME} SHARED)

ltjs_add_defaults (${PROJECT_NAME})

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME "${PROJECT_NAME}"
		PREFIX ""
)

set (LTJS_TMP_LINKER_FLAGS "")

if (MSVC)
	set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

	if (NOT LTJS_USE_SAFE_SEH)
		set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()
endif ()

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		ltjs_lib_lt_mem
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../..
		../../../../../../libs/bibendovsky_spul_lib/include
		../../../../../../libs/lith
		../../../../../sdk/inc
		../../../../kernel/src
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		s_nul.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		s_nul.cpp
)

install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION
		"."
)
