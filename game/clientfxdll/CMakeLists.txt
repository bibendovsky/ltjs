set(LTJS_TARGET ltjs_game_client_fx)

include(ltjs_common)

ltjs_add_default_options(stdafx.h)

add_definitions(-DMEMTRACK_CLIENTFX)

include_directories(
    ../libs/ltguimgr
    ../shared
    ../../engine/sdk/inc
    ../../libs/mfcstub
    ../../libs/stdlith
    ../../libs/butemgr
    ../../libs/cryptmgr
    ../../libs/lith
)

set(
    HDRS
    ../shared/bankedlist.h
    ../shared/bankedlist_impl.h
    ../shared/basefx.h
    bouncychunkfx.h
    camjitterfx.h
    camwobblefx.h
    clientfx.h
    createfx.h
    cycletimer.h
    ../shared/debugnew.h
    ../shared/debugnew_impl.h
    dynalightfx.h
    fallingstufffx.h
    flarespritefx.h
    lightningfx.h
    ltbbouncychunkfx.h
    ltbmodelfx.h
    memarray.h
    nullfx.h
    particlesystemfx.h
    playrandomsoundfx.h
    playsoundfx.h
    polyfanfx.h
    polytubefx.h
    resource.h
    spritefx.h
    spritesystemfx.h
    stdafx.h
    swaplist.h
)

set(
    SRCS
    basefx.cpp
    bouncychunkfx.cpp
    camjitterfx.cpp
    camwobblefx.cpp
    clientfx.cpp
    createfx.cpp
    cycletimer.cpp
    ../shared/debugnew.cpp
    dynalightfx.cpp
    fallingstufffx.cpp
    flarespritefx.cpp
    lightningfx.cpp
    ltbbouncychunkfx.cpp
    ltbmodelfx.cpp
    ../../engine/sdk/inc/ltmodule.cpp
    ../../engine/sdk/inc/ltquatbase.cpp
    nullfx.cpp
    particlesystemfx.cpp
    playrandomsoundfx.cpp
    playsoundfx.cpp
    polyfanfx.cpp
    polytubefx.cpp
    spritefx.cpp
    spritesystemfx.cpp
    stdafx.cpp
)

if (MSVC)
    set_source_files_properties(
        ../../engine/sdk/inc/ltmodule.cpp
        ../../engine/sdk/inc/ltquatbase.cpp
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

if (MSVC)
    set_target_properties(
        ${LTJS_TARGET}
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )
endif ()

set_target_properties(
    ${LTJS_TARGET}
    PROPERTIES
        OUTPUT_NAME clientfx
        SUFFIX .fxd
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_GAME_BIN_DIR}
)
