set(LTJS_TARGET ltjs_lib_mfc_stub)

include(ltjs_common)

ltjs_add_default_options()

include_directories(
    ../stdlith
)

set(
    HDRS
    mfcs_misc.h
    mfcs_point.h
    mfcs_rect.h
    mfcs_string.h
    mfcs_types.h
    mfcstub.h
    stdafx.h
)

set(
    SRCS
    mfcs_point.cpp
    mfcs_rect.cpp
    mfcs_string.cpp
)

add_library(
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
