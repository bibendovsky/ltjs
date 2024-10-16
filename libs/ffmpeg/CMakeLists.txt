cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(ltjs_ffmpeg VERSION 1.0.0 LANGUAGES C)

include(ltjs_common)

add_library(ltjs_ffmpeg STATIC "")
add_library(ltjs::ffmpeg ALIAS ltjs_ffmpeg)

message(STATUS "[ltjs_ffmpeg] v${PROJECT_VERSION}")

ltjs_add_defaults(ltjs_ffmpeg)

include(TestBigEndian)
test_big_endian(LTJS_FFMPEG_HAVE_BIGENDIAN)
message(STATUS "[ltjs_ffmpeg] Is big-endian: ${LTJS_FFMPEG_HAVE_BIGENDIAN}")

set_target_properties(ltjs_ffmpeg PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)

target_compile_definitions(ltjs_ffmpeg
	PRIVATE
		_ISOC11_SOURCE
		__STRICT_ANSI__
		_FILE_OFFSET_BITS=64
		_LARGEFILE_SOURCE
		_USE_MATH_DEFINES
		LTJS_FFMPEG_AV_HAVE_BIGENDIAN=${LTJS_FFMPEG_HAVE_BIGENDIAN}
)

target_compile_options(ltjs_ffmpeg
	PRIVATE
		$<$<C_COMPILER_ID:MSVC>:/wd4127 /wd4244 /wd4456 /wd4457>
)

target_include_directories(ltjs_ffmpeg PUBLIC .)

# ===========================================================================

set(ltjs_ffmpeg_libavcodec_sources
	libavcodec/allcodecs.c
	libavcodec/avcodec.c
	libavcodec/avcodec.h
	libavcodec/avcodec_internal.h
	libavcodec/avpacket.c
	libavcodec/bink.c
	libavcodec/binkaudio.c
	libavcodec/binkdata.h
	libavcodec/binkdsp.c
	libavcodec/binkdsp.h
	libavcodec/bitstream.c
	libavcodec/bitstream.h
	libavcodec/bitstream_filters.c
	libavcodec/blockdsp.c
	libavcodec/blockdsp.h
	libavcodec/bsf.c
	libavcodec/bsf.h
	libavcodec/bsf_internal.h
	libavcodec/bytestream.h
	libavcodec/codec.h
	libavcodec/codec_id.h
	libavcodec/codec_internal.h
	libavcodec/codec_par.c
	libavcodec/codec_par.h
	libavcodec/decode.c
	libavcodec/decode.h
	libavcodec/defs.h
	libavcodec/get_bits.h
	libavcodec/get_buffer.c
	libavcodec/hpeldsp.c
	libavcodec/hpeldsp.h
	libavcodec/internal.h
	libavcodec/mathops.h
	libavcodec/options.c
	libavcodec/packet.h
	libavcodec/packet_internal.h
	libavcodec/pixels.h
	libavcodec/put_bits.h
	libavcodec/refstruct.c
	libavcodec/refstruct.h
	libavcodec/rnd_avg.h
	libavcodec/utils.c
	libavcodec/version.c
	libavcodec/version.h
	libavcodec/version_major.h
	libavcodec/vlc.c
	libavcodec/vlc.h
	libavcodec/wma_freqs.c
	libavcodec/wma_freqs.h
	${ltjs_ffmpeg_libavcodec_header_only_sources}
)

set(ltjs_ffmpeg_libavcodec_header_only_sources
	libavcodec/bit_depth_template.c
	libavcodec/hpel_template.c
	libavcodec/pel_template.c
)

source_group("libavcodec" FILES ${ltjs_ffmpeg_libavcodec_sources} ${ltjs_ffmpeg_libavcodec_header_only_sources})
set_source_files_properties(${ltjs_ffmpeg_libavcodec_header_only_sources} PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(ltjs_ffmpeg PRIVATE ${ltjs_ffmpeg_libavcodec_sources})

# ---------------------------------------------------------------------------

set(ltjs_ffmpeg_libavformat_sources
	libavformat/allformats.c
	libavformat/avformat.c
	libavformat/avformat.h
	libavformat/avio.c
	libavformat/avio.h
	libavformat/aviobuf.c
	libavformat/avio_internal.h
	libavformat/bink.c
	libavformat/demux.c
	libavformat/demux.h
	libavformat/demux_utils.c
	libavformat/format.c
	libavformat/internal.h
	libavformat/options.c
	libavformat/seek.c
	libavformat/utils.c
	libavformat/version.c
	libavformat/version.h
	libavformat/version_major.h
)

source_group("libavformat" FILES ${ltjs_ffmpeg_libavformat_sources})
target_sources(ltjs_ffmpeg PRIVATE ${ltjs_ffmpeg_libavformat_sources})

# ---------------------------------------------------------------------------

set(ltjs_ffmpeg_libavutil_sources
	libavutil/avassert.h
	libavutil/avstring.c
	libavutil/avstring.h
	libavutil/avutil.h
	libavutil/buffer.c
	libavutil/buffer.h
	libavutil/buffer_internal.h
	libavutil/channel_layout.c
	libavutil/channel_layout.h
	libavutil/common.h
	libavutil/dict.c
	libavutil/dict.h
	libavutil/error.c
	libavutil/error.h
	libavutil/ffversion.h
	libavutil/frame.c
	libavutil/frame.h
	libavutil/imgutils.c
	libavutil/imgutils.h
	libavutil/intfloat.h
	libavutil/intmath.c
	libavutil/intmath.h
	libavutil/intreadwrite.h
	libavutil/log.c
	libavutil/log.h
	libavutil/log2_tab.c
	libavutil/macros.h
	libavutil/mathematics.c
	libavutil/mathematics.h
	libavutil/mem.c
	libavutil/mem.h
	libavutil/mem_internal.h
	libavutil/pixdesc.c
	libavutil/pixdesc.h
	libavutil/pixfmt.h
	libavutil/qsort.h
	libavutil/rational.c
	libavutil/rational.h
	libavutil/reverse.c
	libavutil/reverse.h
	libavutil/samplefmt.c
	libavutil/samplefmt.h
	libavutil/tx.c
	libavutil/tx.h
	libavutil/tx_float.c
	libavutil/tx_priv.h
	libavutil/version.c
	libavutil/version.h
)

set(ltjs_ffmpeg_libavutil_header_only_sources
	libavutil/tx_template.c
)

source_group("libavutil" FILES ${ltjs_ffmpeg_libavutil_sources} ${ltjs_ffmpeg_libavutil_header_only_sources})
set_source_files_properties(${ltjs_ffmpeg_libavutil_header_only_sources} PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(ltjs_ffmpeg PRIVATE ${ltjs_ffmpeg_libavutil_sources})
