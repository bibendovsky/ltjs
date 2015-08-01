set(LTJS_TARGET ltjs_lib_lt_gui_mgr)

include(ltjs_common)

ltjs_add_default_options()

add_definitions(-D_LITHTECH2)
add_definitions(-DLT15_COMPAT)

include_directories(
    ../../../engine/sdk/inc
    ../../../engine/sdk/inc/compat
)

set(
    HDRS
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
    SRCS
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
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)
