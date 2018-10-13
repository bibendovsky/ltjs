cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_mfc_stub VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_mfc_stub STATIC)

ltjs_add_defaults(ltjs_lib_mfc_stub)

target_include_directories(
	ltjs_lib_mfc_stub
	PRIVATE
		../stdlith
)

target_sources(
	ltjs_lib_mfc_stub
	PRIVATE
		mfcs_misc.h
		mfcs_point.h
		mfcs_rect.h
		mfcs_string.h
		mfcs_types.h
		mfcstub.h
		stdafx.h
)

target_sources(
	ltjs_lib_mfc_stub
	PRIVATE
		mfcs_point.cpp
		mfcs_rect.cpp
		mfcs_string.cpp
)

