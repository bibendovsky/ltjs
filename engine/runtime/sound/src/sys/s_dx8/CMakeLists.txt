cmake_minimum_required(VERSION 3.5.1)
project(ltjs_snd_drv_dx8 VERSION 0.0.1 LANGUAGES CXX)

find_package(LTJS_FFmpeg REQUIRED 2.8.11)

option(LTJS_INSTALL_SND_DRV_DX8 "Install DirectSound sound driver." OFF)

include(ltjs_common)

include_directories(
	${LTJS_FFMPEG_INCLUDE_DIR}
	../..
	../../../../../../directx/include
	../../../../../../libs/bibendovsky_spul_lib/include
	../../../../../../libs/lith
	../../../../../sdk/inc
	../../../../kernel/src
	../../../../shared/src/sys/win
)

set(
	headers
	../../ltjs_audio_decoder.h
	../../ltjs_audio_utils.h
	eax.h
	s_dx8.h
)

set(
	sources
	../../../../kernel/src/sys/win/counter.cpp
	../../ltjs_audio_decoder.cpp
	../../ltjs_audio_utils.cpp
	s_dx8.cpp
)

add_library(
	ltjs_snd_drv_dx8 SHARED
	${headers}
	${sources}
)

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
		path_to_dx_libs
		${CMAKE_SOURCE_DIR}/directx/lib/x86
	)

	set(
		libs
		${path_to_dx_libs}/dxguid.lib
		${CMAKE_CURRENT_SOURCE_DIR}/eaxguid.lib
		${ffmpeg_libs}
		winmm
		msacm32
	)
endif ()

if (MINGW)
	set(
		libs
		${CMAKE_CURRENT_SOURCE_DIR}/eaxguid.lib
		${ffmpeg_libs}
		dxguid
		msacm32
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