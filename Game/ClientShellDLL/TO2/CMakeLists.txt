set(LTJS_TARGET ltjs_to2_client_shell)

include(ltjs_common)

ltjs_add_default_options(stdafx.h)

add_definitions(-D_CLIENTBUILD)
add_definitions(-DMEMTRACK_CLIENT)
add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-D_NOLFBUILD>)
add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-DNO_PRAGMA_LIBS>)

include_directories(
    ../../shared/to2
    .
    ../clientshellshared
    ../../clientres/shared
    ../../clientres/to2
    ../../shared
    ../../../engine/sdk/inc
    ../../../libs/stdlith
    ../../../libs/butemgr
    ../../../libs/cryptmgr
    ../../../libs/lith
    ../../../libs/mfcstub
    ../../libs/ltguimgr
    ../../../libs/regmgr
    ../../libs/wonapi
    ../../libs/serverdir
)

set(
    HDRS
    to2clientweaponallocator.h
    hudair.h
    hudammo.h
    hudcarrying.h
    hudchooser.h
    hudcompass.h
    hudcrosshair.h
    huddamagedir.h
    huddoomsday.h
    hudhiding.h
    hudhidingbar.h
    hudobjectives.h
    hudprogressbar.h
    hudradio.h
    hudrespawn.h
    ../../../engine/sdk/inc/iltbaseclass.h
    ../../libs/serverdir/iserverdir.h
    ../../../engine/sdk/inc/ltmodule.h
    ../../../engine/sdk/inc/ltobjref.h
    ../../shared/ltpoly.h
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
    to2gameclientshell.h
    to2hudmgr.h
    to2interfacemgr.h
    to2layoutmgr.h
    ../../shared/to2/to2missionbutemgr.h
    to2playermgr.h
    to2playerstats.h
    to2screenmgr.h
    ../../shared/to2/to2versionmgr.h
)

set(
    SRCS
    globalsinit.cpp
    hudair.cpp
    hudammo.cpp
    hudcarrying.cpp
    hudchooser.cpp
    hudcompass.cpp
    hudcrosshair.cpp
    huddamagedir.cpp
    huddoomsday.cpp
    hudhiding.cpp
    hudhidingbar.cpp
    hudobjectives.cpp
    hudprogressbar.cpp
    hudradio.cpp
    hudrespawn.cpp
    ../../../engine/sdk/inc/iltbaseclass.cpp
    ../../../engine/sdk/inc/ltmodule.cpp
    ../../../engine/sdk/inc/ltobjref.cpp
    ../../../engine/sdk/inc/ltquatbase.cpp
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
    ../../shared/to2/to2missionbutemgr.cpp
    to2playermgr.cpp
    to2playerstats.cpp
    to2screenmgr.cpp
    ../../shared/to2/to2versionmgr.cpp
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
    ${LTJS_TARGET} SHARED
    ${HDRS}
    ${SRCS}
)

set_target_properties(
    ${LTJS_TARGET}
    PROPERTIES
        OUTPUT_NAME cshell
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_TO2_BIN_DIR}
)

if (MSVC)
    set_target_properties(
        ${LTJS_TARGET}
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )

    set(
        PLIBS
        winmm
    )
endif ()

target_link_libraries(
    ${LTJS_TARGET}
    ${PLIBS}
    ltjs_lib_client_shell
    ltjs_lib_bute_mgr
    ltjs_lib_crypt_mgr
    ltjs_lib_reg_mgr
    ltjs_lib_lt_gui_mgr
    ltjs_lib_mfc_stub
    ltjs_lib_std_lith
    ltjs_server_dir
)
