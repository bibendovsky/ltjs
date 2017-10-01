cmake_minimum_required(VERSION 3.5.1)
project(ltjs_to2_object VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    .
    ../../../engine/sdk/inc
    ../../../libs/butemgr
    ../../../libs/cryptmgr
    ../../../libs/lith
    ../../../libs/mfcstub
    ../../../libs/regmgr
    ../../../libs/stdlith
    ../../libs
    ../../libs/serverdir
    ../../shared
    ../../shared/to2
    ../objectshared
)

set(
    headers
    ../../shared/to2/to2missionbutemgr.h
    ../../shared/to2/to2versionmgr.h
    stdafx.h
    to2gameservershell.h
)

set(
    sources
    ../../shared/to2/to2missionbutemgr.cpp
    ../../shared/to2/to2versionmgr.cpp
    globalsinit.cpp
    stdafx.cpp
    to2gameservershell.cpp
)

add_library(
    ltjs_to2_object SHARED
    ${headers}
    ${sources}
)

set_target_properties(
    ltjs_to2_object
    PROPERTIES
        OUTPUT_NAME object
        PREFIX ""
        SUFFIX .lto
        RUNTIME_OUTPUT_DIRECTORY ${LTJS_TO2_BIN_DIR}
)

if (MSVC)
    set_target_properties(
        ltjs_to2_object
        PROPERTIES
            LINK_FLAGS /SUBSYSTEM:WINDOWS
    )
endif ()

target_link_libraries(
    ltjs_to2_object
    ltjs_lib_object
    ltjs_lib_bute_mgr
    ltjs_lib_std_lith
    ltjs_lib_client_shell
    ltjs_lib_crypt_mgr
    ltjs_lib_reg_mgr
    ltjs_lib_mfc_stub
    ltjs_server_dir
)

ltjs_add_defaults(ltjs_to2_object)

target_compile_definitions(
    ltjs_to2_object
    PRIVATE _SERVERBUILD
    PRIVATE MEMTRACK_SERVER
    PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:_NOLFBUILD>
    PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:USE_INTEL_COMPILER>
)

if (MSVC)
    target_compile_options(
        ltjs_to2_object
        PRIVATE -GR
    )
endif ()
