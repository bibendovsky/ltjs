################
# Common stuff #
################

function(ltjs_add_default_options)
    if (${ARGC} GREATER 1)
        message(FATAL_ERROR "Too many arguments.")
    endif ()

    if (${ARGC})
        string(LENGTH ${ARGV0} LENGTH)

        if (NOT ${LENGTH})
            message(FATAL_ERROR "Empty PCH file name.")
        endif ()
    endif ()

    # Defines
    # =======

    add_definitions(-DNOPS2)
    add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-D_FINAL>)

    if (MSVC)
        add_compile_options($<$<CONFIG:DEBUG>:-D_CRT_SECURE_NO_WARNINGS>)
        add_compile_options($<$<CONFIG:DEBUG>:-D_ITERATOR_DEBUG_LEVEL=0>)

        # Accept deprecated containers for Visual C++ 14
        add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)
    endif ()


    # Compiler flags
    # ==============

    # Common compiler flags
    # ---------------------

    if (MSVC)
        # Warning Level
        add_compile_options(-W4)

        # Multi-processor Compilation
        add_compile_options(-MP)

        # Eliminate Duplicate Strings
        add_compile_options(-GF)

        # Disable Minimal Rebuild
        add_compile_options(-Gm-)

        # Disable RTTI
        add_compile_options(-GR-)

        # Use Precompiled Headers
        if (${ARGC})
            add_compile_options(-Yu${ARGV0})
        else()
            add_compile_options(-Y-)
        endif ()

        # No Enhanced Instructions
        # (Prevents overflow of the x87 FPU stack)
        add_compile_options(-arch:IA32)

        # Suppress "unreferenced formal parameter" warning
        add_compile_options(-wd4100)

        # Suppress "The POSIX name for this item is deprecated" warning
        add_compile_options(-wd4996)
    endif ()

    if (NOT MSVC)
        add_compile_options("-std=c++11")
        add_compile_options("-Wfatal-errors")
    endif ()

    # ---------------------
    # Debug compile options
    # ---------------------

    if (MSVC)
        # Runtime Library (Multi-threaded Debug)
        add_compile_options($<$<CONFIG:DEBUG>:-MTd>)
    endif ()


    # -----------------------
    # Release compile options
    # -----------------------

    if (MSVC)
        # Runtime Library (Multi-threaded)
        add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-MT>)

        # Enable Intrinsic Function
        add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-Oi>)

        # In-line Function Expansion (Only __inline)
        add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-Ob1>)

        # Favour size or speed (speed)
        add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-Ot>)

        # Omit Frame Pointers
        add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-Oy>)

        # Disable Security Check
        add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:-GS->)
    endif ()

endfunction(ltjs_add_default_options)
