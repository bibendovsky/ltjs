cmake_minimum_required(VERSION 3.5.1)
project(ltjs_snd_drv_oal VERSION 0.0.1 LANGUAGES CXX)

find_package(OpenAL REQUIRED 1.1)
find_package(LTJS_FFmpeg REQUIRED 2.8.11)

option(LTJS_INSTALL_SND_DRV_OAL "Install OpenAL sound driver." ON)

include(ltjs_common)

include_directories(
	${OPENAL_INCLUDE_DIR}
	${LTJS_FFMPEG_INCLUDE_DIR}
	../..
	../../../../../../libs/bibendovsky_spul_lib/include
	../../../../../../libs/lith
	../../../../../sdk/inc
	../../../../kernel/src
)

set(
	headers
	../../ltjs_audio_decoder.h
	../../ltjs_audio_utils.h
	../../ltjs_wave_utils.h
	s_oal.h
)

set(
	sources
	../../ltjs_audio_decoder.cpp
	../../ltjs_audio_utils.cpp
	../../ltjs_wave_utils.cpp
	s_oal.cpp
)

add_library(
	ltjs_snd_drv_oal SHARED
	${headers}
	${sources}
)

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

if (MSVC)
	set_target_properties(
		ltjs_snd_drv_oal
		PROPERTIES
			LINK_FLAGS /SUBSYSTEM:WINDOWS
	)

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

target_link_libraries(
	ltjs_snd_drv_oal
	bibendovsky_spul_lib
	ltjs_lib_lt_mem
	${libs}
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
