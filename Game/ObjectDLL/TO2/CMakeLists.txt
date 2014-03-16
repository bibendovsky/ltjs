set(LTJS_TARGET ltjs_to2_object)

include(ltjs_common)

ltjs_add_default_options()

add_definitions(-D_SERVERBUILD)
add_definitions(-DMEMTRACK_SERVER)
add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-D_NOLFBUILD>)
add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-DUSE_INTEL_COMPILER>)

include_directories(
    .
    ../objectshared
    ../../shared
    ../../../engine/sdk/inc
    ../../../libs/stdlith
    ../../../libs/butemgr
    ../../../libs/cryptmgr
    ../../../libs/lith
    ../../../libs/mfcstub
    ../../../libs/regmgr
    ../../libs
    ../../shared/to2
    ../../libs/serverdir
)

set(
    HDRS
    stdafx.h
    to2gameservershell.h
    ../../shared/to2/to2missionbutemgr.h
    ../../shared/to2/to2versionmgr.h
)

set(
    SRCS
    globalsinit.cpp
    stdafx.cpp
    to2gameservershell.cpp
    ../../shared/to2/to2missionbutemgr.cpp
    ../../shared/to2/to2versionmgr.cpp
)

add_library(
    ${LTJS_TARGET} SHARED
    ${HDRS}
    ${SRCS}
)

set_target_properties(
    ${LTJS_TARGET}
    PROPERTIES
        OUTPUT_NAME object
        SUFFIX .lto
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_TO2_BIN_DIR}
)

if (MSVC)
    set_target_properties(
        ${LTJS_TARGET}
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )
endif ()

target_link_libraries(
    ${LTJS_TARGET}
    ltjs_lib_object
    ltjs_lib_bute_mgr
    ltjs_lib_std_lith
    ltjs_lib_crypt_mgr
    ltjs_lib_reg_mgr
    ltjs_lib_mfc_stub
    ltjs_server_dir
)
