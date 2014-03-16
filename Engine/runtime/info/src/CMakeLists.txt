set(LTJS_TARGET ltjs_lib_info)

include(ltjs_common)

ltjs_add_default_options()

include_directories(
    .
    ../../../sdk/inc
    ../../../sdk/inc/libltinfo
    ../../kernel/src
    ../../shared/src
)

set(
    HDRS
    ../../../sdk/inc/libltinfo/iltoutputredir.h
    ../../../sdk/inc/libltinfo.h
    outputredir.h
)

set(
    SRCS
    outputredir.cpp
    sys/win32/outputredir_win32.cpp
)

add_library(
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
