cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(ltjs_audio VERSION 1.0.0 LANGUAGES CXX)

include (ltjs_common)

add_library(ltjs_audio STATIC "")
add_library(ltjs::audio ALIAS ltjs_audio)

ltjs_add_defaults(ltjs_audio)

target_link_libraries(ltjs_audio PRIVATE bibendovsky_spul_lib)

target_compile_definitions(ltjs_audio
	PRIVATE
		#_CRT_SECURE_NO_WARNINGS
)

target_include_directories(ltjs_audio
	PUBLIC
		include
	PRIVATE
		src/minimp3
)

target_sources(ltjs_audio
	PRIVATE
		include/ltjs_audio_decoder.h
	PRIVATE
		src/ltjs_audio_converter.cpp
		src/ltjs_audio_converter.h
		src/ltjs_audio_decoder.cpp
		src/ltjs_audio_limits.cpp
		src/ltjs_audio_limits.h
		src/ltjs_audio_sample_converter.cpp
		src/ltjs_audio_sample_converter.h
		src/ltjs_basic_audio_decoder.cpp
		src/ltjs_basic_audio_decoder.h
		src/ltjs_mp3_audio_decoder.cpp
		src/ltjs_mp3_audio_decoder.h
		src/ltjs_ms_ima_adpcm_audio_decoder.cpp
		src/ltjs_ms_ima_adpcm_audio_decoder.h
		src/ltjs_pcm_audio_decoder.cpp
		src/ltjs_pcm_audio_decoder.h
)
