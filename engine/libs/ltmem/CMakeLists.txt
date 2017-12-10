cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_lt_mem VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../../../libs/stdlith
    ../../sdk/inc
    ../../runtime/shared/src
    ../../runtime/kernel/src
    ../../runtime/kernel/mem/src
)

set(
    headers
    generalheap.h
    generalheapgroup.h
    lilfixedheap.h
    lilfixedheapgroup.h
    ../../sdk/inc/ltmem.h
    ltmemdebug.h
    ltmemheap.h
    ltmemtrack.h
    stdafx.h
)

set(
    sources
    ltmem.cpp
    ltmemdebug.cpp
    ltmemheap.cpp
    ltmemstats.cpp
    ltmemtrack.cpp
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
    ltjs_lib_lt_mem STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_lt_mem stdafx.h)
