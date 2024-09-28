cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_snd_drv_oal VERSION 0.0.1 LANGUAGES CXX)

find_package (OpenAL REQUIRED 1.1)

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

	set (
		libs
		${OPENAL_LIBRARY}
	)
endif ()

if (MINGW)
	set (
		libs
		${OPENAL_LIBRARY}
		iconv
		z
	)
endif ()

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		bibendovsky_spul_lib
		ltjs_lib
		ltjs_lib_lt_mem
		ltjs::audio
		${libs}
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../..
		../../../../../../libs/lith
		../../../../../../libs/openal_soft/include/AL
		../../../../../../libs/spdlog/include
		../../../../../sdk/inc
		../../../../kernel/src
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../ltjs_audio_utils.h
		../../ltjs_eax_api.h
		ltjs_oal_efx_symbols.h
		ltjs_oal_eax_lt_filter.h
		ltjs_oal_efx_lt_filter.h
		ltjs_oal_lt_filter.h
		ltjs_oal_lt_sound_sys.h
		ltjs_oal_lt_sound_sys_generic_stream.h
		ltjs_oal_lt_sound_sys_orientation_3d.h
		ltjs_oal_lt_sound_sys_streaming_source.h
		ltjs_oal_lt_sound_user_data.h
		ltjs_oal_lt_sound_sys_vector_3d.h
		ltjs_oal_object.h
		ltjs_oal_system.h
		ltjs_oal_utils.h
		s_oal.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../ltjs_audio_utils.cpp
		../../ltjs_eax_api.cpp
		ltjs_oal_efx_symbols.cpp
		ltjs_oal_eax_lt_filter.cpp
		ltjs_oal_efx_lt_filter.cpp
		ltjs_oal_lt_filter.cpp
		ltjs_oal_lt_sound_sys.cpp
		ltjs_oal_lt_sound_sys_generic_stream.cpp
		ltjs_oal_lt_sound_sys_orientation_3d.cpp
		ltjs_oal_lt_sound_sys_streaming_source.cpp
		ltjs_oal_lt_sound_sys_vector_3d.cpp
		ltjs_oal_object.cpp
		ltjs_oal_system.cpp
		ltjs_oal_utils.cpp
		s_oal.cpp
)

install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION
		"."
)
