set(LTJS_TARGET ltjs_lib_bute_mgr)

include(ltjs_common)

ltjs_add_default_options(stdafx.h)

add_definitions(-D_NOMFC)
add_definitions(-DWINVER=0x500)

include_directories(
    ../stdlith
    ../mfcstub
    ../cryptmgr
    ../lith
)

set(
    HDRS
    arange.h
    avector.h
    butemgr.h
    stdafx.h
)

set(
    SRCS
    arange.cpp
    avector.cpp
    butemgr.cpp
    stdafx.cpp
)

if (MSVC)
    set_source_files_properties(
        stdafx.cpp
        PROPERTIES
            # Precompiled Header (Create)
            COMPILE_FLAGS -Yc
)
endif ()

add_library(
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
