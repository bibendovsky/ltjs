cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_d3d_render VERSION 0.0.1 LANGUAGES CXX)

option (LTJS_USE_DIRECT_MUSIC8 "Use DirectMusic8 for music." OFF)
option (LTJS_USE_D3DX9 "Use Direct3D 9 extensions." OFF)

if (LTJS_USE_DIRECT_MUSIC8 OR LTJS_USE_D3DX9)
	set (LTJS_USE_BUNDLED_DIRECTX ON)
endif ()

include (ltjs_common)

add_library (${PROJECT_NAME} STATIC)

ltjs_add_defaults (${PROJECT_NAME})

if (NOT (${CMAKE_VERSION} VERSION_LESS "3.16.0"))
	target_precompile_headers(
		${PROJECT_NAME}
		PRIVATE
			[["precompile.h"]]
	)
endif ()

target_compile_definitions (
	${PROJECT_NAME}
	PRIVATE
		__D3DREND
		__D3D
		MODEL_SUPPORT_ABC
		$<$<CONFIG:DEBUG>:D3D_DEBUG_INFO>
)

if (NOT MSVC)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			_XM_NO_INTRINSICS_
	)
endif ()

if (LTJS_USE_D3DX9)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			LTJS_USE_D3DX9
	)
endif ()

if (LTJS_USE_BUNDLED_DIRECTX)
	target_compile_definitions (
		${PROJECT_NAME}
		PRIVATE
			LTJS_USE_BUNDLED_DIRECTX
	)
endif ()

target_include_directories (
	${PROJECT_NAME}
	PRIVATE
		.
		../../../../../../libs/DirectXMath/Inc
		../../../../../../libs/stdlith
		../../../../../sdk/inc
		../../../../../sdk/inc/physics
		../../../../client/src
		../../../../kernel/io/src
		../../../../kernel/mem/src
		../../../../kernel/src
		../../../../kernel/src/sys/win
		../../../../lithtemplate
		../../../../model/src
		../../../../model/src/sys/d3d
		../../../../shared/src
		../../../../shared/src/sys/win
		../../../../world/src
)

if (LTJS_USE_BUNDLED_DIRECTX)
	target_include_directories (
		${PROJECT_NAME}
		PRIVATE
			../../../../../../directx/include
	)
endif ()

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../../../../../sdk/inc/iclientshell.h
		../../../../../sdk/inc/iltbenchmark.h
		../../../../../sdk/inc/iltclient.h
		../../../../../sdk/inc/iltcommon.h
		../../../../../sdk/inc/iltcsbase.h
		../../../../../sdk/inc/iltdirectmusic.h
		../../../../../sdk/inc/iltdrawprim.h
		../../../../../sdk/inc/iltinfo.h
		../../../../../sdk/inc/iltmath.h
		../../../../../sdk/inc/iltrenderstyles.h
		../../../../../sdk/inc/iltsoundmgr.h
		../../../../../sdk/inc/iltspritecontrol.h
		../../../../../sdk/inc/iltstream.h
		../../../../../sdk/inc/ilttexinterface.h
		../../../../../sdk/inc/lithtech.h
		../../../../../sdk/inc/ltanimtracker.h
		../../../../../sdk/inc/ltassert.h
		../../../../../sdk/inc/ltbasedefs.h
		../../../../../sdk/inc/ltbasetypes.h
		../../../../../sdk/inc/ltcodes.h
		../../../../../sdk/inc/ltinteger.h
		../../../../../sdk/inc/ltlink.h
		../../../../../sdk/inc/ltmatrix.h
		../../../../../sdk/inc/ltmodule.h
		../../../../../sdk/inc/ltplane.h
		../../../../../sdk/inc/ltpvalue.h
		../../../../../sdk/inc/ltquatbase.h
		../../../../../sdk/inc/ltrect.h
		../../../../../sdk/inc/ltrenderstyle.h
		../../../../../sdk/inc/ltrotation.h
		../../../../../sdk/inc/ltvector.h
		../../../../client/src/client_formatmgr.h
		../../../../kernel/mem/src/de_memory.h
		../../../../kernel/src/dsys.h
		../../../../kernel/src/sys/win/counter.h
		../../../../kernel/src/sys/win/dsys_interface.h
		../../../../kernel/src/sys/win/interface_helpers.h
		../../../../kernel/src/sys/win/lthread.h
		../../../../kernel/src/sys/win/render.h
		../../../../kernel/src/syscounter.h
		../../../../model/src/animtracker.h
		../../../../model/src/ltb.h
		../../../../model/src/model.h
		../../../../model/src/model_ops.h
		../../../../model/src/modelallocations.h
		../../../../model/src/transformmaker.h
		../../../../shared/src/bdefs.h
		../../../../shared/src/colorops.h
		../../../../shared/src/conparse.h
		../../../../shared/src/debuggeometry.h
		../../../../shared/src/dtxmgr.h
		../../../../shared/src/gamemath.h
		../../../../shared/src/geometry.h
		../../../../shared/src/geomroutines.h
		../../../../shared/src/lightmap_compress.h
		../../../../shared/src/nexus.h
		../../../../shared/src/objectmgr.h
		../../../../shared/src/pixelformat.h
		../../../../shared/src/renderinfostruct.h
		../../../../shared/src/renderobject.h
		../../../../shared/src/sys/win/d3dddstructs.h
		../../../../shared/src/sys/win/renderstruct.h
		../../../../shared/src/sysddstructs.h
		../../../../shared/src/version_info.h
		../../../../world/src/de_mainworld.h
		../../../../world/src/de_objects.h
		../../../../world/src/de_sprite.h
		../../../../world/src/de_world.h
		../../../../world/src/fullintersectline.h
		../../../../world/src/intersect_line.h
		../../../../world/src/light_table.h
		../../../../world/src/world_tree.h
		../../../../world/src/worldtreehelper.h
		../shadows/d3dshadowtexture.h
		3d_ops.h
		aabb.h
		clipline.h
		common_draw.h
		common_init.h
		common_stuff.h
		d3d_convar.h
		d3d_defs.h
		d3d_device.h
		d3d_device_wrapper.h
		d3d_draw.h
		d3d_init.h
		d3d_renderblock.h
		d3d_rendershader.h
		d3d_rendershader_base.h
		d3d_rendershader_default.h
		d3d_rendershader_dynamiclight.h
		d3d_rendershader_glow.h
		d3d_rendershader_gouraud.h
		d3d_rendershader_gouraud_fullbright.h
		d3d_rendershader_lightmap.h
		d3d_renderstatemgr.h
		d3d_renderstyle.h
		d3d_renderstyleinterface.h
		d3d_renderworld.h
		d3d_renderworld_occluder.h
		d3d_shell.h
		d3d_surface.h
		d3d_texture.h
		d3d_utils.h
		d3d_viewparams.h
		d3dmeshrendobj_rigid.h
		d3dmeshrendobj_skel.h
		d3dmeshrendobj_vertanim.h
		devicelightlist.h
		dirtyrect.h
		draw_canvas.h
		drawlight.h
		drawobjects.h
		drawsky.h
		drawworldmodel.h
		fixedpoint.h
		iaggregateshader.h
		lteffectimpl.h
		lteffectinclude.h
		lteffectshadermgr.h
		ltpixelshadermgr.h
		ltshaderdevicestateimp.h
		ltvertexshadermgr.h
		memstats_world.h
		modeldebug.h
		modelshadowshader.h
		objectgroupmgr.h
		polyclip.h
		precompile.h
		relevantlightlist.h
		rendererconsolevars.h
		rendererframestats.h
		renderlight.h
		rendermodelinfolist.h
		rendermodelpiecelist.h
		rendershadowlist.h
		renderstylelookuptables.h
		renderstylemap.h
		rendertarget.h
		rendertargetmgr.h
		screenglowmgr.h
		setupmodel.h
		setuptouchinglights.h
		simd_utilities.h
		tagnodes.h
		texturescriptdefs.h
		texturescriptevaluator.h
		texturescriptinstance.h
		texturescriptinterpreter.h
		texturescriptmgr.h
		texturescriptvarmgr.h
		vertexbuffercontroller.h
)

target_sources (
	${PROJECT_NAME}
	PRIVATE
		../shadows/d3dshadowtexture.cpp
		3d_ops.cpp
		aabb.cpp
		common_draw.cpp
		common_init.cpp
		common_stuff.cpp
		d3d_device.cpp
		d3d_device_wrapper.cpp
		d3d_draw.cpp
		d3d_drawsky.cpp
		d3d_init.cpp
		d3d_optimizedsurface.cpp
		d3d_renderblock.cpp
		d3d_rendershader_dynamiclight.cpp
		d3d_rendershader_glow.cpp
		d3d_rendershader_gouraud.cpp
		d3d_rendershader_gouraud_fullbright.cpp
		d3d_rendershader_lightmap.cpp
		d3d_renderstatemgr.cpp
		d3d_renderstyle.cpp
		d3d_renderstyleinterface.cpp
		d3d_renderworld.cpp
		d3d_renderworld_occluder.cpp
		d3d_shell.cpp
		d3d_surface.cpp
		d3d_texture.cpp
		d3d_utils.cpp
		d3d_viewparams.cpp
		d3dmeshrendobj_rigid.cpp
		d3dmeshrendobj_skel.cpp
		d3dmeshrendobj_vertanim.cpp
		devicelightlist.cpp
		dirtyrect.cpp
		draw_canvas.cpp
		drawlight.cpp
		drawlinesystem.cpp
		drawmodel.cpp
		drawobjects.cpp
		drawparticles.cpp
		drawparticles_a.cpp
		drawpolygrid.cpp
		drawsky.cpp
		drawsprite.cpp
		drawvolumeeffects.cpp
		drawworldmodel.cpp
		lteffectimpl.cpp
		lteffectinclude.cpp
		lteffectshadermgr.cpp
		ltpixelshadermgr.cpp
		ltshaderdevicestateimp.cpp
		ltvertexshadermgr.cpp
		memstats_world.cpp
		modeldebug.cpp
		modelshadowshader.cpp
		objectgroupmgr.cpp
		precompile.cpp
		relevantlightlist.cpp
		rendererconsolevars.cpp
		rendererframestats.cpp
		renderlight.cpp
		rendermodelinfolist.cpp
		rendermodelpiecelist.cpp
		rendershadowlist.cpp
		renderstylelookuptables.cpp
		renderstylemap.cpp
		rendertarget.cpp
		rendertargetmgr.cpp
		screenglowmgr.cpp
		setupmodel.cpp
		setuptouchinglights.cpp
		tagnodes.cpp
		texturescriptinstance.cpp
		texturescriptinterpreter.cpp
		texturescriptmgr.cpp
		texturescriptvarmgr.cpp
		vertexbuffercontroller.cpp
)
