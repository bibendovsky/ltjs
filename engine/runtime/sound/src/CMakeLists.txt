cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_ilt_sound VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../../../sdk/inc
    ../../../../libs/bibendovsky_spul_lib/include
)

set(
    headers
    iltsound.h
)

set(
    sources
    iltsound.cpp
)

add_library(
    ltjs_lib_ilt_sound STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_ilt_sound)

target_compile_definitions(
    ltjs_lib_ilt_sound
    PRIVATE USE_ABSTRACT_SOUND_INTERFACES
)