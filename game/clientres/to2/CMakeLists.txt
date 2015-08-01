set(LTJS_TARGET ltjs_to2_client_res)

include(ltjs_common)

ltjs_add_default_options()

include_directories(
    ../shared
    ../shared/lang/en
    ../../shared/lang/en
)

set(
    HDRS
    clientres.h
    ../shared/clientresshared.h
)

set(
    SRCS
    lang/en/clientres.rc
    main.cpp
)

add_library(
    ${LTJS_TARGET} SHARED
    ${HDRS}
    ${SRCS}
)

set_target_properties(
    ${LTJS_TARGET}
    PROPERTIES
        OUTPUT_NAME cres
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_TO2_BIN_DIR}
)
