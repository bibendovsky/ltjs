cmake_minimum_required(VERSION 3.5.1)
project(ltjs_snd_drv VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../..
    ../../../../../../directx/include
    ../../../../../../libs/lith
    ../../../../../sdk/inc
    ../../../../kernel/src
    ../../../../shared/src/sys/win
)

set(
    headers
    eax.h
    s_dx8.h
    stdafx.h
)

set(
    sources
    ../../../../kernel/src/sys/win/counter.cpp
    parsewavefile.cpp
    s_dx8.cpp
    stdafx.cpp
)

add_library(
    ltjs_snd_drv SHARED
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_snd_drv)

set_target_properties(
    ltjs_snd_drv
    PROPERTIES
        OUTPUT_NAME snddrv
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_BIN_DIR}
)

if (MSVC)
    set_target_properties(
        ltjs_snd_drv
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
        winmm
        msacm32
    )
endif ()

if (MINGW)
    set(
        libs
        ${CMAKE_CURRENT_SOURCE_DIR}/eaxguid.lib
        dxguid
        msacm32
        winmm
    )
endif ()

target_link_libraries(
    ltjs_snd_drv
    ltjs_lib_lt_mem
    ${libs}
)
