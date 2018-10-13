cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(ltjs_lib_lith VERSION 0.0.1 LANGUAGES CXX)

include(ltjs_common)

add_library(ltjs_lib_lith STATIC)

ltjs_add_defaults(ltjs_lib_lith)

target_sources(
	ltjs_lib_lith
	PRIVATE
		basehash.h
		baselist.h
		baselistcounter.h
		dprintf.h
		lith.h
		lithbaselist.h
		lithchunkallocator.h
		lithsimpary.h
		lithsimparystat.h
		lithtmpl.h
		lithtypes.h
		virtlist.h
)

target_sources(
	ltjs_lib_lith
	PRIVATE
		basehash.cpp
		baselist.cpp
		baselistcounter.cpp
		dprintf.cpp
		lith.cpp
		lithbaselist.cpp
		lithchunkallocator.cpp
		lithsimpary.cpp
		lithsimparystat.cpp
		virtlist.cpp
)
