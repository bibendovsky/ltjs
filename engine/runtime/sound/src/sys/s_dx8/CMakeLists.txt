cmake_minimum_required(VERSION 3.1.3)
project(ltjs_snd_drv_dx8 VERSION 0.0.1 LANGUAGES CXX)

find_package(LTJS_FFmpeg REQUIRED 2.8.11)

option(LTJS_INSTALL_SND_DRV_DX8 "Install DirectSound sound driver." OFF)

include(ltjs_common)

add_library(ltjs_snd_drv_dx8 SHARED)

ltjs_add_defaults(ltjs_snd_drv_dx8)

set_target_properties(
	ltjs_snd_drv_dx8
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
		ltjs_snd_drv_dx8
		PROPERTIES
			LINK_FLAGS /SUBSYSTEM:WINDOWS
	)

	set(
		libs
		${ffmpeg_libs}
		dsound
		dxguid
		winmm
	)
endif ()

if (MINGW)
	set(
		libs
		${ffmpeg_libs}
		dsound
		dxguid
		winmm
		iconv
		z
	)
endif ()

target_link_libraries(
	ltjs_snd_drv_dx8
	bibendovsky_spul_lib
	ltjs_lib_lt_mem
	${libs}
)

target_include_directories(
	ltjs_snd_drv_dx8
	PRIVATE
		${LTJS_FFMPEG_INCLUDE_DIR}
		../..
		../../../../../../libs/bibendovsky_spul_lib/include
		../../../../../../libs/lith
		../../../../../sdk/inc
		../../../../kernel/src
		../../../../shared/src/sys/win
)

target_sources(
	ltjs_snd_drv_dx8
	PRIVATE
		../../ltjs_audio_decoder.h
		../../ltjs_audio_utils.h
		eax.h
		s_dx8.h
)

target_sources(
	ltjs_snd_drv_dx8
	PRIVATE
		../../../../kernel/src/sys/win/counter.cpp
		../../ltjs_audio_decoder.cpp
		../../ltjs_audio_utils.cpp
		s_dx8.cpp
)

if (LTJS_INSTALL_SND_DRV_DX8)
	install(
		TARGETS
		ltjs_snd_drv_dx8
		RUNTIME DESTINATION "."
	)
endif ()

install(
	TARGETS
	ltjs_snd_drv_dx8
	RUNTIME DESTINATION "ltjs_dx8_"
)
