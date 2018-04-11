cmake_minimum_required(VERSION 3.5.1)
project(ltjs_snd_drv_nul VERSION 0.0.1 LANGUAGES CXX)

option(LTJS_INSTALL_SND_DRV_NUL "Install NUL sound driver." OFF)

include(ltjs_common)

include_directories(
	../..
	../../../../../../libs/bibendovsky_spul_lib/include
	../../../../../../libs/lith
	../../../../../sdk/inc
	../../../../kernel/src
)

set(
	headers
	s_nul.h
)

set(
	sources
	s_nul.cpp
)

add_library(
	ltjs_snd_drv_nul SHARED
	${headers}
	${sources}
)

ltjs_add_defaults(ltjs_snd_drv_nul)

set_target_properties(
	ltjs_snd_drv_nul
	PROPERTIES
		OUTPUT_NAME ltjs_snddrv
		PREFIX ""
)

if (MSVC)
	set_target_properties(
		ltjs_snd_drv_nul
		PROPERTIES
			LINK_FLAGS /SUBSYSTEM:WINDOWS
	)
endif ()

target_link_libraries(
	ltjs_snd_drv_nul
	ltjs_lib_lt_mem
)

if (LTJS_INSTALL_SND_DRV_OAL)
	install(
		TARGETS
		ltjs_snd_drv_nul
		RUNTIME DESTINATION "."
	)
endif ()

install(
	TARGETS
	ltjs_snd_drv_nul
	RUNTIME DESTINATION "ltjs_nul_"
)
