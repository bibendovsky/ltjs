cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_game_client_fx VERSION 0.0.1 LANGUAGES CXX)

option (LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

include (ltjs_common)

add_library (${PROJECT_NAME} SHARED)

ltjs_add_defaults (${PROJECT_NAME} stdafx.h)

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME ltjs_clientfx
		PREFIX ""
)

set (LTJS_TMP_LINKER_FLAGS "")

if (MSVC)
	set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

	if (NOT LTJS_USE_SAFE_SEH)
		set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()
endif ()

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		MEMTRACK_CLIENTFX
		_NOMFC
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		../../engine/sdk/inc
		../../libs/butemgr
		../../libs/cryptmgr
		../../libs/lith
		../../libs/mfcstub
		../../libs/stdlith
		../libs/ltguimgr
		../shared
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../shared/bankedlist.h
		../shared/bankedlist_impl.h
		../shared/basefx.h
		../shared/debugnew.h
		../shared/debugnew_impl.h
		bouncychunkfx.h
		camjitterfx.h
		camwobblefx.h
		clientfx.h
		createfx.h
		cycletimer.h
		dynalightfx.h
		fallingstufffx.h
		flarespritefx.h
		lightningfx.h
		ltbbouncychunkfx.h
		ltbmodelfx.h
		memarray.h
		nullfx.h
		particlesystemfx.h
		playrandomsoundfx.h
		playsoundfx.h
		polyfanfx.h
		polytubefx.h
		resource.h
		spritefx.h
		spritesystemfx.h
		stdafx.h
		swaplist.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../engine/sdk/inc/ltmodule.cpp
		../../engine/sdk/inc/ltquatbase.cpp
		../shared/debugnew.cpp
		basefx.cpp
		bouncychunkfx.cpp
		camjitterfx.cpp
		camwobblefx.cpp
		clientfx.cpp
		createfx.cpp
		cycletimer.cpp
		dynalightfx.cpp
		fallingstufffx.cpp
		flarespritefx.cpp
		lightningfx.cpp
		ltbbouncychunkfx.cpp
		ltbmodelfx.cpp
		nullfx.cpp
		particlesystemfx.cpp
		playrandomsoundfx.cpp
		playsoundfx.cpp
		polyfanfx.cpp
		polytubefx.cpp
		spritefx.cpp
		spritesystemfx.cpp
		stdafx.cpp
)

if (MSVC)
	set_source_files_properties (
		../../engine/sdk/inc/ltmodule.cpp
		../../engine/sdk/inc/ltquatbase.cpp
		PROPERTIES
			# Precompiled Header (None)
			COMPILE_FLAGS -Y-
	)

	set_source_files_properties (
		stdafx.cpp
		PROPERTIES
			# Precompiled Header (Create)
			COMPILE_FLAGS -Yc
	)
endif ()

install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION
		"./Game"
)
