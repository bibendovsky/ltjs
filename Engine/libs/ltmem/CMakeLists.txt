set(LTJS_TARGET ltjs_lib_lt_mem)

include(ltjs_common)

ltjs_add_default_options(stdafx.h)

include_directories(
    ../../../libs/stdlith
    ../../sdk/inc
    ../../runtime/shared/src
    ../../runtime/kernel/src
    ../../runtime/kernel/mem/src
)

set(
    HDRS
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
    SRCS
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
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
