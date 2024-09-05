cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)

################
# Common stuff #
################

function (ltjs_add_defaults)
	if (NOT (${ARGC} EQUAL 1))
		message (FATAL_ERROR "Usage: ltjs_add_defaults <target_name>")
	endif ()

	set_target_properties (
		${ARGV0}
		PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
	)

	target_compile_definitions (
		${ARGV0}
		PRIVATE
			NOPS2
			$<$<NOT:$<CONFIG:DEBUG>>:_FINAL>
			$<$<CONFIG:DEBUG>:_DEBUG>
			$<$<BOOL:${LTJS_SDL_BACKEND}>:LTJS_SDL_BACKEND>
			$<$<BOOL:${LTJS_USE_D3DX9}>:LTJS_USE_D3DX9>
	)

	if (MSVC)
		target_compile_definitions (
			${ARGV0}
			PRIVATE
				$<$<CONFIG:DEBUG>:_CRT_SECURE_NO_WARNINGS>
				$<$<CONFIG:DEBUG>:_ITERATOR_DEBUG_LEVEL=0>
				_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
		)
	endif ()

	if (WIN32)
		target_compile_definitions (
			${ARGV0}
			PRIVATE
				NOMINMAX
		)
	endif ()

	if (MSVC)
		target_compile_options (
			${ARGV0}
			PRIVATE
			# Warning Level
				-W4
			# Multi-processor Compilation
				-MP
			# Suppress "unreferenced formal parameter" warning
				-wd4100
			# Suppress "The POSIX name for this item is deprecated" warning
				-wd4996
			# Runtime Library (Multi-threaded Debug)
				$<$<CONFIG:DEBUG>:-MTd>
		)

		if (CMAKE_SIZEOF_VOID_P EQUAL 4)
			target_compile_options (
				${ARGV0}
				PRIVATE
				# No Enhanced Instructions (prevents overflow of the x87 FPU stack)
					-arch:IA32
			)
		endif ()
	endif ()

	if (MINGW)
		target_compile_options (
			${ARGV0}
			# Warning Level
			PRIVATE
				-Wfatal-errors
		)
	endif ()

	target_link_libraries (
		${ARGV0}
		PRIVATE
			SDL2W::SDL2Wmain
			SDL2W::SDL2W
	)

	if (NOT ${PROJECT_NAME} STREQUAL "ltjs_lib")
		target_link_libraries (
			${ARGV0}
			PRIVATE
				ltjs_lib
		)
	endif ()
endfunction (ltjs_add_defaults)
