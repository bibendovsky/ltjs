cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(bibendovsky_spul_lib VERSION 1.0.1 LANGUAGES CXX)


option(BIBENDOVSKY_SPUL_USE_PCH "Use precompiled headers if available." OFF)
option(BIBENDOVSKY_SPUL_STATIC_CRT "Use static runtime library if available." OFF)
option(BIBENDOVSKY_SPUL_VC_DISABLE_CHECKED_ITERATORS "Disables checked iterators and disables iterator debugging (Visual C++)." OFF)


set(
	headers
	include/bibendovsky_spul_algorithm.h
	include/bibendovsky_spul_ascii_utils.h
	include/bibendovsky_spul_encoding_utils.h
	include/bibendovsky_spul_endian.h
	include/bibendovsky_spul_enum_flags.h
	include/bibendovsky_spul_file_stream.h
	include/bibendovsky_spul_four_cc.h
	include/bibendovsky_spul_memory_stream.h
	include/bibendovsky_spul_path_utils.h
	include/bibendovsky_spul_platform.h
	include/bibendovsky_spul_precompiled.h
	include/bibendovsky_spul_riff_four_ccs.h
	include/bibendovsky_spul_riff_reader.h
	include/bibendovsky_spul_scope_guard.h
	include/bibendovsky_spul_stream.h
	include/bibendovsky_spul_substream.h
	include/bibendovsky_spul_utf8_utils.h
	include/bibendovsky_spul_wave_format.h
	include/bibendovsky_spul_wave_format_utils.h
	include/bibendovsky_spul_wave_format_tag.h
	include/bibendovsky_spul_wave_four_ccs.h
	include/bibendovsky_spul_wchar_utils.h
)

set(
	sources
	src/bibendovsky_spul_ascii_utils.cpp
	src/bibendovsky_spul_encoding_utils.cpp
	src/bibendovsky_spul_file_stream.cpp
	src/bibendovsky_spul_memory_stream.cpp
	src/bibendovsky_spul_path_utils.cpp
	src/bibendovsky_spul_precompiled.cpp
	src/bibendovsky_spul_riff_four_ccs.cpp
	src/bibendovsky_spul_riff_reader.cpp
	src/bibendovsky_spul_stream.cpp
	src/bibendovsky_spul_substream.cpp
	src/bibendovsky_spul_utf8_utils.cpp
	src/bibendovsky_spul_wave_format_utils.cpp
	src/bibendovsky_spul_wave_four_ccs.cpp
	src/bibendovsky_spul_wchar_utils.cpp
)

if (BIBENDOVSKY_SPUL_USE_PCH)
	if (MSVC)
		set_source_files_properties(
			src/bibendovsky_spul_precompiled.cpp
			PROPERTIES
				COMPILE_FLAGS -Yc
		)
	endif ()
endif ()

add_library(
	bibendovsky_spul_lib STATIC
	${sources}
	${headers}
)

target_include_directories(
	bibendovsky_spul_lib
	PRIVATE include
)

set_target_properties(
	bibendovsky_spul_lib
	PROPERTIES
	PUBLIC_HEADER "${headers}"
)

if (BIBENDOVSKY_SPUL_USE_PCH)
	if (MSVC)
		target_compile_definitions(
			bibendovsky_spul_lib
			PRIVATE -DBIBENDOVSKY_SPUL_USE_PCH
		)

		target_compile_options(
			bibendovsky_spul_lib
			PRIVATE -Yubibendovsky_spul_precompiled.h
		)
	endif ()
endif ()

if (BIBENDOVSKY_SPUL_VC_DISABLE_CHECKED_ITERATORS)
	if (MSVC)
		target_compile_definitions(
			bibendovsky_spul_lib
			PRIVATE $<$<CONFIG:DEBUG>:_ITERATOR_DEBUG_LEVEL=0>
		)

		target_compile_options(
			bibendovsky_spul_lib
			PRIVATE -Yubibendovsky_spul_precompiled.h
		)
	endif ()
endif ()

if (BIBENDOVSKY_SPUL_STATIC_CRT)
	if (MSVC)
		target_compile_options(
			bibendovsky_spul_lib
			PRIVATE $<$<CONFIG:DEBUG>:-MTd>
			PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:-MT>
		)
	endif ()
endif ()

set_target_properties(
	bibendovsky_spul_lib
	PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(
	bibendovsky_spul_lib
	PROPERTIES
	OUTPUT_NAME "bibendovsky_spul"
	PROJECT_LABEL "bibendovsky_spul_lib"
)

install(
	TARGETS
	bibendovsky_spul_lib
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include
)
