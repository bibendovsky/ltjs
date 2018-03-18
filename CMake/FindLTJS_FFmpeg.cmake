#
# A very simple "find_package" implementation for FFmpeg.
#
# Generated variables:
#    - LTJS_FFMPEG_INCLUDE_DIR - path to FFmpeg headers.
#    - LTJS_FFMPEG_LIBRARY_DIR - path to FFmpeg libraries.
#    - LTJS_FFMPEG_LIBRARY - FFmpeg libraries for target_link_libraries, etc.
#    - LTJS_FFMPEG_VERSION - FFmpeg detected version.
#


find_path(
	LTJS_FFMPEG_INCLUDE_DIR
	NAMES
	libavcodec/avcodec.h
	libavformat/avformat.h
	libavutil/avutil.h
	libavutil/opt.h
	libswresample/swresample.h
)

if (LTJS_FFMPEG_INCLUDE_DIR)
	file(
		STRINGS
		"${LTJS_FFMPEG_INCLUDE_DIR}/libavutil/ffversion.h"
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
endif ()

find_library(
	LTJS_FFMPEG_LIBRARY_DIR
	NAMES
	avcodec
	avformat
	avutil
	swresample
	swscale
)

if (LTJS_FFMPEG_LIBRARY_DIR)
	set(
		LTJS_FFMPEG_LIBRARY
		${LTJS_FFMPEG_LIBRARY_DIR}/avcodec${CMAKE_STATIC_LIBRARY_SUFFIX}
		${LTJS_FFMPEG_LIBRARY_DIR}/avformat${CMAKE_STATIC_LIBRARY_SUFFIX}
		${LTJS_FFMPEG_LIBRARY_DIR}/avutil${CMAKE_STATIC_LIBRARY_SUFFIX}
		${LTJS_FFMPEG_LIBRARY_DIR}/swresample${CMAKE_STATIC_LIBRARY_SUFFIX}
		${LTJS_FFMPEG_LIBRARY_DIR}/swscale${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
endif ()


include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	LTJS_FFMPEG
	REQUIRED_VARS LTJS_FFMPEG_INCLUDE_DIR LTJS_FFMPEG_LIBRARY_DIR
	VERSION_VAR LTJS_FFMPEG_VERSION
)
