cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_lib_bute_mgr VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		_NOMFC
)

if (WIN32)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			WINVER=0x0500
	)
endif ()

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../cryptmgr
		../lith
		../mfcstub
		../stdlith
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		arange.h
		avector.h
		butemgr.h
		stdafx.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		arange.cpp
		avector.cpp
		butemgr.cpp
		stdafx.cpp
)
