set(LTJS_TARGET ltjs_to2_server_res)

include(ltjs_common)

ltjs_add_default_options()

set(
    HDRS
    serverres.h
)

set(
    SRCS
    main.cpp
    serverres.rc
)

add_library(
    ${LTJS_TARGET} SHARED
    ${HDRS}
    ${SRCS}
)

set_target_properties(
    ${LTJS_TARGET}
    PROPERTIES
        OUTPUT_NAME sres
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_TO2_BIN_DIR}
)
