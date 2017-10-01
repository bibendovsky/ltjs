cmake_minimum_required(VERSION 3.5.1)
project(ltjs_to2_client_shell VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
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

set(
    headers
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

set(
    sources
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

if (MSVC)
    set_source_files_properties(
        ../../../engine/sdk/inc/iltbaseclass.cpp
        ../../../engine/sdk/inc/ltmodule.cpp
        ../../../engine/sdk/inc/ltobjref.cpp
        ../../../engine/sdk/inc/ltquatbase.cpp
        PROPERTIES
            # Precompiled Header (None)
            COMPILE_FLAGS -Y-
    )

    set_source_files_properties(
        stdafx.cpp
        PROPERTIES
            # Precompiled Header (Create)
            COMPILE_FLAGS -Yc
    )
endif ()

add_library(
    ltjs_to2_client_shell SHARED
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_to2_client_shell stdafx.h)

target_compile_definitions(
    ltjs_to2_client_shell
    PRIVATE _CLIENTBUILD
    PRIVATE MEMTRACK_CLIENT
    PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:_NOLFBUILD>
    PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:NO_PRAGMA_LIBS>
)

if (MSVC)
    target_compile_options(
        ltjs_to2_client_shell
        PRIVATE -GR
    )
endif ()

set_target_properties(
    ltjs_to2_client_shell
    PROPERTIES
        OUTPUT_NAME ltjs_cshell
        PREFIX ""
)

if (MSVC)
    set_target_properties(
        ltjs_to2_client_shell
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )

    set(
        libs
        winmm
    )
endif ()

if (MINGW)
    set(
        libs
        winmm
    )
endif ()

target_link_libraries(
    ltjs_to2_client_shell
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

install(
    TARGETS
    ltjs_to2_client_shell
    RUNTIME DESTINATION "./Game"
)
