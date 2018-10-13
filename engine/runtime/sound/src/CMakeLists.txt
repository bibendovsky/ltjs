cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_ilt_sound VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_ilt_sound STATIC)

ltjs_add_defaults(ltjs_lib_ilt_sound)

target_compile_definitions(
	ltjs_lib_ilt_sound
	PRIVATE
		USE_ABSTRACT_SOUND_INTERFACES
)

target_include_directories(
	ltjs_lib_ilt_sound
	PRIVATE
		../../../sdk/inc
		../../../../libs/bibendovsky_spul_lib/include
)

target_sources(
	ltjs_lib_ilt_sound
	PRIVATE
		iltsound.h
)

target_sources(
	ltjs_lib_ilt_sound
	PRIVATE
		iltsound.cpp
)
