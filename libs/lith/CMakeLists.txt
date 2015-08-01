set(LTJS_TARGET ltjs_lib_lith)

include(ltjs_common)

ltjs_add_default_options()

set(
    HDRS
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
    SRCS
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
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
