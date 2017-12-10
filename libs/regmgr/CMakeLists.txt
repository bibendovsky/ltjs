cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_reg_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../lith
)

set(
    headers
    regmgr.h
)

set(
    sources
    regmgr.cpp
)

add_library(
    ltjs_lib_reg_mgr STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_reg_mgr)
