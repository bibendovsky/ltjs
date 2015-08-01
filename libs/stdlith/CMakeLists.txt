set(LTJS_TARGET ltjs_lib_std_lith)

include(ltjs_common)

ltjs_add_default_options()

include_directories(
    ../zlib
)

set(
    HDRS
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
    SRCS
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
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
