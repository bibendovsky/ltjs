cmake_minimum_required(VERSION 3.5.1)

################
# Common stuff #
################

function (ltjs_add_defaults)
    if (NOT ((${ARGC} EQUAL 1) OR (${ARGC} EQUAL 2)))
        message(FATAL_ERROR "Usage: ltjs_add_defaults <target_name> [<pch_header>]")
    endif ()


    set_target_properties(
        ${ARGV0}
        PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    target_compile_definitions(
        ${ARGV0}
        PRIVATE NOPS2
        PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:_FINAL>
        PRIVATE $<$<CONFIG:DEBUG>:_DEBUG>
    )

    if (MSVC)
        target_compile_definitions(
            ${ARGV0}
            PRIVATE $<$<CONFIG:DEBUG>:_CRT_SECURE_NO_WARNINGS>
            PRIVATE $<$<CONFIG:DEBUG>:_ITERATOR_DEBUG_LEVEL=0>
            PRIVATE _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
        )
    endif ()

	if (WIN32)
		target_compile_definitions(
			${ARGV0}
			PRIVATE NOMINMAX
		)
	endif ()

    if (MSVC)
        target_compile_options(
            ${ARGV0}
            # Warning Level
            PRIVATE -W4
            # Multi-processor Compilation
            PRIVATE -MP
            # No Enhanced Instructions (prevents overflow of the x87 FPU stack)
            PRIVATE -arch:IA32
            # Suppress "unreferenced formal parameter" warning
            PRIVATE -wd4100
            # Suppress "The POSIX name for this item is deprecated" warning
            PRIVATE -wd4996
            # Use Precompiled Headers
            PRIVATE $<$<EQUAL:${ARGC},1>:-Y->
            PRIVATE $<$<EQUAL:${ARGC},2>:-Yu${ARGV1}>
            # Runtime Library (Multi-threaded Debug)
            PRIVATE $<$<CONFIG:DEBUG>:-MTd>
        )
    endif ()

    if (MINGW)
        target_compile_options(
            ${ARGV0}
            # Warning Level
            PRIVATE -Wfatal-errors
        )
    endif ()
endfunction (ltjs_add_defaults)
