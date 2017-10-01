cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_mfc_stub VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../stdlith
)

set(
    headers
    mfcs_misc.h
    mfcs_point.h
    mfcs_rect.h
    mfcs_string.h
    mfcs_types.h
    mfcstub.h
    stdafx.h
)

set(
    sources
    mfcs_point.cpp
    mfcs_rect.cpp
    mfcs_string.cpp
)

add_library(
    ltjs_lib_mfc_stub STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_mfc_stub)
