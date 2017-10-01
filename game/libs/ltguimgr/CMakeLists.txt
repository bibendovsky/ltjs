cmake_minimum_required(VERSION 3.5.1)
project(ltjs_lib_lt_gui_mgr VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

include_directories(
    ../../../engine/sdk/inc
    ../../../engine/sdk/inc/compat
)

set(
    headers
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

set(
    sources
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

add_library(
    ltjs_lib_lt_gui_mgr STATIC
    ${headers}
    ${sources}
)

ltjs_add_defaults(ltjs_lib_lt_gui_mgr)

target_compile_definitions(
    ltjs_lib_lt_gui_mgr
    PRIVATE _LITHTECH2
    PRIVATE LT15_COMPAT
)
