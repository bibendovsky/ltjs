cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_lith VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_sources (
	${PROJECT_NAME}
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

target_sources (
	${PROJECT_NAME}
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
