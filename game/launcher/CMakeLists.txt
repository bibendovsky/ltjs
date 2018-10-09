cmake_minimum_required(VERSION 3.5.1)
project(ltjs_launchers VERSION 0.0.1)

if (NOT WIN32)
	message(FATAL_ERROR "Windows only.")
endif ()

add_subdirectory(to2)
