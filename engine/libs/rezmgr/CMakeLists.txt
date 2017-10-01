cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_rez_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../../../libs/lith
    ../../sdk/inc
)

set(
    headers
    rezfile.h
    rezhash.h
    rezmgr.h
    reztypes.h
    rezutils.h
)

set(
    sources
    rezfile.cpp
    rezhash.cpp
    rezmgr.cpp
    rezutils.cpp
)

add_library(
    ltjs_lib_rez_mgr STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_rez_mgr)
