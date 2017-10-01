cmake_minimum_required(VERSION 3.5.1)
project(ltjs_to2_server_res VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../../../engine/sdk/inc
)

set(
    headers
    serverres.h
)

set(
    sources
    main.cpp
    serverres.rc
)

add_library(
    ltjs_to2_server_res SHARED
    ${headers}
    ${sources}
)

set_target_properties(
    ltjs_to2_server_res
    PROPERTIES
        OUTPUT_NAME sres
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_TO2_BIN_DIR}
)

ltjs_add_defaults(ltjs_to2_server_res)
