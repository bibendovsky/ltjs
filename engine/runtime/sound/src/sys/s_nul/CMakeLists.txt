cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_snd_drv_nul VERSION 0.0.1 LANGUAGES CXX)

option(LTJS_INSTALL_SND_DRV_NUL "Install NUL sound driver." OFF)
option(LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

include(ltjs_common)

add_library(ltjs_snd_drv_nul SHARED)

ltjs_add_defaults(ltjs_snd_drv_nul)

set_target_properties(
	ltjs_snd_drv_nul
	PROPERTIES
		OUTPUT_NAME ltjs_snddrv
		PREFIX ""
)

set(LTJS_TMP_LINKER_FLAGS "")

if (MSVC)
	set(LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

	if (NOT LTJS_USE_SAFE_SEH)
		set(LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()
endif ()

set_target_properties(
	ltjs_snd_drv_nul
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

target_link_libraries(
	ltjs_snd_drv_nul
	ltjs_lib_lt_mem
)

target_include_directories(
	ltjs_snd_drv_nul
	PRIVATE
		../..
		../../../../../../libs/bibendovsky_spul_lib/include
		../../../../../../libs/lith
		../../../../../sdk/inc
		../../../../kernel/src
)

target_sources(
	ltjs_snd_drv_nul
	PRIVATE
		s_nul.h
)

target_sources(
	ltjs_snd_drv_nul
	PRIVATE
		s_nul.cpp
)

if (LTJS_INSTALL_SND_DRV_NUL)
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
