cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_lib_mfc_stub VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../stdlith
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		mfcs_misc.h
		mfcs_point.h
		mfcs_rect.h
		mfcs_string.h
		mfcs_types.h
		mfcstub.h
		stdafx.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		mfcs_point.cpp
		mfcs_rect.cpp
		mfcs_string.cpp
)

