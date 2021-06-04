cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_sound_drivers VERSION 0.0.1 LANGUAGES CXX)


if (WIN32)
	add_subdirectory (s_dx8)
endif ()

add_subdirectory (s_nul)
add_subdirectory (s_oal)
