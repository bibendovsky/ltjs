cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_snd_drv_oal VERSION 0.0.1 LANGUAGES CXX)

find_package(OpenAL REQUIRED 1.1)
find_package(LTJS_FFmpeg REQUIRED 2.8.11)

option(LTJS_INSTALL_SND_DRV_OAL "Install OpenAL sound driver." ON)
option(LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

include(ltjs_common)

add_library(ltjs_snd_drv_oal SHARED)

ltjs_add_defaults(ltjs_snd_drv_oal)

set_target_properties(
	ltjs_snd_drv_oal
	PROPERTIES
		OUTPUT_NAME ltjs_snddrv
		PREFIX ""
)

if (LTJS_FFMPEG_LIBRARY_DIR)
	set(
		ffmpeg_libs
		${LTJS_FFMPEG_LIBRARY}
	)
endif ()

set(LTJS_TMP_LINKER_FLAGS "")

if (MSVC)
	set(LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

	if (NOT LTJS_USE_SAFE_SEH)
		set(LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()

	set(
		libs
		${ffmpeg_libs}
		${OPENAL_LIBRARY}
	)
endif ()

if (MINGW)
	set(
		libs
		${ffmpeg_libs}
		${OPENAL_LIBRARY}
		iconv
		z
	)
endif ()

set_target_properties(
	ltjs_snd_drv_oal
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

target_link_libraries(
	ltjs_snd_drv_oal
	bibendovsky_spul_lib
	ltjs_lib_lt_mem
	${libs}
)

target_include_directories(
	ltjs_snd_drv_oal
	PRIVATE
		${LTJS_FFMPEG_INCLUDE_DIR}
		../..
		../../../../../../libs/bibendovsky_spul_lib/include
		../../../../../../libs/lith
		../../../../../../libs/openal_soft/include/AL
		../../../../../sdk/inc
		../../../../kernel/src
)

target_sources(
	ltjs_snd_drv_oal
	PRIVATE
		../../ltjs_audio_decoder.h
		../../ltjs_audio_utils.h
		s_oal.h
)

target_sources(
	ltjs_snd_drv_oal
	PRIVATE
		../../ltjs_audio_decoder.cpp
		../../ltjs_audio_utils.cpp
		s_oal.cpp
)

if (LTJS_INSTALL_SND_DRV_OAL)
	install(
		TARGETS
		ltjs_snd_drv_oal
		RUNTIME DESTINATION "."
	)
endif ()

install(
	TARGETS
	ltjs_snd_drv_oal
	RUNTIME DESTINATION "ltjs_oal_"
)
