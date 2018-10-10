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
		../../../libs/imgui/misc/stl
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
		../../../libs/imgui/misc/stl/imgui_stl.h
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
		../../../libs/imgui/misc/stl/imgui_stl.cpp
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

target_compile_definitions(
	ltjs_nolf2_launcher
	PRIVATE
		LTJS_NOLF2
)

set_target_properties(
	ltjs_nolf2_launcher
	PROPERTIES
		OUTPUT_NAME ltjs_nolf2
)

if (MSVC)
	set_target_properties(
		ltjs_nolf2_launcher
		PROPERTIES
			LINK_FLAGS /SUBSYSTEM:WINDOWS
	)
endif ()

if (MINGW)
	target_link_libraries(
		ltjs_nolf2_launcher
		mingw32
	)
endif ()

target_link_libraries(
	ltjs_nolf2_launcher
	${LTJS_SDL2_LIBRARY}
	bibendovsky_spul_lib
)

if (WIN32)
	target_link_libraries(
		ltjs_nolf2_launcher
		imm32
		version
		winmm
	)
endif ()

install(
	TARGETS
	ltjs_nolf2_launcher
	RUNTIME DESTINATION "."
)

#
# Supported languages.
#
install(
	FILES
		res/languages.txt
	DESTINATION
		ltjs/nolf2/launcher
)

#
# Fonts.
#
install(
	FILES
		res/fonts/noto_sans_display_condensed.ttf
		res/fonts/noto_sans_display_condensed_bold.ttf
		res/fonts/noto_sans_display_condensed_license_ofl.txt
		res/fonts/noto_sans_display_condensed_readme.txt
	DESTINATION
		ltjs/nolf2/launcher/fonts
)

#
# Images (common).
#
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

#
# Images (German).
#
install(
	FILES
		res/lang/de/bmp/canceld.bmp
		res/lang/de/bmp/cancelf.bmp
		res/lang/de/bmp/cancelu.bmp
		res/lang/de/bmp/company1webd.bmp
		res/lang/de/bmp/company1webf.bmp
		res/lang/de/bmp/company1webu.bmp
		res/lang/de/bmp/company2webd.bmp
		res/lang/de/bmp/company2webf.bmp
		res/lang/de/bmp/company2webu.bmp
		res/lang/de/bmp/custombackground.bmp
		res/lang/de/bmp/customd.bmp
		res/lang/de/bmp/customf.bmp
		res/lang/de/bmp/customu.bmp
		res/lang/de/bmp/customx.bmp
		res/lang/de/bmp/demomainappbackground.bmp
		res/lang/de/bmp/detailsettingsbackground.bmp
		res/lang/de/bmp/displaybackground.bmp
		res/lang/de/bmp/displayd.bmp
		res/lang/de/bmp/displayf.bmp
		res/lang/de/bmp/displayu.bmp
		res/lang/de/bmp/displayx.bmp
		res/lang/de/bmp/highdetaild.bmp
		res/lang/de/bmp/highdetailf.bmp
		res/lang/de/bmp/highdetailu.bmp
		res/lang/de/bmp/installd.bmp
		res/lang/de/bmp/installf.bmp
		res/lang/de/bmp/installu.bmp
		res/lang/de/bmp/lowdetaild.bmp
		res/lang/de/bmp/lowdetailf.bmp
		res/lang/de/bmp/lowdetailu.bmp
		res/lang/de/bmp/mainappbackground.bmp
		res/lang/de/bmp/mediumdetaild.bmp
		res/lang/de/bmp/mediumdetailf.bmp
		res/lang/de/bmp/mediumdetailu.bmp
		res/lang/de/bmp/nextd.bmp
		res/lang/de/bmp/nextf.bmp
		res/lang/de/bmp/nextu.bmp
		res/lang/de/bmp/nextx.bmp
		res/lang/de/bmp/okd.bmp
		res/lang/de/bmp/okf.bmp
		res/lang/de/bmp/oku.bmp
		res/lang/de/bmp/optionsbackground.bmp
		res/lang/de/bmp/optionsd.bmp
		res/lang/de/bmp/optionsf.bmp
		res/lang/de/bmp/optionsu.bmp
		res/lang/de/bmp/optionsx.bmp
		res/lang/de/bmp/playd.bmp
		res/lang/de/bmp/playf.bmp
		res/lang/de/bmp/playu.bmp
		res/lang/de/bmp/previousd.bmp
		res/lang/de/bmp/previousf.bmp
		res/lang/de/bmp/previousu.bmp
		res/lang/de/bmp/previousx.bmp
		res/lang/de/bmp/publisher1webd.bmp
		res/lang/de/bmp/publisher1webf.bmp
		res/lang/de/bmp/publisher1webu.bmp
		res/lang/de/bmp/publisher2webd.bmp
		res/lang/de/bmp/publisher2webf.bmp
		res/lang/de/bmp/publisher2webu.bmp
		res/lang/de/bmp/quitd.bmp
		res/lang/de/bmp/quitf.bmp
		res/lang/de/bmp/quitu.bmp
		res/lang/de/bmp/serverd.bmp
		res/lang/de/bmp/serverf.bmp
		res/lang/de/bmp/serveru.bmp
		res/lang/de/bmp/serverx.bmp
	DESTINATION
		ltjs/nolf2/launcher/images/de
)

#
# Images (English).
#
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

#
# Images (Spanish).
#
install(
	FILES
		res/lang/es/bmp/canceld.bmp
		res/lang/es/bmp/cancelf.bmp
		res/lang/es/bmp/cancelu.bmp
		res/lang/es/bmp/company1webd.bmp
		res/lang/es/bmp/company1webf.bmp
		res/lang/es/bmp/company1webu.bmp
		res/lang/es/bmp/company2webd.bmp
		res/lang/es/bmp/company2webf.bmp
		res/lang/es/bmp/company2webu.bmp
		res/lang/es/bmp/custombackground.bmp
		res/lang/es/bmp/customd.bmp
		res/lang/es/bmp/customf.bmp
		res/lang/es/bmp/customu.bmp
		res/lang/es/bmp/customx.bmp
		res/lang/es/bmp/demomainappbackground.bmp
		res/lang/es/bmp/detailsettingsbackground.bmp
		res/lang/es/bmp/displaybackground.bmp
		res/lang/es/bmp/displayd.bmp
		res/lang/es/bmp/displayf.bmp
		res/lang/es/bmp/displayu.bmp
		res/lang/es/bmp/displayx.bmp
		res/lang/es/bmp/highdetaild.bmp
		res/lang/es/bmp/highdetailf.bmp
		res/lang/es/bmp/highdetailu.bmp
		res/lang/es/bmp/installd.bmp
		res/lang/es/bmp/installf.bmp
		res/lang/es/bmp/installu.bmp
		res/lang/es/bmp/lowdetaild.bmp
		res/lang/es/bmp/lowdetailf.bmp
		res/lang/es/bmp/lowdetailu.bmp
		res/lang/es/bmp/mainappbackground.bmp
		res/lang/es/bmp/mediumdetaild.bmp
		res/lang/es/bmp/mediumdetailf.bmp
		res/lang/es/bmp/mediumdetailu.bmp
		res/lang/es/bmp/nextd.bmp
		res/lang/es/bmp/nextf.bmp
		res/lang/es/bmp/nextu.bmp
		res/lang/es/bmp/nextx.bmp
		res/lang/es/bmp/okd.bmp
		res/lang/es/bmp/okf.bmp
		res/lang/es/bmp/oku.bmp
		res/lang/es/bmp/optionsbackground.bmp
		res/lang/es/bmp/optionsd.bmp
		res/lang/es/bmp/optionsf.bmp
		res/lang/es/bmp/optionsu.bmp
		res/lang/es/bmp/optionsx.bmp
		res/lang/es/bmp/playd.bmp
		res/lang/es/bmp/playf.bmp
		res/lang/es/bmp/playu.bmp
		res/lang/es/bmp/previousd.bmp
		res/lang/es/bmp/previousf.bmp
		res/lang/es/bmp/previousu.bmp
		res/lang/es/bmp/previousx.bmp
		res/lang/es/bmp/publisher1webd.bmp
		res/lang/es/bmp/publisher1webf.bmp
		res/lang/es/bmp/publisher1webu.bmp
		res/lang/es/bmp/publisher2webd.bmp
		res/lang/es/bmp/publisher2webf.bmp
		res/lang/es/bmp/publisher2webu.bmp
		res/lang/es/bmp/quitd.bmp
		res/lang/es/bmp/quitf.bmp
		res/lang/es/bmp/quitu.bmp
		res/lang/es/bmp/serverd.bmp
		res/lang/es/bmp/serverf.bmp
		res/lang/es/bmp/serveru.bmp
		res/lang/es/bmp/serverx.bmp
	DESTINATION
		ltjs/nolf2/launcher/images/es
)

#
# Images (French).
#
install(
	FILES
		res/lang/fr/bmp/canceld.bmp
		res/lang/fr/bmp/cancelf.bmp
		res/lang/fr/bmp/cancelu.bmp
		res/lang/fr/bmp/company1webd.bmp
		res/lang/fr/bmp/company1webf.bmp
		res/lang/fr/bmp/company1webu.bmp
		res/lang/fr/bmp/company2webd.bmp
		res/lang/fr/bmp/company2webf.bmp
		res/lang/fr/bmp/company2webu.bmp
		res/lang/fr/bmp/custombackground.bmp
		res/lang/fr/bmp/customd.bmp
		res/lang/fr/bmp/customf.bmp
		res/lang/fr/bmp/customu.bmp
		res/lang/fr/bmp/customx.bmp
		res/lang/fr/bmp/demomainappbackground.bmp
		res/lang/fr/bmp/detailsettingsbackground.bmp
		res/lang/fr/bmp/displaybackground.bmp
		res/lang/fr/bmp/displayd.bmp
		res/lang/fr/bmp/displayf.bmp
		res/lang/fr/bmp/displayu.bmp
		res/lang/fr/bmp/displayx.bmp
		res/lang/fr/bmp/highdetaild.bmp
		res/lang/fr/bmp/highdetailf.bmp
		res/lang/fr/bmp/highdetailu.bmp
		res/lang/fr/bmp/installd.bmp
		res/lang/fr/bmp/installf.bmp
		res/lang/fr/bmp/installu.bmp
		res/lang/fr/bmp/lowdetaild.bmp
		res/lang/fr/bmp/lowdetailf.bmp
		res/lang/fr/bmp/lowdetailu.bmp
		res/lang/fr/bmp/mainappbackground.bmp
		res/lang/fr/bmp/mediumdetaild.bmp
		res/lang/fr/bmp/mediumdetailf.bmp
		res/lang/fr/bmp/mediumdetailu.bmp
		res/lang/fr/bmp/nextd.bmp
		res/lang/fr/bmp/nextf.bmp
		res/lang/fr/bmp/nextu.bmp
		res/lang/fr/bmp/nextx.bmp
		res/lang/fr/bmp/okd.bmp
		res/lang/fr/bmp/okf.bmp
		res/lang/fr/bmp/oku.bmp
		res/lang/fr/bmp/optionsbackground.bmp
		res/lang/fr/bmp/optionsd.bmp
		res/lang/fr/bmp/optionsf.bmp
		res/lang/fr/bmp/optionsu.bmp
		res/lang/fr/bmp/optionsx.bmp
		res/lang/fr/bmp/playd.bmp
		res/lang/fr/bmp/playf.bmp
		res/lang/fr/bmp/playu.bmp
		res/lang/fr/bmp/previousd.bmp
		res/lang/fr/bmp/previousf.bmp
		res/lang/fr/bmp/previousu.bmp
		res/lang/fr/bmp/previousx.bmp
		res/lang/fr/bmp/publisher1webd.bmp
		res/lang/fr/bmp/publisher1webf.bmp
		res/lang/fr/bmp/publisher1webu.bmp
		res/lang/fr/bmp/publisher2webd.bmp
		res/lang/fr/bmp/publisher2webf.bmp
		res/lang/fr/bmp/publisher2webu.bmp
		res/lang/fr/bmp/quitd.bmp
		res/lang/fr/bmp/quitf.bmp
		res/lang/fr/bmp/quitu.bmp
		res/lang/fr/bmp/serverd.bmp
		res/lang/fr/bmp/serverf.bmp
		res/lang/fr/bmp/serveru.bmp
		res/lang/fr/bmp/serverx.bmp
	DESTINATION
		ltjs/nolf2/launcher/images/fr
)

#
# Images (Italian).
#
install(
	FILES
		res/lang/it/bmp/canceld.bmp
		res/lang/it/bmp/cancelf.bmp
		res/lang/it/bmp/cancelu.bmp
		res/lang/it/bmp/company1webd.bmp
		res/lang/it/bmp/company1webf.bmp
		res/lang/it/bmp/company1webu.bmp
		res/lang/it/bmp/company2webd.bmp
		res/lang/it/bmp/company2webf.bmp
		res/lang/it/bmp/company2webu.bmp
		res/lang/it/bmp/custombackground.bmp
		res/lang/it/bmp/customd.bmp
		res/lang/it/bmp/customf.bmp
		res/lang/it/bmp/customu.bmp
		res/lang/it/bmp/customx.bmp
		res/lang/it/bmp/demomainappbackground.bmp
		res/lang/it/bmp/detailsettingsbackground.bmp
		res/lang/it/bmp/displaybackground.bmp
		res/lang/it/bmp/displayd.bmp
		res/lang/it/bmp/displayf.bmp
		res/lang/it/bmp/displayu.bmp
		res/lang/it/bmp/displayx.bmp
		res/lang/it/bmp/highdetaild.bmp
		res/lang/it/bmp/highdetailf.bmp
		res/lang/it/bmp/highdetailu.bmp
		res/lang/it/bmp/installd.bmp
		res/lang/it/bmp/installf.bmp
		res/lang/it/bmp/installu.bmp
		res/lang/it/bmp/lowdetaild.bmp
		res/lang/it/bmp/lowdetailf.bmp
		res/lang/it/bmp/lowdetailu.bmp
		res/lang/it/bmp/mainappbackground.bmp
		res/lang/it/bmp/mediumdetaild.bmp
		res/lang/it/bmp/mediumdetailf.bmp
		res/lang/it/bmp/mediumdetailu.bmp
		res/lang/it/bmp/nextd.bmp
		res/lang/it/bmp/nextf.bmp
		res/lang/it/bmp/nextu.bmp
		res/lang/it/bmp/nextx.bmp
		res/lang/it/bmp/okd.bmp
		res/lang/it/bmp/okf.bmp
		res/lang/it/bmp/oku.bmp
		res/lang/it/bmp/optionsbackground.bmp
		res/lang/it/bmp/optionsd.bmp
		res/lang/it/bmp/optionsf.bmp
		res/lang/it/bmp/optionsu.bmp
		res/lang/it/bmp/optionsx.bmp
		res/lang/it/bmp/playd.bmp
		res/lang/it/bmp/playf.bmp
		res/lang/it/bmp/playu.bmp
		res/lang/it/bmp/previousd.bmp
		res/lang/it/bmp/previousf.bmp
		res/lang/it/bmp/previousu.bmp
		res/lang/it/bmp/previousx.bmp
		res/lang/it/bmp/publisher1webd.bmp
		res/lang/it/bmp/publisher1webf.bmp
		res/lang/it/bmp/publisher1webu.bmp
		res/lang/it/bmp/publisher2webd.bmp
		res/lang/it/bmp/publisher2webf.bmp
		res/lang/it/bmp/publisher2webu.bmp
		res/lang/it/bmp/quitd.bmp
		res/lang/it/bmp/quitf.bmp
		res/lang/it/bmp/quitu.bmp
		res/lang/it/bmp/serverd.bmp
		res/lang/it/bmp/serverf.bmp
		res/lang/it/bmp/serveru.bmp
		res/lang/it/bmp/serverx.bmp
	DESTINATION
		ltjs/nolf2/launcher/images/it
)

#
# Strings (common).
#
install(
	FILES
		res/strings/launcher.txt
	DESTINATION
		ltjs/nolf2/launcher/strings
)

#
# Strings (Germany).
#
install(
	FILES
		res/lang/de/strings/launcher.txt
	DESTINATION
		ltjs/nolf2/launcher/strings/de
)

#
# Strings (English).
#
install(
	FILES
		res/lang/en/strings/launcher.txt
	DESTINATION
		ltjs/nolf2/launcher/strings/en
)

#
# Strings (Spanish).
#
install(
	FILES
		res/lang/es/strings/launcher.txt
	DESTINATION
		ltjs/nolf2/launcher/strings/es
)

#
# Strings (French).
#
install(
	FILES
		res/lang/fr/strings/launcher.txt
	DESTINATION
		ltjs/nolf2/launcher/strings/fr
)

#
# Strings (Italian).
#
install(
	FILES
		res/lang/it/strings/launcher.txt
	DESTINATION
		ltjs/nolf2/launcher/strings/it
)
