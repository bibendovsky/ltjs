#
# A very simple "find_package" implementation for SDL2.
#
# User-provided/generated variables:
#
#    - LTJS_SDL2_LIB_NAME - the name of SDL2 library.
#                           Default: SDL2
#    - LTJS_SDL2_MAIN_LIB_NAME - the name of SDL2main library.
#                                Default: SDL2main
#
# Generated variables:
#    - LTJS_SDL2_INCLUDE_DIR - path to SDL2 headers.
#    - LTJS_SDL2_LIBRARY_DIR - path to SDL2 libraries.
#    - LTJS_SDL2_LIBRARY - required, SDL2 and SDL2main libraries for target_link_libraries, etc.
#    - LTJS_SDL2_VERSION - SDL2 detected version.
#


unset(LTJS_SDL2_TMP_IS_INCS_FOUND)
unset(LTJS_SDL2_TMP_IS_LIBS_FOUND)


#
# Headers
#

find_path(
	LTJS_SDL2_INCLUDE_DIR
		SDL.h
	HINTS
		PATH_SUFFIXES
			SDL2
			include/SDL2
)

if (LTJS_SDL2_INCLUDE_DIR)
	set(
		LTJS_SDL2_TMP_VERSION_H
		${LTJS_SDL2_INCLUDE_DIR}/SDL_version.h
	)

	if (EXISTS ${LTJS_SDL2_TMP_VERSION_H})
		set(
			LTJS_SDL2_TMP_DIGIT_REGEX
			"^[0-9]$"
		)

		set(
			LTJS_SDL2_TMP_MAJOR_REGEX
			"^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9])$"
		)

		set(
			LTJS_SDL2_TMP_MINOR_REGEX
			"^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9])$"
		)

		set(
			LTJS_SDL2_TMP_PATCH_REGEX
			"^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9])$"
		)

		file(
			STRINGS
			${LTJS_SDL2_TMP_VERSION_H}
			LTJS_SDL2_MAJOR_VERSION_LINE
			REGEX ${LTJS_SDL2_TMP_MAJOR_REGEX}
		)

		file(
			STRINGS
			${LTJS_SDL2_TMP_VERSION_H}
			LTJS_SDL2_MINOR_VERSION_LINE
			REGEX ${LTJS_SDL2_TMP_MINOR_REGEX}
		)

		file(
			STRINGS
			${LTJS_SDL2_TMP_VERSION_H}
			LTJS_SDL2_PATCH_VERSION_LINE
			REGEX ${LTJS_SDL2_TMP_PATCH_REGEX}
		)

		string(
			REGEX REPLACE
			${LTJS_SDL2_TMP_MAJOR_REGEX}
			"\\1"
			LTJS_SDL2_MAJOR_VERSION
			${LTJS_SDL2_MAJOR_VERSION_LINE}
		)

		string(
			REGEX REPLACE
			${LTJS_SDL2_TMP_MINOR_REGEX}
			"\\1"
			LTJS_SDL2_MINOR_VERSION
			${LTJS_SDL2_MINOR_VERSION_LINE}
		)

		string(
			REGEX REPLACE
			${LTJS_SDL2_TMP_PATCH_REGEX}
			"\\1"
			LTJS_SDL2_PATCH_VERSION
			${LTJS_SDL2_PATCH_VERSION_LINE}
		)

		if (LTJS_SDL2_MAJOR_VERSION MATCHES ${LTJS_SDL2_TMP_DIGIT_REGEX} AND
			LTJS_SDL2_MINOR_VERSION MATCHES ${LTJS_SDL2_TMP_DIGIT_REGEX} AND
			LTJS_SDL2_PATCH_VERSION MATCHES ${LTJS_SDL2_TMP_DIGIT_REGEX}
			)
			set(
				LTJS_SDL2_VERSION_LINE
				${LTJS_SDL2_MAJOR_VERSION}.${LTJS_SDL2_MINOR_VERSION}.${LTJS_SDL2_PATCH_VERSION}
			)
		endif ()
	else ()
		unset(LTJS_SDL2_MAJOR_VERSION_LINE)
		unset(LTJS_SDL2_MINOR_VERSION_LINE)
		unset(LTJS_SDL2_PATCH_VERSION_LINE)
	endif ()

	if (LTJS_SDL2_VERSION_LINE)
		set(LTJS_SDL2_TMP_IS_INCS_FOUND TRUE)
		set(LTJS_SDL2_VERSION ${LTJS_SDL2_VERSION_LINE})
	else ()
		message("Failed to detect SDL2 version.")
	endif ()
else ()
	message("Required SDL2 headers not found.")
endif ()

#
# Libraries
#

unset(LTJS_SDL2_TMP_SDL_LIBRARY)
unset(LTJS_SDL2_TMP_FOUND_LIBS)
unset(LTJS_SDL2_TMP_MISSING_LIBS)

if (NOT LTJS_SDL2_LIB_NAME)
	set(LTJS_SDL2_LIB_NAME SDL2 CACHE STRING "The name of SDL2 library.")
endif ()

if (NOT LTJS_SDL2_MAIN_LIB_NAME)
	set(LTJS_SDL2_MAIN_LIB_NAME SDL2main CACHE STRING "The name of SDL2main library.")
endif ()

find_library(
	LTJS_SDL2_LIBRARY_DIR
	NAMES
		${LTJS_SDL2_LIB_NAME}
	HINTS
		PATH_SUFFIXES
			lib
)

if (LTJS_SDL2_LIBRARY_DIR)
	if (MINGW)
		list(
			APPEND LTJS_SDL2_TMP_SDL_LIBRARY
			mingw32
		)
	endif ()

	if (NOT IS_DIRECTORY ${LTJS_SDL2_LIBRARY_DIR})
		get_filename_component(LTJS_SDL2_LIBRARY_DIR ${LTJS_SDL2_LIBRARY_DIR} DIRECTORY)
		set(LTJS_SDL2_LIBRARY_DIR ${LTJS_SDL2_LIBRARY_DIR} CACHE PATH "SDL2 library directory." FORCE)
	endif ()

	set(
		LTJS_SDL2_TMP_LIB
		${LTJS_SDL2_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${LTJS_SDL2_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
	)

	if (EXISTS ${LTJS_SDL2_TMP_LIB})
		list(APPEND LTJS_SDL2_TMP_FOUND_LIBS ${LTJS_SDL2_TMP_LIB})
	else ()
		list(APPEND LTJS_SDL2_TMP_MISSING_LIBS ${LTJS_SDL2_TMP_LIB})
	endif ()


	if (LTJS_SDL2_TMP_MISSING_LIBS)
		unset(LTJS_SDL2_LIBRARY)

		message("Missing SDL2 library (${LTJS_SDL2_LIB_NAME}).")
		message(FATAL_ERROR "")
	else ()
		set(
			LTJS_SDL2_TMP_LIB
			${LTJS_SDL2_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${LTJS_SDL2_MAIN_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
		)

		if (EXISTS ${LTJS_SDL2_TMP_LIB})
			list(APPEND LTJS_SDL2_TMP_FOUND_LIBS ${LTJS_SDL2_TMP_LIB})
		endif ()

		set(LTJS_SDL2_TMP_IS_LIBS_FOUND "TRUE")

		list(
			APPEND LTJS_SDL2_TMP_SDL_LIBRARY
			${LTJS_SDL2_TMP_FOUND_LIBS}
			Threads::Threads
		)

		if (WIN32)
			list(
				APPEND LTJS_SDL2_TMP_SDL_LIBRARY
				imm32
				version
				winmm
			)
		endif ()

		set(LTJS_SDL2_LIBRARY ${LTJS_SDL2_TMP_SDL_LIBRARY})
	endif ()
endif ()

if (NOT LTJS_SDL2_TMP_IS_INCS_FOUND OR NOT LTJS_SDL2_TMP_IS_LIBS_FOUND)
	message("")
	message("*** SDL2 not found ***")
	message("")
	message(FATAL_ERROR "")
endif ()


#
# Find threading library
#

if (NOT APPLE)
	find_package(Threads)
endif ()


#
# Default handler
#

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	LTJS_SDL2
	REQUIRED_VARS LTJS_SDL2_LIB_NAME LTJS_SDL2_MAIN_LIB_NAME LTJS_SDL2_INCLUDE_DIR LTJS_SDL2_LIBRARY_DIR
	VERSION_VAR LTJS_SDL2_VERSION
)
