cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)
project (ltjs_lib_freetype VERSION 0.0.1 LANGUAGES C)


add_library (${PROJECT_NAME} STATIC "")


set (
	LTJS_FREETYPE_SRC_PATH
	"${CMAKE_SOURCE_DIR}/engine/runtime/ui/src/freetype-2.0.1"
)


target_include_directories (
	${PROJECT_NAME}
	PUBLIC
		"${LTJS_FREETYPE_SRC_PATH}/include/freetype/config"
		"${LTJS_FREETYPE_SRC_PATH}/include/freetype"
		"${LTJS_FREETYPE_SRC_PATH}/include"
	PRIVATE
		"${LTJS_FREETYPE_SRC_PATH}/src"
)


target_sources (
	${PROJECT_NAME}
	PRIVATE
		${LTJS_FREETYPE_SRC_PATH}/src/autohint/ahangles.c
		${LTJS_FREETYPE_SRC_PATH}/src/autohint/ahglobal.c
		${LTJS_FREETYPE_SRC_PATH}/src/autohint/ahglyph.c
		${LTJS_FREETYPE_SRC_PATH}/src/autohint/ahhint.c
		${LTJS_FREETYPE_SRC_PATH}/src/autohint/ahmodule.c

		${LTJS_FREETYPE_SRC_PATH}/src/base/ftbbox.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftcalc.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftdebug.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftextend.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftglyph.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftinit.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftlist.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftmm.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftnames.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftobjs.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftoutln.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftstream.c
		${LTJS_FREETYPE_SRC_PATH}/src/base/ftsystem.c

		${LTJS_FREETYPE_SRC_PATH}/src/cache/ftlru.c
		${LTJS_FREETYPE_SRC_PATH}/src/cache/ftcmanag.c
		${LTJS_FREETYPE_SRC_PATH}/src/cache/ftcglyph.c
		${LTJS_FREETYPE_SRC_PATH}/src/cache/ftcchunk.c
		${LTJS_FREETYPE_SRC_PATH}/src/cache/ftcsbits.c
		${LTJS_FREETYPE_SRC_PATH}/src/cache/ftcimage.c

		${LTJS_FREETYPE_SRC_PATH}/src/cff/t2objs.c
		${LTJS_FREETYPE_SRC_PATH}/src/cff/t2load.c
		${LTJS_FREETYPE_SRC_PATH}/src/cff/t2gload.c
		${LTJS_FREETYPE_SRC_PATH}/src/cff/t2parse.c
		${LTJS_FREETYPE_SRC_PATH}/src/cff/t2driver.c

		${LTJS_FREETYPE_SRC_PATH}/src/cid/cidparse.c
		${LTJS_FREETYPE_SRC_PATH}/src/cid/cidload.c
		${LTJS_FREETYPE_SRC_PATH}/src/cid/cidriver.c
		${LTJS_FREETYPE_SRC_PATH}/src/cid/cidgload.c
		${LTJS_FREETYPE_SRC_PATH}/src/cid/cidobjs.c

		${LTJS_FREETYPE_SRC_PATH}/src/psaux/psobjs.c
		${LTJS_FREETYPE_SRC_PATH}/src/psaux/t1decode.c
		${LTJS_FREETYPE_SRC_PATH}/src/psaux/psauxmod.c

		${LTJS_FREETYPE_SRC_PATH}/src/psnames/psmodule.c

		${LTJS_FREETYPE_SRC_PATH}/src/raster/ftraster.c
		${LTJS_FREETYPE_SRC_PATH}/src/raster/ftrend1.c

		${LTJS_FREETYPE_SRC_PATH}/src/sfnt/ttload.c
		${LTJS_FREETYPE_SRC_PATH}/src/sfnt/ttcmap.c
		${LTJS_FREETYPE_SRC_PATH}/src/sfnt/ttsbit.c
		${LTJS_FREETYPE_SRC_PATH}/src/sfnt/ttpost.c
		${LTJS_FREETYPE_SRC_PATH}/src/sfnt/sfobjs.c
		${LTJS_FREETYPE_SRC_PATH}/src/sfnt/sfdriver.c

		${LTJS_FREETYPE_SRC_PATH}/src/smooth/ftgrays.c
		${LTJS_FREETYPE_SRC_PATH}/src/smooth/ftsmooth.c

		${LTJS_FREETYPE_SRC_PATH}/src/truetype/ttobjs.c
		${LTJS_FREETYPE_SRC_PATH}/src/truetype/ttpload.c
		${LTJS_FREETYPE_SRC_PATH}/src/truetype/ttgload.c
		${LTJS_FREETYPE_SRC_PATH}/src/truetype/ttinterp.c
		${LTJS_FREETYPE_SRC_PATH}/src/truetype/ttdriver.c

		${LTJS_FREETYPE_SRC_PATH}/src/type1/t1parse.c
		${LTJS_FREETYPE_SRC_PATH}/src/type1/t1load.c
		${LTJS_FREETYPE_SRC_PATH}/src/type1/t1driver.c
		${LTJS_FREETYPE_SRC_PATH}/src/type1/t1afm.c
		${LTJS_FREETYPE_SRC_PATH}/src/type1/t1gload.c
		${LTJS_FREETYPE_SRC_PATH}/src/type1/t1objs.c

		${LTJS_FREETYPE_SRC_PATH}/src/winfonts/winfnt.c
)
