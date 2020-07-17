cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_ilt_sound VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		USE_ABSTRACT_SOUND_INTERFACES
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../../../sdk/inc
		../../../../libs/bibendovsky_spul_lib/include
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		iltsound.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		iltsound.cpp
)
