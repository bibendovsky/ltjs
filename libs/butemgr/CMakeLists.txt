cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_bute_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../cryptmgr
    ../lith
    ../mfcstub
    ../stdlith
)

set(
    headers
    arange.h
    avector.h
    butemgr.h
    stdafx.h
)

set(
    sources
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
    ltjs_lib_bute_mgr STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_bute_mgr stdafx.h)

target_compile_definitions(
    ltjs_lib_bute_mgr
    PRIVATE _NOMFC)

set_target_properties(
    ltjs_lib_bute_mgr
    PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if (WIN32)
    target_compile_definitions(
        ltjs_lib_bute_mgr
        PRIVATE WINVER=0x500)
endif ()
