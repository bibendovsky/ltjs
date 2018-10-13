cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_launchers VERSION 0.0.1)

if (NOT WIN32)
	message(FATAL_ERROR "Windows only.")
endif ()

add_subdirectory(to2)
