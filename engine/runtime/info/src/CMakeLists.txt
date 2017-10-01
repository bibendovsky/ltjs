cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_info VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    .
    ../../../sdk/inc
    ../../../sdk/inc/libltinfo
    ../../kernel/src
    ../../shared/src
)

set(
    headers
    ../../../sdk/inc/libltinfo/iltoutputredir.h
    ../../../sdk/inc/libltinfo.h
    outputredir.h
)

set(
    sources
    outputredir.cpp
    sys/win32/outputredir_win32.cpp
)

add_library(
    ltjs_lib_info STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_info)
