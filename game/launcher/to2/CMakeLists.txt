cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_nolf2 VERSION 0.0.1 LANGUAGES CXX)

option (LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

if (NOT WIN32)
	message (FATAL_ERROR "Windows only.")
endif ()

include (ltjs_common)

add_executable (${PROJECT_NAME})

ltjs_add_defaults (${PROJECT_NAME})

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME ltjs_nolf2
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
		LTJS_NOLF2
)

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		SDL2W::SDL2Wmain
		SDL2W::SDL2W
		bibendovsky_spul_lib
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../../../libs/bibendovsky_spul_lib/include
		../../../libs/glad/1_1
		../../../libs/imgui
		../../../libs/imgui/misc/stl
		../../../libs/spdlog/include
		../../shared
		..
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../libs/glad/1_1/glad.h
		../../../libs/imgui/imconfig.h
		../../../libs/imgui/imgui.h
		../../../libs/imgui/imgui_internal.h
		../../../libs/imgui/imstb_rectpack.h
		../../../libs/imgui/imstb_textedit.h
		../../../libs/imgui/imstb_truetype.h
		../../../libs/imgui/misc/stl/imgui_stl.h
		../ltjs_launcher.h
		../ltjs_launcher_resource_strings.h
		../ltjs_launcher_search_paths.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../libs/glad/1_1/glad.cpp
		../../../libs/imgui/imgui.cpp
		../../../libs/imgui/imgui_demo.cpp
		../../../libs/imgui/imgui_draw.cpp
		../../../libs/imgui/imgui_widgets.cpp
		../../../libs/imgui/misc/stl/imgui_stl.cpp
		../ltjs_launcher.cpp
		../ltjs_launcher_resource_strings.cpp
		../ltjs_launcher_search_paths.cpp
)

if (WIN32)
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			res/ltjs_launcher.rc
	)
endif ()


install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION
		"."
)

install (
	DIRECTORY
		"ltjs"
	DESTINATION
		"."
)
