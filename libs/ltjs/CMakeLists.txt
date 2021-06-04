cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib VERSION 0.0.1 LANGUAGES CXX)

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		include/ltjs_ascii.h
		include/ltjs_c_string.h
		include/ltjs_circular_queue.h
		include/ltjs_char_conv.h
		include/ltjs_code_page.h
		include/ltjs_dinput.h
		include/ltjs_exception.h
		include/ltjs_file.h
		include/ltjs_file_system_path.h
		include/ltjs_index_type.h
		include/ltjs_iunknown_uresource.h
		include/ltjs_language.h
		include/ltjs_language_mgr.h
		include/ltjs_main_window_descriptor.h
		include/ltjs_script_tokenizer.h
		include/ltjs_sdl_ensure_result.h
		include/ltjs_sdl_exception.h
		include/ltjs_sdl_joystick_guid.h
		include/ltjs_sdl_subsystem.h
		include/ltjs_sdl_uresources.h
		include/ltjs_sdl_utils.h
		include/ltjs_shell_string_formatter.h
		include/ltjs_shared_data_mgr.h
		include/ltjs_shared_library.h
		include/ltjs_shell_resource_mgr.h
		include/ltjs_system_event.h
		include/ltjs_system_event_handler.h
		include/ltjs_system_event_handler_mgr.h
		include/ltjs_system_event_mgr.h
		include/ltjs_system_event_queue.h
		include/ltjs_ucs.h
		include/ltjs_ucs_detail.h
		include/ltjs_utf8_detail.h
		include/ltjs_utf32_detail.h
		include/ltjs_windows_1252.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		src/ltjs_ascii.cpp
		src/ltjs_c_string.cpp
		src/ltjs_circular_queue.cpp
		src/ltjs_char_conv.cpp
		src/ltjs_code_page.cpp
		src/ltjs_dinput.cpp
		src/ltjs_exception.cpp
		src/ltjs_file.cpp
		src/ltjs_file_system_path.cpp
		src/ltjs_index_type.cpp
		src/ltjs_iunknown_uresource.cpp
		src/ltjs_language.cpp
		src/ltjs_language_mgr.cpp
		src/ltjs_main_window_descriptor.cpp
		src/ltjs_script_tokenizer.cpp
		src/ltjs_sdl_ensure_result.cpp
		src/ltjs_sdl_exception.cpp
		src/ltjs_sdl_joystick_guid.cpp
		src/ltjs_sdl_subsystem.cpp
		src/ltjs_sdl_uresources.cpp
		src/ltjs_sdl_utils.cpp
		src/ltjs_shared_data_mgr.cpp
		src/ltjs_shared_library.cpp
		src/ltjs_shell_string_formatter.cpp
		src/ltjs_shell_resource_mgr.cpp
		src/ltjs_system_event.cpp
		src/ltjs_system_event_handler.cpp
		src/ltjs_system_event_handler_mgr.cpp
		src/ltjs_system_event_mgr.cpp
		src/ltjs_system_event_queue.cpp
		src/ltjs_ucs.cpp
		src/ltjs_ucs_detail.cpp
		src/ltjs_utf8_detail.cpp
		src/ltjs_utf32_detail.cpp
		src/ltjs_windows_1252.cpp
)
