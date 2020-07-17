cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_to2_object VERSION 0.0.1 LANGUAGES CXX)

option (LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

include (ltjs_common)

add_library (${PROJECT_NAME} SHARED)

ltjs_add_defaults (${PROJECT_NAME})

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME ltjs_object
		PREFIX ""
)

set (LTJS_TMP_LINKER_FLAGS "")

if (MSVC)
	set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

	if (NOT LTJS_USE_SAFE_SEH)
		set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()
endif ()

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		_SERVERBUILD
		MEMTRACK_SERVER
		$<$<NOT:$<CONFIG:DEBUG>>:_NOLFBUILD>
		$<$<NOT:$<CONFIG:DEBUG>>:USE_INTEL_COMPILER>
)

if (MSVC)
	target_compile_options (
		${PROJECT_NAME}
		PRIVATE
			-GR
	)
endif ()

target_include_directories (
	${PROJECT_NAME}
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

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../shared/to2/to2missionbutemgr.h
		../../shared/to2/to2versionmgr.h
		stdafx.h
		to2gameservershell.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../shared/to2/to2missionbutemgr.cpp
		../../shared/to2/to2versionmgr.cpp
		globalsinit.cpp
		stdafx.cpp
		to2gameservershell.cpp
)

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		ltjs_lib_object
		ltjs_lib_bute_mgr
		ltjs_lib_std_lith
		ltjs_lib_client_shell
		ltjs_lib_crypt_mgr
		ltjs_lib_reg_mgr
		ltjs_lib_mfc_stub
		ltjs_server_dir
)

install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION
		"./Game"
)
