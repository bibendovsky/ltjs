#
# A very simple "find_package" implementation for FFmpeg.
#
# Generated variables:
#    - LTJS_FFMPEG_INCLUDE_DIR - path to FFmpeg headers.
#    - LTJS_FFMPEG_LIBRARY_DIR - path to FFmpeg libraries.
#    - LTJS_FFMPEG_LIBRARY - FFmpeg libraries for target_link_libraries, etc.
#    - LTJS_FFMPEG_VERSION - FFmpeg detected version.
#


unset(LTJS_FFMPEG_TMP_IS_INCS_FOUND)
unset(LTJS_FFMPEG_TMP_IS_LIBS_FOUND)


#
# Headers
#

set(
	LTJS_FFMPEG_TMP_REF_INCS
	libavcodec/avcodec.h
	libavformat/avformat.h
	libavutil/avutil.h
	libavutil/ffversion.h
	libavutil/opt.h
	libswresample/swresample.h
)

find_path(
	LTJS_FFMPEG_INCLUDE_DIR
	NAMES
	${LTJS_FFMPEG_TMP_REF_INCS}
)

if (LTJS_FFMPEG_INCLUDE_DIR)
	set(LTJS_FFMPEG_TMP_FFVERSION_H "${LTJS_FFMPEG_INCLUDE_DIR}/libavutil/ffversion.h")

	if (EXISTS ${LTJS_FFMPEG_TMP_FFVERSION_H})
		file(
			STRINGS
			"${LTJS_FFMPEG_TMP_FFVERSION_H}"
			LTJS_FFMPEG_VERSION_LINE
			REGEX "^#define[ \t]+FFMPEG_VERSION[ \t]+\"[0-9]+\.[0-9]+\.[0-9]+\"$"
		)

		string(
			REGEX REPLACE
			"#define[ \t]+FFMPEG_VERSION[ \t]+\"([0-9]+\.[0-9]+\.[0-9]+)\""
			"\\1"
			LTJS_FFMPEG_VERSION
			"${LTJS_FFMPEG_VERSION_LINE}"
		)
	else ()
		unset(LTJS_FFMPEG_VERSION_LINE)
	endif ()

	if (LTJS_FFMPEG_VERSION_LINE)
		set(LTJS_FFMPEG_TMP_IS_INCS_FOUND "TRUE")
	else ()
		message("Failed to detect FFmpeg version.")
	endif ()
else ()
	message("Required FFmpeg headers not found.")
endif ()


#
# Libraries
#

unset(LTJS_FFMPEG_TMP_FOUND_LIBS)
unset(LTJS_FFMPEG_TMP_MISSING_LIBS)

set(
	LTJS_FFMPEG_TMP_REF_LIBS
	avcodec
	avformat
	avutil
	swresample
	swscale
)

find_library(
	LTJS_FFMPEG_LIBRARY_DIR
	NAMES
	${LTJS_FFMPEG_LIB_LIST}
	NAMES_PER_DIR
	NO_DEFAULT_PATH
)

if (LTJS_FFMPEG_LIBRARY_DIR)
	if (NOT IS_DIRECTORY "${LTJS_FFMPEG_LIBRARY_DIR}")
		get_filename_component(LTJS_FFMPEG_LIBRARY_DIR "${LTJS_FFMPEG_LIBRARY_DIR}" DIRECTORY)
		set(LTJS_FFMPEG_LIBRARY_DIR "${LTJS_FFMPEG_LIBRARY_DIR}" CACHE PATH "FFmpeg library directory." FORCE)
	endif ()

	foreach (LTJS_FFMPEG_TMP IN LISTS LTJS_FFMPEG_TMP_REF_LIBS)
		set(
			LTJS_FFMPEG_TMP_LIB
			"${LTJS_FFMPEG_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${LTJS_FFMPEG_TMP}${CMAKE_STATIC_LIBRARY_SUFFIX}"
		)

		if (EXISTS "${LTJS_FFMPEG_TMP_LIB}")
			list(APPEND LTJS_FFMPEG_TMP_FOUND_LIBS "${LTJS_FFMPEG_TMP_LIB}")
		else ()
			list(APPEND LTJS_FFMPEG_TMP_MISSING_LIBS "${LTJS_FFMPEG_TMP_LIB}")
		endif ()
	endforeach ()

	list(LENGTH LTJS_FFMPEG_TMP_FOUND_LIBS LTJS_FFMPEG_TMP_FOUND_LIBS_COUNT)
	list(LENGTH LTJS_FFMPEG_TMP_MISSING_LIBS LTJS_FFMPEG_TMP_MISSING_LIBS_COUNT)

	if (LTJS_FFMPEG_TMP_MISSING_LIBS)
		unset(LTJS_FFMPEG_LIBRARY)

		message("Missing FFmpeg libraries:")

		foreach (LTJS_FFMPEG_TMP IN LISTS LTJS_FFMPEG_TMP_MISSING_LIBS)
			message("    - ${LTJS_FFMPEG_TMP}")
		endforeach ()
	else ()
		set(LTJS_FFMPEG_TMP_IS_LIBS_FOUND "TRUE")
		set(LTJS_FFMPEG_LIBRARY "${LTJS_FFMPEG_TMP_FOUND_LIBS}")
	endif ()
endif ()

if (NOT LTJS_FFMPEG_TMP_IS_INCS_FOUND OR NOT LTJS_FFMPEG_TMP_IS_LIBS_FOUND)
	message("")
	message("*** FFmpeg not found ***")
	message("")
	message(FATAL_ERROR "")
endif ()


include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	LTJS_FFMPEG
	REQUIRED_VARS LTJS_FFMPEG_INCLUDE_DIR LTJS_FFMPEG_LIBRARY_DIR
	VERSION_VAR LTJS_FFMPEG_VERSION
)
