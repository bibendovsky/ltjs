#
# Locate SDL2 library.
#
# User-provided/generated variables:
#    - LTJS_SDL2_LIB_NAME - the name of SDL2 library (default: SDL2).
#    - LTJS_SDL2_MAIN_LIB_NAME - the name of SDL2main library (default: SDL2main).
#
# Generated variables:
#    - LTJS_SDL2_INCLUDE_DIR - path to SDL2 headers.
#    - LTJS_SDL2_LIBRARY_DIR - path to SDL2 libraries.
#    - LTJS_SDL2_VERSION - SDL2 detected version.
#    - LTJS_SDL2_FOUND - set to TRUE if SDL2 found.
#
# Imported targets:
#    - LTJS::SDL2
#


#
# Find threading library
#
find_package(Threads)


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

unset(LTJS_SDL2_TMP_ARE_INCS_FOUND)

if (LTJS_SDL2_INCLUDE_DIR)
	set(
		LTJS_SDL2_TMP_VERSION_H
		${LTJS_SDL2_INCLUDE_DIR}/SDL_version.h
	)

	unset(LTJS_SDL2_VERSION_LINE)

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
	endif ()

	if (LTJS_SDL2_VERSION_LINE)
		set(LTJS_SDL2_TMP_ARE_INCS_FOUND TRUE)
		set(LTJS_SDL2_VERSION ${LTJS_SDL2_VERSION_LINE})
	endif ()
endif ()


#
# Libraries
#
unset(LTJS_SDL2_TMP_IS_LIB_FOUND)
unset(LTJS_SDL2_TMP_IS_MAIN_LIB_FOUND)
unset(LTJS_SDL2_TMP_ARE_LIBS_FOUND)
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
	if (NOT IS_DIRECTORY ${LTJS_SDL2_LIBRARY_DIR})
		get_filename_component(LTJS_SDL2_LIBRARY_DIR ${LTJS_SDL2_LIBRARY_DIR} DIRECTORY)
		set(LTJS_SDL2_LIBRARY_DIR ${LTJS_SDL2_LIBRARY_DIR} CACHE PATH "SDL2 library directory." FORCE)
	endif ()

	#
	# SDL2
	#
	set(
		LTJS_SDL2_TMP_LIB
		${LTJS_SDL2_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${LTJS_SDL2_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
	)

	if (EXISTS ${LTJS_SDL2_TMP_LIB})
		set(LTJS_SDL2_TMP_IS_LIB_FOUND TRUE)
		list(APPEND LTJS_SDL2_TMP_FOUND_LIBS ${LTJS_SDL2_TMP_LIB})
	endif ()


	#
	# SDL2main
	#
	# FIXME Are all platforms has "SDL2main"?
	#
	set(
		LTJS_SDL2_TMP_LIB
		${LTJS_SDL2_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${LTJS_SDL2_MAIN_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
	)

	if (EXISTS ${LTJS_SDL2_TMP_LIB})
		set(LTJS_SDL2_TMP_IS_MAIN_LIB_FOUND TRUE)
		list(APPEND LTJS_SDL2_TMP_FOUND_LIBS ${LTJS_SDL2_TMP_LIB})
	endif ()

	if (LTJS_SDL2_TMP_IS_LIB_FOUND AND LTJS_SDL2_TMP_IS_MAIN_LIB_FOUND)
		set(LTJS_SDL2_ARE_LIBS_FOUND TRUE)
	endif ()

	if (LTJS_SDL2_ARE_LIBS_FOUND)
		set(LTJS_SDL2_LIBRARY ${LTJS_SDL2_TMP_FOUND_LIBS})
	endif ()
endif ()

if (LTJS_SDL2_TMP_ARE_INCS_FOUND AND LTJS_SDL2_ARE_LIBS_FOUND)
	set(LTJS_SDL2_FOUND TRUE)
else ()
	unset(LTJS_SDL2_FOUND)
endif ()


#
# Default handler
#
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	LTJS_SDL2
	REQUIRED_VARS
		LTJS_SDL2_FOUND
		LTJS_SDL2_VERSION
		LTJS_SDL2_LIB_NAME
		LTJS_SDL2_MAIN_LIB_NAME
		LTJS_SDL2_INCLUDE_DIR
		LTJS_SDL2_LIBRARY_DIR
	VERSION_VAR
		LTJS_SDL2_VERSION
)


#
# Add imported target
#
if (LTJS_SDL2_FOUND)
	set(LTJS_SDL2_TMP_LIBRARIES "")

	if (MINGW)
		list(
			APPEND LTJS_SDL2_TMP_LIBRARIES
			mingw32
		)
	endif ()

	list(
		APPEND LTJS_SDL2_TMP_LIBRARIES
		${LTJS_SDL2_LIBRARY}
		Threads::Threads
	)

	if (WIN32)
		list(
			APPEND LTJS_SDL2_TMP_LIBRARIES
			imm32
			setupapi
			version
			winmm
		)
	endif ()


	add_library(LTJS::SDL2 INTERFACE IMPORTED)

	set_property(
		TARGET LTJS::SDL2
		PROPERTY
			INTERFACE_LINK_LIBRARIES
				${LTJS_SDL2_TMP_LIBRARIES}
	)
endif ()
