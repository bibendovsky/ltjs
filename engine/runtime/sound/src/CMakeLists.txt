set(LTJS_TARGET ltjs_lib_ilt_sound)

include(ltjs_common)

ltjs_add_default_options()

# User defines
add_definitions(-DUSE_ABSTRACT_SOUND_INTERFACES)

include_directories(
    ../../../sdk/inc
)

set(
    HDRS
    iltsound.h
)

set(
    SRCS
    iltsound.cpp
)

add_library(
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
