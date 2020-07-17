cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_lt_gui_mgr VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		_LITHTECH2
		LT15_COMPAT
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../../../engine/sdk/inc
		../../../engine/sdk/inc/compat
)

target_sources (
	${PROJECT_NAME}
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

target_sources (
	${PROJECT_NAME}
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
