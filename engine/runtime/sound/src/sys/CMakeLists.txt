cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_sound_drivers VERSION 0.0.1 LANGUAGES CXX)


set (LTJS_SND_DRV_NUL_DEFINE "NUL")
set (LTJS_SND_DRV_DX8_DEFINE "DX8")
set (LTJS_SND_DRV_OAL_DEFINE "OAL")

set (LTJS_SND_DRV_NAME_LIST "")

list (
	APPEND
	LTJS_SND_DRV_NAME_LIST
	${LTJS_SND_DRV_OAL_DEFINE}
)

if (WIN32)
	list (
		APPEND
		LTJS_SND_DRV_NAME_LIST
		${LTJS_SND_DRV_DX8_DEFINE}
	)
endif ()

list (
	APPEND
	LTJS_SND_DRV_NAME_LIST
	${LTJS_SND_DRV_NUL_DEFINE}
)

set (LTJS_SND_DRV ${LTJS_SND_DRV_OAL_DEFINE} CACHE STRING "Sound driver to install.")

set_property (
	CACHE
		LTJS_SND_DRV
	PROPERTY
		STRINGS
			${LTJS_SND_DRV_NAME_LIST}
)

if (LTJS_SND_DRV STREQUAL ${LTJS_SND_DRV_NUL_DEFINE})
	set (LTJS_SND_DRV_NAME "NUL")
elseif (WIN32 AND LTJS_SND_DRV STREQUAL ${LTJS_SND_DRV_DX8_DEFINE})
	set (LTJS_SND_DRV_NAME "DirectSound 8")
elseif (LTJS_SND_DRV STREQUAL ${LTJS_SND_DRV_OAL_DEFINE})
	set (LTJS_SND_DRV_NAME "OpenAL")
else ()
	message (FATAL_ERROR "[${PROJECT_NAME}] Unsupported sound driver: ${LTJS_SND_DRV_NAME}")
endif ()

message (STATUS "[${PROJECT_NAME}] Install driver: ${LTJS_SND_DRV_NAME}")


if (WIN32)
	add_subdirectory (s_dx8)
endif ()

add_subdirectory (s_nul)
add_subdirectory (s_oal)
