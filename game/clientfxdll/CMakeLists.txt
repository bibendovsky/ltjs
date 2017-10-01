cmake_minimum_required(VERSION 3.5.1)
project(ltjs_game_client_fx VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../../engine/sdk/inc
    ../../libs/butemgr
    ../../libs/cryptmgr
    ../../libs/lith
    ../../libs/mfcstub
    ../../libs/stdlith
    ../libs/ltguimgr
    ../shared
)

set(
    headers
    ../shared/bankedlist.h
    ../shared/bankedlist_impl.h
    ../shared/basefx.h
    ../shared/debugnew.h
    ../shared/debugnew_impl.h
    bouncychunkfx.h
    camjitterfx.h
    camwobblefx.h
    clientfx.h
    createfx.h
    cycletimer.h
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
    sources
    ../../engine/sdk/inc/ltmodule.cpp
    ../../engine/sdk/inc/ltquatbase.cpp
    ../shared/debugnew.cpp
    basefx.cpp
    bouncychunkfx.cpp
    camjitterfx.cpp
    camwobblefx.cpp
    clientfx.cpp
    createfx.cpp
    cycletimer.cpp
    dynalightfx.cpp
    fallingstufffx.cpp
    flarespritefx.cpp
    lightningfx.cpp
    ltbbouncychunkfx.cpp
    ltbmodelfx.cpp
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
    ltjs_game_client_fx SHARED
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_game_client_fx stdafx.h)

target_compile_definitions(
    ltjs_game_client_fx
    PRIVATE MEMTRACK_CLIENTFX)

set_target_properties(
    ltjs_game_client_fx
    PROPERTIES
        OUTPUT_NAME clientfx
        PREFIX ""
        SUFFIX .fxd
)

if (MSVC)
    set_target_properties(
        ltjs_game_client_fx
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )
endif ()

install(
    TARGETS
    ltjs_game_client_fx
    RUNTIME DESTINATION "./game"
)
