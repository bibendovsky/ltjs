cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_ui VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

if (!LTJS_SDL_BACKEND)
message (FATAL_ERROR "WTF")
endif ()

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		__D3D
		__D3DREND
)

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		ltjs_lib_freetype
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../../../libs/lith
		../../../../libs/stdlith
		../../../libs/rezmgr
		../../../sdk/inc
		../../../sdk/inc/compat
		../../client/src
		../../comm/src
		../../controlfilemgr
		../../crtcompat
		../../kernel/io/src
		../../kernel/mem/src
		../../kernel/net/src
		../../kernel/src
		../../kernel/src/sys/win
		../../lithtemplate
		../../model/src
		../../mpm/src
		../../render_b/src
		../../server/src
		../../shared/src
		../../shared/src/sys/win
		../../sound/src
		../../ui/src
		../../world/src
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../sdk/inc/cui.h
		../../../sdk/inc/cuibase.h
		../../../sdk/inc/cuibutton.h
		../../../sdk/inc/cuicheck.h
		../../../sdk/inc/cuidropdownlist.h
		../../../sdk/inc/cuifont.h
		../../../sdk/inc/cuiformattedpolystring.h
		../../../sdk/inc/cuiinterval.h
		../../../sdk/inc/cuilist.h
		../../../sdk/inc/cuimessage.h
		../../../sdk/inc/cuioption.h
		../../../sdk/inc/cuipolystring.h
		../../../sdk/inc/cuiprogress.h
		../../../sdk/inc/cuislider.h
		../../../sdk/inc/cuistaticimage.h
		../../../sdk/inc/cuistatictext.h
		../../../sdk/inc/cuitypes.h
		../../../sdk/inc/cuiwidget.h
		../../../sdk/inc/cuiwindow.h
		../../../sdk/inc/iltfontmanager.h
		../../../sdk/inc/iltwidgetmanager.h
		cuibase_impl.h
		cuibitmapfont.h
		cuibutton_impl.h
		cuicheck_impl.h
		cuidebug.h
		cuidropdownlist_impl.h
		cuifont_impl.h
		cuifontmanager.h
		cuiformattedpolystring_impl.h
		cuiinterval_impl.h
		cuilinklist.h
		cuilist_impl.h
		cuimessagequeue.h
		cuioption_impl.h
		cuipolystring_impl.h
		cuipolytex.h
		cuiprogress_impl.h
		cuiredblack.h
		cuirenderstate.h
		cuislider_impl.h
		cuistaticimage_impl.h
		cuistatictext_impl.h
		cuivectorfont.h
		cuiwidget_impl.h
		cuiwidgetmanager.h
		cuiwindow_impl.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		cuibase.cpp
		cuibase_impl.cpp
		cuibitmapfont.cpp
		cuibutton.cpp
		cuibutton_impl.cpp
		cuicheck.cpp
		cuicheck_impl.cpp
		cuidropdownlist.cpp
		cuidropdownlist_impl.cpp
		cuifont_impl.cpp
		cuifontmanager.cpp
		cuiformattedpolystring.cpp
		cuiformattedpolystring_impl.cpp
		cuiinterval.cpp
		cuiinterval_impl.cpp
		cuilinklist.cpp
		cuilist.cpp
		cuilist_impl.cpp
		cuimessagequeue.cpp
		cuioption.cpp
		cuioption_impl.cpp
		cuipolystring.cpp
		cuipolystring_impl.cpp
		cuipolytex.cpp
		cuiprogress.cpp
		cuiprogress_impl.cpp
		cuiredblack.cpp
		cuirenderstate.cpp
		cuislider.cpp
		cuislider_impl.cpp
		cuistaticimage.cpp
		cuistaticimage_impl.cpp
		cuistatictext.cpp
		cuistatictext_impl.cpp
		cuivectorfont.cpp
		cuiwidget.cpp
		cuiwidget_impl.cpp
		cuiwidgetmanager.cpp
		cuiwindow.cpp
		cuiwindow_impl.cpp
		sdl_cuivectorfont.cpp
		sys/d3d/cuidebug.cpp
)
