set(LTJS_TARGET ltjs_snd_drv)

include(ltjs_common)

ltjs_add_default_options()

include_directories(
    ../../../../../../directx/include
    ../../../../../sdk/inc
    ../../../../../../libs/lith
    ../..
    ../../../../shared/src/sys/win
    ../../../../kernel/src
)

set(
    HDRS
    eax.h
    s_dx8.h
    stdafx.h
)

set(
    SRCS
    ../../../../kernel/src/sys/win/counter.cpp
    parsewavefile.cpp
    s_dx8.cpp
    stdafx.cpp
)

add_library(
    ${LTJS_TARGET} SHARED
    ${HDRS}
    ${SRCS}
)

set_target_properties(
    ${LTJS_TARGET}
    PROPERTIES
        OUTPUT_NAME snddrv
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_BIN_DIR}
)

if (MSVC)
    set_target_properties(
        ${LTJS_TARGET}
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )

    set(
        PATH_TO_DX_LIBS
        ${CMAKE_SOURCE_DIR}/directx/lib/x86
    )

    set(
        PLIBS
        ${PATH_TO_DX_LIBS}/dxguid.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/eaxguid.lib
        winmm
        msacm32
    )
endif ()

if (MINGW)
    set(
        PLIBS
        ${CMAKE_CURRENT_SOURCE_DIR}/eaxguid.lib
        dxguid
        msacm32
        winmm
    )
endif ()

target_link_libraries(
    ${LTJS_TARGET}
    ltjs_lib_lt_mem
    ${PLIBS}
)
