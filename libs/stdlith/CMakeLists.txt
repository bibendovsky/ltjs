cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_std_lith VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../zlib
)

set(
    headers
    abstractio.h
    copy_lists.h
    dynarray.h
    fastlinklist.h
    fileio.h
    genhash.h
    genlist.h
    glink.h
    goodlinklist.h
    helpers.h
    l_allocator.h
    linklist.h
    lithexception.h
    memory.h
    memoryio.h
    morect.h
    multilinklist.h
    object_bank.h
    stdafx.h
    stdlith.h
    stdlithdefs.h
    stringholder.h
    struct_bank.h
    zfstream.h
)

set(
    sources
    abstractio.cpp
    dynarray.cpp
    fileio.cpp
    goodlinklist.cpp
    helpers.cpp
    l_allocator.cpp
    linklist.cpp
    lithexception.cpp
    memory.cpp
    memoryio.cpp
    stringholder.cpp
    struct_bank.cpp
    zfstream.cpp
)

add_library(
    ltjs_lib_std_lith STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_std_lith)
