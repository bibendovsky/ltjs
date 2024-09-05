cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_to2_client_res VERSION 0.0.1 LANGUAGES CXX)

if (LTJS_SDL_BACKEND)
	install (
		DIRECTORY
			"ltjs"
		DESTINATION
			"."
	)
else ()
	option (LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

	include (ltjs_common)

	add_library (${PROJECT_NAME} SHARED)

	ltjs_add_defaults (${PROJECT_NAME})

	set_target_properties (
		${PROJECT_NAME}
		PROPERTIES
			OUTPUT_NAME ltjs_cres
			PREFIX ""
	)

	set (LTJS_TMP_LINKER_FLAGS "")

	if (MSVC AND NOT LTJS_USE_SAFE_SEH)
		if (NOT LTJS_USE_SAFE_SEH)
			set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
		endif ()
	endif ()

	set_target_properties (
		${PROJECT_NAME}
		PROPERTIES
			LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
	)

	target_include_directories (
		${PROJECT_NAME}
		PRIVATE
			../shared
			../shared/lang/en
			../../shared/lang/en
	)

	target_sources (
		${PROJECT_NAME}
		PRIVATE
			clientres.h
			../shared/clientresshared.h
	)

	target_sources (
		${PROJECT_NAME}
		PRIVATE
			lang/en/clientres.rc
			main.cpp
	)

	install (
		TARGETS
			${PROJECT_NAME}
		RUNTIME DESTINATION
			"./Game"
	)
endif ()
