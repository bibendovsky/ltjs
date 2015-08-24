set(LTJS_TARGET ltjs_server_dir)

include(ltjs_common)

ltjs_add_default_options()

include_directories(
    .
    ../../../engine/sdk/inc
    ../../libs/wonapi
    ../../clientres/shared
)

set(
    HDRS
    cserverdir_titan.h
    iserverdir.h
    iserverdir_titan.h
    stdafx.h
    titantest.h
    winsync.h
)

set(
    SRCS
    cserverdir_titan.cpp
    stdafx.cpp
    titantest.cpp
)

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

if (MINGW)
    set(
        LIBS
        ws2_32
    )
endif ()

set_target_properties(
    ${LTJS_TARGET}
    PROPERTIES
        OUTPUT_NAME serverdir
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_BIN_DIR}
)

target_link_libraries(
    ${LTJS_TARGET}
    ltjs_lib_won_api
    ${LIBS}
)
