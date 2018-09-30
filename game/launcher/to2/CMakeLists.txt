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
		../../../libs/bibendovsky_spul_lib/include
		../../../libs/glad/1_1
		../../../libs/imgui
		../../shared
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
		../../shared/ltjs_resource_strings.h
		../../shared/ltjs_script_tokenizer.h
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
		../../shared/ltjs_resource_strings.cpp
		../../shared/ltjs_script_tokenizer.cpp
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
			LINK_FLAGS /SUBSYSTEM:WINDOWS
	)
endif ()

target_link_libraries(
	ltjs_nolf2_launcher
	${LTJS_SDL2_LIBRARY}
	bibendovsky_spul_lib
)

install(
	TARGETS
	ltjs_nolf2_launcher
	RUNTIME DESTINATION "."
)

install(
	FILES
		res/fonts/noto_sans_display_condensed.ttf
		res/fonts/noto_sans_display_condensed_bold.ttf
		res/fonts/noto_sans_display_condensed_license_ofl.txt
		res/fonts/noto_sans_display_condensed_readme.txt
	DESTINATION
		ltjs/nolf2/launcher/fonts
)

install(
	FILES
		res/bmp/boxbackground.bmp
		res/bmp/checkboxc.bmp
		res/bmp/checkboxf.bmp
		res/bmp/checkboxn.bmp
		res/bmp/closed.bmp
		res/bmp/closeu.bmp
		res/bmp/error.bmp
		res/bmp/information.bmp
		res/bmp/minimized.bmp
		res/bmp/minimizeu.bmp
		res/bmp/warning.bmp
	DESTINATION
		ltjs/nolf2/launcher/images
)

install(
	FILES
		res/lang/en/bmp/canceld.bmp
		res/lang/en/bmp/cancelf.bmp
		res/lang/en/bmp/cancelu.bmp
		res/lang/en/bmp/company1webd.bmp
		res/lang/en/bmp/company1webf.bmp
		res/lang/en/bmp/company1webu.bmp
		res/lang/en/bmp/company2webd.bmp
		res/lang/en/bmp/company2webf.bmp
		res/lang/en/bmp/company2webu.bmp
		res/lang/en/bmp/custombackground.bmp
		res/lang/en/bmp/customd.bmp
		res/lang/en/bmp/customf.bmp
		res/lang/en/bmp/customu.bmp
		res/lang/en/bmp/customx.bmp
		res/lang/en/bmp/demomainappbackground.bmp
		res/lang/en/bmp/detailsettingsbackground.bmp
		res/lang/en/bmp/displaybackground.bmp
		res/lang/en/bmp/displayd.bmp
		res/lang/en/bmp/displayf.bmp
		res/lang/en/bmp/displayu.bmp
		res/lang/en/bmp/displayx.bmp
		res/lang/en/bmp/highdetaild.bmp
		res/lang/en/bmp/highdetailf.bmp
		res/lang/en/bmp/highdetailu.bmp
		res/lang/en/bmp/installd.bmp
		res/lang/en/bmp/installf.bmp
		res/lang/en/bmp/installu.bmp
		res/lang/en/bmp/lowdetaild.bmp
		res/lang/en/bmp/lowdetailf.bmp
		res/lang/en/bmp/lowdetailu.bmp
		res/lang/en/bmp/mainappbackground.bmp
		res/lang/en/bmp/mediumdetaild.bmp
		res/lang/en/bmp/mediumdetailf.bmp
		res/lang/en/bmp/mediumdetailu.bmp
		res/lang/en/bmp/nextd.bmp
		res/lang/en/bmp/nextf.bmp
		res/lang/en/bmp/nextu.bmp
		res/lang/en/bmp/nextx.bmp
		res/lang/en/bmp/okd.bmp
		res/lang/en/bmp/okf.bmp
		res/lang/en/bmp/oku.bmp
		res/lang/en/bmp/optionsbackground.bmp
		res/lang/en/bmp/optionsd.bmp
		res/lang/en/bmp/optionsf.bmp
		res/lang/en/bmp/optionsu.bmp
		res/lang/en/bmp/optionsx.bmp
		res/lang/en/bmp/playd.bmp
		res/lang/en/bmp/playf.bmp
		res/lang/en/bmp/playu.bmp
		res/lang/en/bmp/previousd.bmp
		res/lang/en/bmp/previousf.bmp
		res/lang/en/bmp/previousu.bmp
		res/lang/en/bmp/previousx.bmp
		res/lang/en/bmp/publisher1webd.bmp
		res/lang/en/bmp/publisher1webf.bmp
		res/lang/en/bmp/publisher1webu.bmp
		res/lang/en/bmp/publisher2webd.bmp
		res/lang/en/bmp/publisher2webf.bmp
		res/lang/en/bmp/publisher2webu.bmp
		res/lang/en/bmp/quitd.bmp
		res/lang/en/bmp/quitf.bmp
		res/lang/en/bmp/quitu.bmp
		res/lang/en/bmp/serverd.bmp
		res/lang/en/bmp/serverf.bmp
		res/lang/en/bmp/serveru.bmp
		res/lang/en/bmp/serverx.bmp
	DESTINATION
		ltjs/nolf2/launcher/images/en
)

install(
	FILES
		res/lang/en/strings/launcher.txt
	DESTINATION
		ltjs/nolf2/launcher/strings/en
)
