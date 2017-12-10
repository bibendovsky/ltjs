cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_lith VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

set(
    headers
    basehash.h
    baselist.h
    baselistcounter.h
    dprintf.h
    lith.h
    lithbaselist.h
    lithchunkallocator.h
    lithsimpary.h
    lithsimparystat.h
    lithtmpl.h
    lithtypes.h
    virtlist.h
)

set(
    sources
    basehash.cpp
    baselist.cpp
    baselistcounter.cpp
    dprintf.cpp
    lith.cpp
    lithbaselist.cpp
    lithchunkallocator.cpp
    lithsimpary.cpp
    lithsimparystat.cpp
    virtlist.cpp
)

add_library(
    ltjs_lib_lith STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_lith)
