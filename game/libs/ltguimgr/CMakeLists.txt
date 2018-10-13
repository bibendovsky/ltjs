cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_lt_gui_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_lt_gui_mgr STATIC)

ltjs_add_defaults(ltjs_lib_lt_gui_mgr)

target_compile_definitions(
	ltjs_lib_lt_gui_mgr
	PRIVATE
		_LITHTECH2
		LT15_COMPAT
)

target_include_directories(
	ltjs_lib_lt_gui_mgr
	PRIVATE
		../../../engine/sdk/inc
		../../../engine/sdk/inc/compat
)

target_sources(
	ltjs_lib_lt_gui_mgr
	PRIVATE
		ltguibutton.h
		ltguicolumnctrl.h
		ltguicommandhandler.h
		ltguictrl.h
		ltguicyclectrl.h
		ltguieditctrl.h
		ltguiframe.h
		ltguilistctrl.h
		ltguimgr.h
		ltguislider.h
		ltguitextitemctrl.h
		ltguitoggle.h
		ltguiwindow.h
		ltquaduvutils.h
		vkdefs.h
		ltguilargetext.h
)

target_sources(
	ltjs_lib_lt_gui_mgr
	PRIVATE
		ltguibutton.cpp
		ltguicolumnctrl.cpp
		ltguicommandhandler.cpp
		ltguictrl.cpp
		ltguicyclectrl.cpp
		ltguieditctrl.cpp
		ltguiframe.cpp
		ltguilistctrl.cpp
		ltguislider.cpp
		ltguitextitemctrl.cpp
		ltguitoggle.cpp
		ltguiwindow.cpp
		ltguilargetext.cpp
)
