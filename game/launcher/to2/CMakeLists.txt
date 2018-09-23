cmake_minimum_required(VERSION 3.5.1)
project(ltjs_nolf2_launcher VERSION 0.0.1 LANGUAGES CXX)

if (NOT WIN32)
	message(FATAL_ERROR "Windows only.")
endif ()

include(ltjs_common)

if (WIN32)
	find_package(LTJS_SDL2 REQUIRED 2.0.8)
else ()
	find_package(LTJS_SDL2 REQUIRED 2.0.4)
endif ()

add_executable(ltjs_nolf2_launcher)

target_include_directories(
	ltjs_nolf2_launcher
	PRIVATE
		${LTJS_SDL2_INCLUDE_DIR}
		../../../libs/glad/1_1
		../../../libs/imgui
		..
)

target_sources(
	ltjs_nolf2_launcher
	PRIVATE
		../../../libs/glad/1_1/glad.h
		../../../libs/imgui/imconfig.h
		../../../libs/imgui/imgui.h
		../../../libs/imgui/imgui_internal.h
		../../../libs/imgui/imstb_rectpack.h
		../../../libs/imgui/imstb_textedit.h
		../../../libs/imgui/imstb_truetype.h
		../ltjs_launcher.h
)

target_sources(
	ltjs_nolf2_launcher
	PRIVATE
		../../../libs/glad/1_1/glad.cpp
		../../../libs/imgui/imgui.cpp
		../../../libs/imgui/imgui_demo.cpp
		../../../libs/imgui/imgui_draw.cpp
		../../../libs/imgui/imgui_widgets.cpp
		../ltjs_launcher.cpp
)

ltjs_add_defaults(ltjs_nolf2_launcher)

set_target_properties(
	ltjs_nolf2_launcher
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(
	ltjs_nolf2_launcher
	PROPERTIES
		OUTPUT_NAME ltjs_nolf2_launcher
)

if (MSVC)
	set_target_properties(
		ltjs_nolf2_launcher
		PROPERTIES
			LINK_FLAGS /SUBSYSTEM:CONSOLE
	)
endif ()

target_link_libraries(
	ltjs_nolf2_launcher
	${LTJS_SDL2_LIBRARY}
)

install(
	TARGETS
	ltjs_nolf2_launcher
	RUNTIME DESTINATION "."
)
