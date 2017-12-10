cmake_minimum_required(VERSION 3.5.1)
project(ltjs_to2_client_res VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../shared
    ../shared/lang/en
    ../../shared/lang/en
)

set(
    headers
    clientres.h
    ../shared/clientresshared.h
)

set(
    sources
    lang/en/clientres.rc
    main.cpp
)

add_library(
    ltjs_to2_client_res SHARED
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_to2_client_res)

set_target_properties(
    ltjs_to2_client_res
    PROPERTIES
        OUTPUT_NAME ltjs_cres
        PREFIX ""
)

install(
    TARGETS
    ltjs_to2_client_res
    RUNTIME DESTINATION "./Game"
)
