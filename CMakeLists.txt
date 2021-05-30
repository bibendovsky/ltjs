cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs VERSION 0.0.1 LANGUAGES CXX)

if (NOT WIN32)
	message (FATAL_ERROR "WIN32 only")
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#
# Options
#
option (LTJS_SDL_BACKEND "Use SDL backend." ON)
option (LTJS_USE_PCH "Use precompiled headers." ON)
option (LTJS_USE_D3DX9 "Use Direct3D 9 extensions." OFF)


#
# SDL2
#
if (WIN32)
	set (LTJS_SDL2_MIN_VERSION "2.0.8")
else ()
	set (LTJS_SDL2_MIN_VERSION "2.0.4")
endif ()

find_package (SDL2W ${LTJS_SDL2_MIN_VERSION} REQUIRED)


#
# FFmpeg
#
find_package (LTJS_FFmpeg REQUIRED 2.8.11)


add_subdirectory (libs/bibendovsky_spul_lib)
add_subdirectory (libs/ltjs)
add_subdirectory (libs/butemgr)
add_subdirectory (libs/cryptmgr)
add_subdirectory (libs/freetype)
add_subdirectory (libs/lith)
add_subdirectory (libs/mfcstub)
add_subdirectory (libs/regmgr)
add_subdirectory (libs/stdlith)

add_subdirectory (engine/libs/ltmem)
add_subdirectory (engine/libs/rezmgr)
add_subdirectory (engine/runtime/info/src)
add_subdirectory (engine/runtime/render_a/src/sys/d3d)
add_subdirectory (engine/runtime/sound/src)
add_subdirectory (engine/runtime/sound/src/sys)
add_subdirectory (engine/runtime/ui/src)
add_subdirectory (engine/runtime/winbuild/lithtech)
add_subdirectory (engine/runtime/winbuild/ltmsg)
add_subdirectory (engine/runtime/winbuild/server)
add_subdirectory (game/clientfxdll)
add_subdirectory (game/clientres/to2)
add_subdirectory (game/clientshelldll/clientshellshared)
add_subdirectory (game/clientshelldll/to2)
add_subdirectory (game/launcher)
add_subdirectory (game/libs/ltguimgr)
add_subdirectory (game/libs/serverdir)
add_subdirectory (game/objectdll/objectshared)
add_subdirectory (game/objectdll/to2)
add_subdirectory (game/serverres/to2)


install (
	DIRECTORY
		"ltjs"
	DESTINATION
		"."
)
