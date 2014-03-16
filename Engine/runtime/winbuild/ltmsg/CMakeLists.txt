set(LTJS_TARGET ltjs_lt_msg)

include(ltjs_common)

ltjs_add_default_options()

set(
    HDRS
    resource.h
)

set(
    SRCS
    init.cpp
    lang/en/ltmsg.rc
)

add_library(
    ${LTJS_TARGET} SHARED
    ${HDRS}
    ${SRCS}
)

set_target_properties(
    ${LTJS_TARGET}
    PROPERTIES
        OUTPUT_NAME ltmsg
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_BIN_DIR}
)
