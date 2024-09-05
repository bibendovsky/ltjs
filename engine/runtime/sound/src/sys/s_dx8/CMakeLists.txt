cmake_minimum_required (VERSION 3.5.0)
project (ltjs_snd_drv_dx8 VERSION 0.0.1 LANGUAGES CXX)

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

if (LTJS_FFMPEG_LIBRARY_DIR)
	set (
		ffmpeg_libs
		${LTJS_FFMPEG_LIBRARY}
	)
endif ()

set (LTJS_TMP_LINKER_FLAGS "")

if (MSVC)
	set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

	if (NOT LTJS_USE_SAFE_SEH)
		set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()

	set (
		libs
		${ffmpeg_libs}
		dsound
		dxguid
		winmm
	)
endif ()

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

if (MINGW)
	set (
		libs
		${ffmpeg_libs}
		dsound
		dxguid
		winmm
		iconv
		z
	)
endif ()

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		bibendovsky_spul_lib
		ltjs_lib_lt_mem
		${libs}
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		${LTJS_FFMPEG_INCLUDE_DIR}
		../..
		../../../../../../libs/bibendovsky_spul_lib/include
		../../../../../../libs/lith
		../../../../../sdk/inc
		../../../../kernel/src
		../../../../shared/src/sys/win
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../ltjs_audio_decoder.h
		../../ltjs_audio_utils.h
		../../ltjs_eax_api.h
		s_dx8.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../../kernel/src/sys/win/counter.cpp
		../../ltjs_audio_decoder.cpp
		../../ltjs_audio_utils.cpp
		../../ltjs_eax_api.cpp
		s_dx8.cpp
)

install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION
		"."
)
