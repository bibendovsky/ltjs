set(LTJS_TARGET ltjs_lib_reg_mgr)

include(ltjs_common)

ltjs_add_default_options()

include_directories(
    ../lith
)

set(
    HDRS
    regmgr.h
)

set(
    SRCS
    regmgr.cpp
)

add_library(
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
