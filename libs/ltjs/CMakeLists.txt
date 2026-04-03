cmake_minimum_required(VERSION 3.24.4 FATAL_ERROR)
project(ltjs_lib VERSION 0.0.1 LANGUAGES CXX)
add_library(${PROJECT_NAME} STATIC)
add_library(ltjs::lib ALIAS ${PROJECT_NAME})
include(ltjs_common)
ltjs_add_defaults(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(${PROJECT_NAME} PRIVATE
	include/ltjs_ascii.h
	include/ltjs_cgm.h
	include/ltjs_circular_queue.h
	include/ltjs_code_page.h
	include/ltjs_dinput.h
	include/ltjs_endian.h
	include/ltjs_exception.h
	include/ltjs_file_path.h
	include/ltjs_file_stream.h
	include/ltjs_four_cc.h
	include/ltjs_language.h
	include/ltjs_language_mgr.h
	include/ltjs_logger.h
	include/ltjs_main_window_descriptor.h
	include/ltjs_read_only_memory_stream.h
	include/ltjs_riff_four_ccs.h
	include/ltjs_riff_reader.h
	include/ltjs_script_tokenizer.h
	include/ltjs_sdl_raii.h
	include/ltjs_sdl_subsystem.h
	include/ltjs_sdl_utility.h
	include/ltjs_shared_data_mgr.h
	include/ltjs_shell_resource_mgr.h
	include/ltjs_shell_string_formatter.h
	include/ltjs_stream.h
	include/ltjs_substream.h
	include/ltjs_sys_event.h
	include/ltjs_sys_event_handler.h
	include/ltjs_sys_event_handler_mgr.h
	include/ltjs_sys_event_mgr.h
	include/ltjs_sys_event_queue.h
	include/ltjs_sys_file_utility.h
	include/ltjs_sys_shared_library.h
	include/ltjs_sys_time.h
	include/ltjs_uuid.h
	include/ltjs_wave_format.h
	include/ltjs_wave_format_utils.h
	include/ltjs_wave_four_ccs.h
)
target_sources(${PROJECT_NAME} PRIVATE
	src/ltjs_cgm.cpp
	src/ltjs_code_page.cpp
	src/ltjs_endian.cpp
	src/ltjs_exception.cpp
	src/ltjs_file_path.cpp
	src/ltjs_file_stream.cpp
	src/ltjs_language_mgr.cpp
	src/ltjs_logger.cpp
	src/ltjs_logger_win32.cpp
	src/ltjs_read_only_memory_stream.cpp
	src/ltjs_riff_reader.cpp
	src/ltjs_script_tokenizer.cpp
	src/ltjs_sdl_raii.cpp
	src/ltjs_sdl_subsystem.cpp
	src/ltjs_sdl_utility.cpp
	src/ltjs_shared_data_mgr.cpp
	src/ltjs_shell_resource_mgr.cpp
	src/ltjs_shell_string_formatter.cpp
	src/ltjs_stream.cpp
	src/ltjs_substream.cpp
	src/ltjs_sys_event_handler_mgr.cpp
	src/ltjs_sys_event_mgr.cpp
	src/ltjs_sys_file_utility.cpp
	src/ltjs_sys_shared_library.cpp
	src/ltjs_sys_time.cpp
	src/ltjs_uuid.cpp
	src/ltjs_wave_format.cpp
	src/ltjs_wave_format_utils.cpp
)
