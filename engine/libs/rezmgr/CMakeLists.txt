set(LTJS_TARGET ltjs_lib_rez_mgr)

include(ltjs_common)

ltjs_add_default_options()

include_directories(
    ../../../libs/lith
    ../../sdk/inc
)

set(
    HDRS
    rezfile.h
    rezhash.h
    rezmgr.h
    reztypes.h
    rezutils.h
)

set(
    SRCS
    rezfile.cpp
    rezhash.cpp
    rezmgr.cpp
    rezutils.cpp
)

add_library(
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
