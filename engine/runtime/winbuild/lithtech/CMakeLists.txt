cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (ltjs_lith_tech VERSION 0.0.1 LANGUAGES CXX)

option (LTJS_USE_DIRECT_MUSIC8 "Use DirectMusic8 for music." OFF)
option (LTJS_USE_D3DX9 "Use Direct3D 9 extensions." OFF)
option (LTJS_USE_FFMPEG_VIDEO_MGR "Use video manager implemented via FFmpeg." ON)
option (LTJS_USE_SAFE_SEH "Use Safe Exception Handlers." OFF)

if (LTJS_USE_DIRECT_MUSIC8 OR LTJS_USE_D3DX9)
	set (LTJS_USE_BUNDLED_DIRECTX ON)
endif ()

include (ltjs_common)

add_executable (${PROJECT_NAME})

ltjs_add_defaults (${PROJECT_NAME})

if (LTJS_USE_PCH AND NOT (${CMAKE_VERSION} VERSION_LESS "3.16.0"))
	target_precompile_headers(
		${PROJECT_NAME}
		PRIVATE
			[["bdefs.h"]]
	)
endif ()

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME ltjs_lithtech
)

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		DE_LOCAL_SERVERBIND
		DE_CLIENT_COMPILE
		DSNDMGR_NO_MFC
		DIRECTENGINE_COMPILE
		__D3D
		__D3DREND
		MODEL_SUPPORT_ABC
		$<$<CONFIG:DEBUG>:D3D_DEBUG_INFO>
)

if (LTJS_USE_FFMPEG_VIDEO_MGR)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			LTJS_USE_FFMPEG_VIDEO_MGR
	)
endif ()

if (NOT MSVC)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			_XM_NO_INTRINSICS_
	)
endif ()

if (LTJS_USE_DIRECT_MUSIC8)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			LTJS_USE_DIRECT_MUSIC8
	)
endif ()

if (LTJS_USE_BUNDLED_DIRECTX)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			LTJS_USE_BUNDLED_DIRECTX
	)
endif ()

set (
	libs
	${LTJS_FFMPEG_LIBRARY}
)

set (LTJS_TMP_LINKER_FLAGS "")

if (WIN32)
	list(
		APPEND
		libs
		dbghelp
	)
endif ()

if (MSVC)
	set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

	if (NOT LTJS_USE_SAFE_SEH)
		set (LTJS_TMP_LINKER_FLAGS "${LTJS_TMP_LINKER_FLAGS} /SAFESEH:NO")
	endif ()

	if (LTJS_USE_BUNDLED_DIRECTX)
		if (CMAKE_SIZEOF_VOID_P EQUAL 4)
			set (
				path_to_dx_libs
				${CMAKE_SOURCE_DIR}/directx/lib/x86
			)
		elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
			set (
				path_to_dx_libs
				${CMAKE_SOURCE_DIR}/directx/lib/x64
			)
		else ()
			message (FATAL_ERROR "Unsupported platform.")
		endif ()

		list(
			APPEND
			libs
			${path_to_dx_libs}/ddraw.lib
			${path_to_dx_libs}/d3d9.lib
			${path_to_dx_libs}/dxguid.lib
			${path_to_dx_libs}/dinput8.lib
			${path_to_dx_libs}/dplayx.lib
			winmm
		)

		if (LTJS_USE_D3DX9)
			list(
				APPEND
				libs
				${path_to_dx_libs}/d3dx9.lib
			)
		endif ()
	else ()
		list(
			APPEND
			libs
			d3d9
			dxguid
			dinput8
			winmm
			ws2_32
		)

		if (LTJS_USE_D3DX9)
			list(
				APPEND
				libs
				d3dx9
			)
		endif ()
	endif ()
endif ()

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		LINK_FLAGS "${LTJS_TMP_LINKER_FLAGS}"
)

if (NOT MSVC)
	list(
		APPEND
		libs
		d3d9
		dxguid
		dinput8
		winmm
		ws2_32
	)

	if (LTJS_USE_D3DX9)
		list(
			APPEND
			libs
			d3dx9
		)
	endif ()
endif ()

if (MINGW)
	list(
		APPEND
		libs
		iconv
		z
	)
endif ()

target_link_libraries (
	${PROJECT_NAME}
	PRIVATE
		bibendovsky_spul_lib
		ltjs::audio
		ltjs_lib_d3d_render
		ltjs_lib_lt_mem
		ltjs_lib_rez_mgr
		ltjs_lib_std_lith
		ltjs_lib_ilt_sound
		ltjs_lib_lith
		ltjs_lib_ui
		ltjs_lib_info
		${libs}
)

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		${LTJS_FFMPEG_INCLUDE_DIR}
		.
		../../../../libs/DirectXMath/Inc
		../../../../libs/lith
		../../../../libs/mfcstub
		../../../../libs/stdlith
		../../../libs/lib_dshow
		../../../libs/rezmgr
		../../../sdk/inc
		../../../sdk/inc/compat
		../../../sdk/inc/physics
		../../../sdk/inc/state_mgr
		../../client/src
		../../comm/src
		../../controlfilemgr
		../../crtcompat
		../../distrobj/src
		../../kernel/io/src
		../../kernel/mem/src
		../../kernel/net/src
		../../kernel/src
		../../kernel/src/sys/win
		../../lithtemplate
		../../model/src
		../../mpm/src
		../../physics/src
		../../render_a/src/sys/d3d
		../../render_b/src
		../../server/src
		../../shared/src
		../../shared/src/sys/win
		../../sound/src
		../../state_mgr/src
		../../ui/src
		../../world/src
)

if (LTJS_USE_BUNDLED_DIRECTX)
	target_include_directories (
		${PROJECT_NAME}
		PRIVATE
			../../../../directx/include
	)
endif ()

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../libs/rezmgr/rezfile.h
		../../../libs/rezmgr/rezhash.h
		../../../libs/rezmgr/rezmgr.h
		../../../libs/rezmgr/reztypes.h
		../../../sdk/inc/cui.h
		../../../sdk/inc/cuifont.h
		../../../sdk/inc/cuiformattedpolystring.h
		../../../sdk/inc/cuimessage.h
		../../../sdk/inc/cuipolystring.h
		../../../sdk/inc/cuitypes.h
		../../../sdk/inc/iaggregate.h
		../../../sdk/inc/iclientshell.h
		../../../sdk/inc/iltbaseclass.h
		../../../sdk/inc/iltbenchmark.h
		../../../sdk/inc/iltclient.h
		../../../sdk/inc/iltcommon.h
		../../../sdk/inc/iltcsbase.h
		../../../sdk/inc/iltcursor.h
		../../../sdk/inc/iltdirectmusic.h
		../../../sdk/inc/iltdrawprim.h
		../../../sdk/inc/iltfontmanager.h
		../../../sdk/inc/iltinfo.h
		../../../sdk/inc/iltmemory.h
		../../../sdk/inc/iltmessage.h
		../../../sdk/inc/iltmodel.h
		../../../sdk/inc/iltphysics.h
		../../../sdk/inc/iltrefcount.h
		../../../sdk/inc/iltrenderstyles.h
		../../../sdk/inc/iltserver.h
		../../../sdk/inc/iltsoundmgr.h
		../../../sdk/inc/iltspritecontrol.h
		../../../sdk/inc/iltstream.h
		../../../sdk/inc/ilttexinterface.h
		../../../sdk/inc/ilttexturestring.h
		../../../sdk/inc/ilttransform.h
		../../../sdk/inc/iltvideomgr.h
		../../../sdk/inc/iltwidgetmanager.h
		../../../sdk/inc/iservershell.h
		../../../sdk/inc/lithtech.h
		../../../sdk/inc/ltanimtracker.h
		../../../sdk/inc/ltassert.h
		../../../sdk/inc/ltbasedefs.h
		../../../sdk/inc/ltbasetypes.h
		../../../sdk/inc/ltcodes.h
		../../../sdk/inc/ltengineobjects.h
		../../../sdk/inc/lterror.h
		../../../sdk/inc/ltinteger.h
		../../../sdk/inc/ltlink.h
		../../../sdk/inc/ltmatrix.h
		../../../sdk/inc/ltmodule.h
		../../../sdk/inc/ltobjectcreate.h
		../../../sdk/inc/ltplane.h
		../../../sdk/inc/ltproperty.h
		../../../sdk/inc/ltpvalue.h
		../../../sdk/inc/ltquatbase.h
		../../../sdk/inc/ltrect.h
		../../../sdk/inc/ltrenderstyle.h
		../../../sdk/inc/ltrotation.h
		../../../sdk/inc/ltserverobj.h
		../../../sdk/inc/ltvector.h
		../../../sdk/inc/physics/aabb.h
		../../../sdk/inc/physics/aabb_tree.h
		../../../sdk/inc/physics/collision_data.h
		../../../sdk/inc/physics/collision_mgr.h
		../../../sdk/inc/physics/collision_object.h
		../../../sdk/inc/physics/coordinate_frame.h
		../../../sdk/inc/physics/math_phys.h
		../../../sdk/inc/physics/matrix.h
		../../../sdk/inc/physics/quaternion.h
		../../../sdk/inc/physics/rigid_body.h
		../../../sdk/inc/physics/triangle.h
		../../../sdk/inc/physics/vector.h
		../../../sdk/inc/server_interface.h
		../../client/src/client_filemgr.h
		../../client/src/client_formatmgr.h
		../../client/src/client_graphmgr.h
		../../client/src/client_ticks.h
		../../client/src/clientde_impl.h
		../../client/src/clientexception.h
		../../client/src/clientmgr.h
		../../client/src/clientshell.h
		../../client/src/cmoveabstract.h
		../../client/src/console.h
		../../client/src/consolecommands.h
		../../client/src/customfontfilemgr.h
		../../client/src/debuggraphmgr.h
		../../client/src/errorlog.h
		../../client/src/linesystem.h
		../../client/src/ltbenchmark_impl.h
		../../client/src/ltinfo_impl.h
		../../client/src/ltmessage_client.h
		../../client/src/lttexturestring.h
		../../client/src/memorywatch.h
		../../client/src/particlesystem.h
		../../client/src/polygrid.h
		../../client/src/predict.h
		../../client/src/setupobject.h
		../../client/src/sprite.h
		../../client/src/sys/win/winclientde_impl.h
		../../client/src/sys/win/winconsole_impl.h
		../../client/src/sysclient.h
		../../client/src/sysclientde_impl.h
		../../client/src/sysconsole_impl.h
		../../client/src/texturestring.h
		../../client/src/texturestringimage.h
		../../client/src/volumeeffect.h
		../../client/src/watermark.h
		../../controlfilemgr/controlfilemgr.h
		../../kernel/io/src/sys/win/de_file.h
		../../kernel/io/src/sysfile.h
		../../kernel/mem/src/de_memory.h
		../../kernel/net/src/localdriver.h
		../../kernel/net/src/netmgr.h
		../../kernel/net/src/packet.h
		../../kernel/net/src/sys/win/socket.h
		../../kernel/src/dsys.h
		../../kernel/src/icommandlineargs.h
		../../kernel/src/ithread.h
		../../kernel/src/sys/win/appresource.h
		../../kernel/src/sys/win/bindmgr.h
		../../kernel/src/sys/win/binkvideomgrimpl.h
		../../kernel/src/sys/win/counter.h
		../../kernel/src/sys/win/dshowvideomgrimpl.h
		../../kernel/src/sys/win/dsys_interface.h
		../../kernel/src/sys/win/dutil.h
		../../kernel/src/sys/win/ltjs_ffmpeg_video_mgr_impl.h
		../../kernel/src/sys/win/ltjs_fmv_player.h
		../../kernel/src/sys/win/input.h
		../../kernel/src/sys/win/interface_helpers.h
		../../kernel/src/sys/win/load_pcx.h
		../../kernel/src/sys/win/ltdirectmusic_impl.h
		../../kernel/src/sys/win/ltdirectmusiccontrolfile.h
		../../kernel/src/sys/win/ltdirectmusicloader.h
		../../kernel/src/sys/win/lthread.h
		../../kernel/src/sys/win/ltrendermgr_impl.h
		../../kernel/src/sys/win/ltthread.h
		../../kernel/src/sys/win/musicdriver.h
		../../kernel/src/sys/win/render.h
		../../kernel/src/sys/win/streamsim.h
		../../kernel/src/sys/win/stringmgr.h
		../../kernel/src/sys/win/systeminfo.h
		../../kernel/src/sys/win/text_mgr.h
		../../kernel/src/sys/win/timemgr.h
		../../kernel/src/sys/win/version_resource.h
		../../kernel/src/sys/win/videomgr.h
		../../kernel/src/sys/win/winbind.h
		../../kernel/src/syscounter.h
		../../kernel/src/sysdebugging.h
		../../kernel/src/sysinput.h
		../../kernel/src/syslibraryloader.h
		../../kernel/src/sysstreamsim.h
		../../kernel/src/systhread.h
		../../kernel/src/systimer.h
		../../kernel/src/sysvideo.h
		../../model/src/animtracker.h
		../../model/src/ltb.h
		../../model/src/model.h
		../../model/src/model_ops.h
		../../model/src/modelallocations.h
		../../model/src/transformmaker.h
		../../physics/src/lt_collision_mgr.h
		../../render_a/src/sys/d3d/clipline.h
		../../render_a/src/sys/d3d/common_draw.h
		../../render_a/src/sys/d3d/d3d_convar.h
		../../render_a/src/sys/d3d/d3d_device.h
		../../render_a/src/sys/d3d/d3d_draw.h
		../../render_a/src/sys/d3d/d3d_renderstatemgr.h
		../../render_a/src/sys/d3d/d3d_renderstyle.h
		../../render_a/src/sys/d3d/d3d_utils.h
		../../render_b/src/gendrawprim.h
		../../render_b/src/sys/d3d/d3ddrawprim.h
		../../render_b/src/sys/d3d/d3dtexinterface.h
		../../render_b/src/sysdrawprim.h
		../../server/src/classmgr.h
		../../server/src/game_serialize.h
		../../server/src/interlink.h
		../../server/src/ltmessage_server.h
		../../server/src/s_client.h
		../../server/src/s_concommand.h
		../../server/src/s_net.h
		../../server/src/s_object.h
		../../server/src/server_consolestate.h
		../../server/src/server_extradata.h
		../../server/src/server_filemgr.h
		../../server/src/serverde_impl.h
		../../server/src/serverevent.h
		../../server/src/serverexception.h
		../../server/src/servermgr.h
		../../server/src/serverobj.h
		../../server/src/smoveabstract.h
		../../server/src/soundtrack.h
		../../shared/src/bdefs.h
		../../shared/src/build_options.h
		../../shared/src/classbind.h
		../../shared/src/clienthack.h
		../../shared/src/collision.h
		../../shared/src/colorops.h
		../../shared/src/compress.h
		../../shared/src/concommand.h
		../../shared/src/conparse.h
		../../shared/src/debuggeometry.h
		../../shared/src/dhashtable.h
		../../shared/src/dtxmgr.h
		../../shared/src/findobj.h
		../../shared/src/ftbase.h
		../../shared/src/ftclient.h
		../../shared/src/ftserv.h
		../../shared/src/gamemath.h
		../../shared/src/genericprop_setup.h
		../../shared/src/genltstream.h
		../../shared/src/geometry.h
		../../shared/src/geomroutines.h
		../../shared/src/impl_common.h
		../../shared/src/lightmap_compress.h
		../../shared/src/lightmap_planes.h
		../../shared/src/lightmapdefs.h
		../../shared/src/ltbbox.h
		../../shared/src/ltmessage.h
		../../shared/src/ltmutex.h
		../../shared/src/lttimer.h
		../../shared/src/misctools.h
		../../shared/src/motion.h
		../../shared/src/moveobject.h
		../../shared/src/moveplayer.h
		../../shared/src/musicmgr.h
		../../shared/src/nexus.h
		../../shared/src/objectmgr.h
		../../shared/src/packetdefs.h
		../../shared/src/parse_world_info.h
		../../shared/src/pixelformat.h
		../../shared/src/ratetracker.h
		../../shared/src/refobj.h
		../../shared/src/renderinfostruct.h
		../../shared/src/renderobject.h
		../../shared/src/shared_iltcommon.h
		../../shared/src/stacktrace.h
		../../shared/src/staticfifo.h
		../../shared/src/stdlterror.h
		../../shared/src/strtools.h
		../../shared/src/sys/win/d3dddstructs.h
		../../shared/src/sys/win/dstreamopenqueuemgr.h
		../../shared/src/sys/win/renderstruct.h
		../../shared/src/sys/win/winstdlterror.h
		../../shared/src/sys/win/winsync.h
		../../shared/src/sysddstructs.h
		../../shared/src/varsetter.h
		../../shared/src/version_info.h
		../../sound/src/iltsound.h
		../../sound/src/ltjs_dmusic_manager.h
		../../sound/src/ltjs_dmusic_segment.h
		../../sound/src/soundbuffer.h
		../../sound/src/sounddata.h
		../../sound/src/soundinstance.h
		../../sound/src/soundmgr.h
		../../sound/src/wave.h
		../../ui/src/cuipolystring_impl.h
		../../world/src/de_mainworld.h
		../../world/src/de_objects.h
		../../world/src/de_sprite.h
		../../world/src/de_world.h
		../../world/src/fullintersectline.h
		../../world/src/intersect_line.h
		../../world/src/light_table.h
		../../world/src/loadstatus.h
		../../world/src/world_blind_object_data.h
		../../world/src/world_blocker_data.h
		../../world/src/world_blocker_math.h
		../../world/src/world_client.h
		../../world/src/world_client_bsp.h
		../../world/src/world_interface.h
		../../world/src/world_particle_blocker_data.h
		../../world/src/world_server.h
		../../world/src/world_server_bsp.h
		../../world/src/world_shared_bsp.h
		../../world/src/world_tree.h
		../../world/src/worldtreehelper.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../sdk/inc/lterror.cpp
		../../../sdk/inc/ltmodule.cpp
		../../../sdk/inc/ltobjref.cpp
		../../../sdk/inc/ltquatbase.cpp
		../../client/src/client_filemgr.cpp
		../../client/src/client_formatmgr.cpp
		../../client/src/client_graphmgr.cpp
		../../client/src/client_iltcommon.cpp
		../../client/src/client_iltmodel.cpp
		../../client/src/client_iltphysics.cpp
		../../client/src/client_iltvideomgr.cpp
		../../client/src/client_ticks.cpp
		../../client/src/clientde_impl.cpp
		../../client/src/clientmgr.cpp
		../../client/src/clientshell.cpp
		../../client/src/cmoveabstract.cpp
		../../client/src/cnet.cpp
		../../client/src/cobject.cpp
		../../client/src/console.cpp
		../../client/src/consolecommands.cpp
		../../client/src/cutil.cpp
		../../client/src/debuggraphmgr.cpp
		../../client/src/errorlog.cpp
		../../client/src/linesystem.cpp
		../../client/src/ltbenchmark_impl.cpp
		../../client/src/ltinfo_impl.cpp
		../../client/src/ltmessage_client.cpp
		../../client/src/lttexturestring.cpp
		../../client/src/memorywatch.cpp
		../../client/src/particlesystem.cpp
		../../client/src/polygrid.cpp
		../../client/src/predict.cpp
		../../client/src/setupobject.cpp
		../../client/src/shellnet.cpp
		../../client/src/shelltransfer.cpp
		../../client/src/shellutil.cpp
		../../client/src/sprite.cpp
		../../client/src/sys/win/customfontfilemgr.cpp
		../../client/src/sys/win/texturestringimage.cpp
		../../client/src/sys/win/winclientde_impl.cpp
		../../client/src/sys/win/winconsole_impl.cpp
		../../client/src/texturestring.cpp
		../../client/src/volumeeffect.cpp
		../../client/src/watermark.cpp
		../../client/src/world_client_bsp.cpp
		../../controlfilemgr/controlfilemgr.cpp
		../../kernel/io/src/sys/win/de_file.cpp
		../../kernel/mem/src/ltmemory.cpp
		../../kernel/mem/src/sys/win/de_memory.cpp
		../../kernel/net/src/localdriver.cpp
		../../kernel/net/src/netmgr.cpp
		../../kernel/net/src/packet.cpp
		../../kernel/net/src/sys/win/udpdriver.cpp
		../../kernel/src/debugging.cpp
		../../kernel/src/icommandlineargs.cpp
		../../kernel/src/sys/win/bindmgr.cpp
		../../kernel/src/sys/win/binkvideomgrimpl.cpp
		../../kernel/src/sys/win/client.cpp
		../../kernel/src/sys/win/counter.cpp
		../../kernel/src/sys/win/dshowvideomgrimpl.cpp
		../../kernel/src/sys/win/dsys_interface.cpp
		../../kernel/src/sys/win/dutil.cpp
		../../kernel/src/sys/win/ltjs_ffmpeg_video_mgr_impl.cpp
		../../kernel/src/sys/win/ltjs_fmv_player.cpp
		../../kernel/src/sys/win/input.cpp
		../../kernel/src/sys/win/inputsim.cpp
		../../kernel/src/sys/win/interface_helpers.cpp
		../../kernel/src/sys/win/load_pcx.cpp
		../../kernel/src/sys/win/lt_cursor_impl.cpp
		../../kernel/src/sys/win/ltdirectmusic_impl.cpp
		../../kernel/src/sys/win/ltdirectmusiccontrolfile.cpp
		../../kernel/src/sys/win/ltdirectmusicloader.cpp
		../../kernel/src/sys/win/lthread.cpp
		../../kernel/src/sys/win/ltlibraryloader.cpp
		../../kernel/src/sys/win/ltrendermgr_impl.cpp
		../../kernel/src/sys/win/ltthread.cpp
		../../kernel/src/sys/win/musicdriver.cpp
		../../kernel/src/sys/win/render.cpp
		../../kernel/src/sys/win/sdl_input.cpp
		../../kernel/src/sys/win/sdl_lt_cursor_impl.cpp
		../../kernel/src/sys/win/streamsim.cpp
		../../kernel/src/sys/win/stringmgr.cpp
		../../kernel/src/sys/win/systeminfo.cpp
		../../kernel/src/sys/win/text_mgr.cpp
		../../kernel/src/sys/win/timemgr.cpp
		../../kernel/src/sys/win/version_resource.cpp
		../../kernel/src/sys/win/videomgr.cpp
		../../model/src/animtracker.cpp
		../../model/src/model.cpp
		../../model/src/model_load.cpp
		../../model/src/modelallocations.cpp
		../../model/src/sys/d3d/d3d_model_load.cpp
		../../model/src/transformmaker.cpp
		../../render_b/src/sys/d3d/d3ddrawprim.cpp
		../../render_b/src/sys/d3d/d3dtexinterface.cpp
		../../server/src/classmgr.cpp
		../../server/src/game_serialize.cpp
		../../server/src/interlink.cpp
		../../server/src/ltmessage_server.cpp
		../../server/src/s_client.cpp
		../../server/src/s_concommand.cpp
		../../server/src/s_intersect.cpp
		../../server/src/s_net.cpp
		../../server/src/s_object.cpp
		../../server/src/server_consolestate.cpp
		../../server/src/server_extradata.cpp
		../../server/src/server_filemgr.cpp
		../../server/src/server_iltcommon.cpp
		../../server/src/server_iltmodel.cpp
		../../server/src/server_iltphysics.cpp
		../../server/src/server_iltsoundmgr.cpp
		../../server/src/serverde_impl.cpp
		../../server/src/serverevent.cpp
		../../server/src/servermgr.cpp
		../../server/src/smoveabstract.cpp
		../../server/src/soundtrack.cpp
		../../server/src/world_server_bsp.cpp
		../../shared/src/bdefs.cpp
		../../shared/src/classbind.cpp
		../../shared/src/collision.cpp
		../../shared/src/compress.cpp
		../../shared/src/concommand.cpp
		../../shared/src/conparse.cpp
		../../shared/src/debuggeometry.cpp
		../../shared/src/dhashtable.cpp
		../../shared/src/dtxmgr.cpp
		../../shared/src/engine_vars.cpp
		../../shared/src/findobj.cpp
		../../shared/src/ftclient.cpp
		../../shared/src/ftserv.cpp
		../../shared/src/gamemath.cpp
		../../shared/src/genericprop_setup.cpp
		../../shared/src/genltstream.cpp
		../../shared/src/geometry.cpp
		../../shared/src/geomroutines.cpp
		../../shared/src/impl_common.cpp
		../../shared/src/interface_linkage.cpp
		../../shared/src/leech.cpp
		../../shared/src/lightmap_compress.cpp
		../../shared/src/lightmap_planes.cpp
		../../shared/src/ltmessage.cpp
		../../shared/src/lttimer.cpp
		../../shared/src/modellt_impl.cpp
		../../shared/src/motion.cpp
		../../shared/src/moveobject.cpp
		../../shared/src/moveplayer.cpp
		../../shared/src/nexus.cpp
		../../shared/src/objectmgr.cpp
		../../shared/src/parse_world_info.cpp
		../../shared/src/pixelformat.cpp
		../../shared/src/ratetracker.cpp
		../../shared/src/shared_iltcommon.cpp
		../../shared/src/shared_iltphysics.cpp
		../../shared/src/spritecontrolimpl.cpp
		../../shared/src/stacktrace.cpp
		../../shared/src/stdlterror.cpp
		../../shared/src/strtools.cpp
		../../shared/src/sys/win/dstreamopenqueuemgr.cpp
		../../shared/src/transformlt_impl.cpp
		../../shared/src/version_info.cpp
		../../sound/src/ltjs_dmusic_manager.cpp
		../../sound/src/ltjs_dmusic_segment.cpp
		../../sound/src/soundbuffer.cpp
		../../sound/src/sounddata.cpp
		../../sound/src/soundinstance.cpp
		../../sound/src/soundmgr.cpp
		../../sound/src/wave.cpp
		../../world/src/de_mainworld.cpp
		../../world/src/de_nodes.cpp
		../../world/src/de_objects.cpp
		../../world/src/fullintersectline.cpp
		../../world/src/intersect_line.cpp
		../../world/src/intersectsweptsphere.cpp
		../../world/src/light_table.cpp
		../../world/src/world_blind_object_data.cpp
		../../world/src/world_blocker_data.cpp
		../../world/src/world_blocker_math.cpp
		../../world/src/world_particle_blocker_data.cpp
		../../world/src/world_shared_bsp.cpp
		../../world/src/world_tree.cpp
)

if (MSVC)
	target_sources (
		${PROJECT_NAME}
		PRIVATE
			lithtech.rc
			res/client.rc2
			ltjs_lithtech_win32_msvc.manifest
	)
endif ()


install (
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION
		"."
)
