cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_to2_client_shell VERSION 0.0.1 LANGUAGES CXX)

option (LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

include (ltjs_common)

add_library (${PROJECT_NAME} SHARED)

ltjs_add_defaults (${PROJECT_NAME})

if (NOT (${CMAKE_VERSION} VERSION_LESS "3.16.0"))
	target_precompile_headers(
		${PROJECT_NAME}
		PRIVATE
			[["stdafx.h"]]
	)
endif ()

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		_CLIENTBUILD
		MEMTRACK_CLIENT
		$<$<NOT:$<CONFIG:DEBUG>>:_NOLFBUILD>
		$<$<NOT:$<CONFIG:DEBUG>>:NO_PRAGMA_LIBS>
)

if (MSVC)
	target_compile_options (
		${PROJECT_NAME}
		PRIVATE
			-GR
	)
endif ()

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME ltjs_cshell
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
		winmm
	)
endif ()

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		ltjs_lib_bute_mgr
		ltjs_lib_client_shell
		ltjs_lib_crypt_mgr
		ltjs_lib_reg_mgr
		ltjs_lib_lt_gui_mgr
		ltjs_lib_mfc_stub
		ltjs_lib_std_lith
		ltjs_server_dir
		${libs}
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../../engine/sdk/inc
		../../../libs/butemgr
		../../../libs/cryptmgr
		../../../libs/lith
		../../../libs/mfcstub
		../../../libs/regmgr
		../../../libs/stdlith
		../../clientres/shared
		../../clientres/to2
		../../libs/ltguimgr
		../../libs/serverdir
		../../libs/wonapi
		../../shared
		../../shared/to2
		../clientshellshared
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../engine/sdk/inc/iltbaseclass.h
		../../../engine/sdk/inc/ltmodule.h
		../../../engine/sdk/inc/ltobjref.h
		../../libs/serverdir/iserverdir.h
		../../shared/ltpoly.h
		../../shared/to2/to2missionbutemgr.h
		../../shared/to2/to2versionmgr.h
		hudair.h
		hudammo.h
		hudcarrying.h
		hudchooser.h
		hudcompass.h
		hudcrosshair.h
		huddamagedir.h
		huddoomsday.h
		hudhealth.h
		hudhiding.h
		hudhidingbar.h
		hudobjectives.h
		hudprogressbar.h
		hudradio.h
		hudrespawn.h
		screenaudio.h
		screenconfigure.h
		screencontrols.h
		screencrosshair.h
		screendisplay.h
		screenendcoopmission.h
		screenenddmmission.h
		screenendmission.h
		screenfailure.h
		screengame.h
		screenhost.h
		screenhostddoptions.h
		screenhostdmoptions.h
		screenhostlevels.h
		screenhostmission.h
		screenhostoptions.h
		screenhosttdmoptions.h
		screenhostweapons.h
		screenjoin.h
		screenjoinlan.h
		screenjoystick.h
		screenkeyboard.h
		screenload.h
		screenmain.h
		screenmouse.h
		screenmulti.h
		screenoptions.h
		screenperformance.h
		screenplayer.h
		screenplayerskills.h
		screenplayerteam.h
		screenprofile.h
		screensave.h
		screensingle.h
		screenteam.h
		stdafx.h
		to2clientweaponallocator.h
		to2gameclientshell.h
		to2hudmgr.h
		to2interfacemgr.h
		to2layoutmgr.h
		to2playermgr.h
		to2playerstats.h
		to2screenmgr.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../engine/sdk/inc/iltbaseclass.cpp
		../../../engine/sdk/inc/ltmodule.cpp
		../../../engine/sdk/inc/ltobjref.cpp
		../../../engine/sdk/inc/ltquatbase.cpp
		../../shared/to2/to2missionbutemgr.cpp
		../../shared/to2/to2versionmgr.cpp
		globalsinit.cpp
		hudair.cpp
		hudammo.cpp
		hudcarrying.cpp
		hudchooser.cpp
		hudcompass.cpp
		hudcrosshair.cpp
		huddamagedir.cpp
		huddoomsday.cpp
		hudhealth.cpp
		hudhiding.cpp
		hudhidingbar.cpp
		hudobjectives.cpp
		hudprogressbar.cpp
		hudradio.cpp
		hudrespawn.cpp
		screenaudio.cpp
		screenconfigure.cpp
		screencontrols.cpp
		screencrosshair.cpp
		screendisplay.cpp
		screenendcoopmission.cpp
		screenenddmmission.cpp
		screenendmission.cpp
		screenfailure.cpp
		screengame.cpp
		screenhost.cpp
		screenhostddoptions.cpp
		screenhostdmoptions.cpp
		screenhostlevels.cpp
		screenhostmission.cpp
		screenhostoptions.cpp
		screenhosttdmoptions.cpp
		screenhostweapons.cpp
		screenjoin.cpp
		screenjoinlan.cpp
		screenjoystick.cpp
		screenkeyboard.cpp
		screenload.cpp
		screenmain.cpp
		screenmouse.cpp
		screenmulti.cpp
		screenoptions.cpp
		screenperformance.cpp
		screenplayer.cpp
		screenplayerskills.cpp
		screenplayerteam.cpp
		screenprofile.cpp
		screensave.cpp
		screensingle.cpp
		screenteam.cpp
		stdafx.cpp
		to2clientweaponallocator.cpp
		to2gameclientshell.cpp
		to2hudmgr.cpp
		to2interfacemgr.cpp
		to2layoutmgr.cpp
		to2playermgr.cpp
		to2playerstats.cpp
		to2screenmgr.cpp
)


install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION
		"./Game"
)
