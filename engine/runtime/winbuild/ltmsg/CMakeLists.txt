cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lt_msg VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

set(
    headers
    resource.h
)

set(
    sources
    init.cpp
    lang/en/ltmsg.rc
)

add_library(
    ltjs_lt_msg SHARED
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lt_msg)

set_target_properties(
    ltjs_lt_msg
    PROPERTIES
        OUTPUT_NAME ltjs_ltmsg
        PREFIX ""
)

install(
    TARGETS
    ltjs_lt_msg
    RUNTIME DESTINATION "."
)
