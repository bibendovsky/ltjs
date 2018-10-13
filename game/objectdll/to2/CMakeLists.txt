cmake_minimum_required(VERSION 3.1.3)
project(ltjs_to2_object VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_to2_object SHARED)

ltjs_add_defaults(ltjs_to2_object)

set_target_properties(
	ltjs_to2_object
	PROPERTIES
		OUTPUT_NAME ltjs_object
		PREFIX ""
)

if (MSVC)
	set_target_properties(
		ltjs_to2_object
		PROPERTIES
			LINK_FLAGS /SUBSYSTEM:WINDOWS
	)
endif ()

target_compile_definitions(
	ltjs_to2_object
	PRIVATE
		_SERVERBUILD
		MEMTRACK_SERVER
		$<$<NOT:$<CONFIG:DEBUG>>:_NOLFBUILD>
		$<$<NOT:$<CONFIG:DEBUG>>:USE_INTEL_COMPILER>
)

if (MSVC)
	target_compile_options(
		ltjs_to2_object
		PRIVATE
			-GR
	)
endif ()

target_include_directories(
	ltjs_to2_object
	PRIVATE
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

target_sources(
	ltjs_to2_object
	PRIVATE
		../../shared/to2/to2missionbutemgr.h
		../../shared/to2/to2versionmgr.h
		stdafx.h
		to2gameservershell.h
)

target_sources(
	ltjs_to2_object
	PRIVATE
		../../shared/to2/to2missionbutemgr.cpp
		../../shared/to2/to2versionmgr.cpp
		globalsinit.cpp
		stdafx.cpp
		to2gameservershell.cpp
)

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

install(
	TARGETS
	ltjs_to2_object
	RUNTIME DESTINATION "./Game"
)
