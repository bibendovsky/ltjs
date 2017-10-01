cmake_minimum_required(VERSION 3.5.1)
project(ltjs_server_dir VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../../../engine/sdk/inc
)

set(
    headers
    iserverdir.h
    stdafx.h
)

set(
    sources
    cserverdir_titan.cpp
    stdafx.cpp
)

add_library(
    ltjs_server_dir SHARED
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_server_dir)

if (MSVC)
    set_target_properties(
        ltjs_server_dir
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )
endif ()

set_target_properties(
    ltjs_server_dir
    PROPERTIES
        OUTPUT_NAME ltjs_serverdir
        PREFIX ""
)

install(
    TARGETS
    ltjs_server_dir
    RUNTIME DESTINATION "."
)
