set(LTJS_TARGET ltjs_lib_crypt_mgr)

include(ltjs_common)

ltjs_add_default_options()

set(
    HDRS
    blowfish.h
    cryptmgr.h
)

set(
    SRCS
    blowfish.cpp
    cryptmgr.cpp
)

add_library(
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
