cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_crypt_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

set(
    headers
    blowfish.h
    cryptmgr.h
)

set(
    sources
    blowfish.cpp
    cryptmgr.cpp
)

add_library(
    ltjs_lib_crypt_mgr STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_crypt_mgr)
