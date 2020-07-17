cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_std_lith VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../zlib
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		abstractio.h
		copy_lists.h
		dynarray.h
		fastlinklist.h
		fileio.h
		genhash.h
		genlist.h
		glink.h
		goodlinklist.h
		helpers.h
		l_allocator.h
		linklist.h
		lithexception.h
		memory.h
		memoryio.h
		morect.h
		multilinklist.h
		object_bank.h
		stdafx.h
		stdlith.h
		stdlithdefs.h
		stringholder.h
		struct_bank.h
		zfstream.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		abstractio.cpp
		dynarray.cpp
		fileio.cpp
		goodlinklist.cpp
		helpers.cpp
		l_allocator.cpp
		linklist.cpp
		lithexception.cpp
		memory.cpp
		memoryio.cpp
		stringholder.cpp
		struct_bank.cpp
		zfstream.cpp
)
