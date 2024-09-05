//-------------------------------------------------------------------
//
//   MODULE    : CUIVECTORFONT.CPP
//
//   PURPOSE   : implements the CUIVectorFont font class
//
//   CREATED   : 1/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifdef LTJS_SDL_BACKEND


#include <cmath>

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "bdefs.h"
#include "dtxmgr.h"
#include "sysstreamsim.h"

#include "cuidebug.h"
#include "cuivectorfont.h"

#include "ltsysoptim.h"
#include "ltfontparams.h"
#include "iltclient.h"
#include "interface_helpers.h"

#include "SDL.h"

#include "ftbuild.h"
#include FT_FREETYPE_H

#include "ltjs_sdl_uresources.h"
#include "ltjs_shared_data_mgr.h"
#include "ltjs_shell_resource_mgr.h"
#include "ltjs_ucs.h"
#include "ltjs_windows_1252.h"


// get the ILTTexInterface from the interface database
static ILTTexInterface* pTexInterface = nullptr;
define_holder(ILTTexInterface, pTexInterface);

//ILTClient game interface
static ILTClient* ilt_client;
define_holder(ILTClient, ilt_client);


struct FtLibraryUDeleter
{
	void operator()(
		::FT_Library resource) const noexcept
	{
		::FT_Done_FreeType(resource);
	}
}; // FtLibraryUDeleter

using FtLibraryUPtr = std::unique_ptr<std::remove_pointer_t<::FT_Library>, FtLibraryUDeleter>;


struct FtFaceUDeleter
{
	void operator()(
		::FT_Face resource) const noexcept
	{
		::FT_Done_Face(resource);
	}
}; // FtFaceUDeleter

using FtFaceUPtr = std::unique_ptr<std::remove_pointer_t<::FT_Face>, FtFaceUDeleter>;


struct Point
{
	int x{};
	int y{};
}; // Point

struct Size
{
	int cx{};
	int cy{};
}; // Point

using CpCodePoints = std::vector<char>;

struct Glyph
{
	int index{};
	ltjs::ucs::CodePoint code_point{};
	char cp_code_point{};
	int width{};
	int height{};
	int horizontal_advance{};
	int bitmap_width{};
	int bitmap_height{};
	Point bitmap_offset{};
}; // Glyph

using Glyphs = std::vector<Glyph>;


struct TextMetric
{
	int tmDescent{};
	int max_width{};
	int tmHeight{};
}; // TextMetric


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WIDTHBYTES
//
//  PURPOSE:	Find the pitch of a bitmap.
//
//	nBitCount -	Number of bits per pixel.
//
//	nPixelWidth	-	Width in pixels.
//
// ----------------------------------------------------------------------- //

#define WIDTHBYTES( nBitCount, nPixelWidth ) (((( uint32 )nBitCount * nPixelWidth + 7) / 8 + 3) & ~3 )


// ----------------------------------------------------------------------- //
//
//  Class:	InstalledFontFace
//
//  PURPOSE:	Container for HFONT and font resource file.
//
// ----------------------------------------------------------------------- //

class InstalledFontFace
{
public:
	// Intialize the font face, from optional font file.
	bool Init(
		char const* pszFontFile,
		char const* pszFontFace,
		int nHeight,
		LTFontParams* fontParams);

	::FT_Face GetHFont() noexcept
	{
		return ft_face_.get();
	}

	int GetHeight() const noexcept
	{
		return m_nHeight;
	}


private:
	using FontData = std::vector<::FT_Byte>;


	FontData font_data_{};
	FtLibraryUPtr ft_library_{};
	FtFaceUPtr ft_face_{};

	int m_nHeight{};


	void reset();
};

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	InstalledFontFace::Init
//
//  PURPOSE:	Intialize the font face, from optional font file.
//
//	pszFontFile	-	Specifies font resource file to install.  If left
//					NULL, then the font face must already be installed on
//					the system.
//
//	pszFontFace -	Font face to create.
//
//	nHeight	-		Height of the font.
//
// ----------------------------------------------------------------------- //

bool InstalledFontFace::Init(
	char const* pszFontFile,
	char const* pszFontFace,
	int nHeight,
	LTFontParams* fontParams)
{
	reset();

	if (!pszFontFile || nHeight <= 0)
	{
		ASSERT(!"Invalid parameters.");
		return false;
	}

	if (fontParams)
	{
		DEBUG_PRINT(1, ("[%s] SKIPPING user defined Font Params", pszFontFile));
	}

	constexpr auto max_font_data_size = 1 * 1'024 * 1'024;

	const auto font_data_size = dsi_get_file_size(pszFontFile);

	if (font_data_size <= 0 || font_data_size > max_font_data_size)
	{
		DEBUG_PRINT(1, ("[%s] Font data size out of range.", pszFontFile));
		return false;
	}

	auto font_data = FontData{};
	font_data.resize(font_data_size);

	const auto load_file_result = dsi_load_file_into_memory(
		pszFontFile,
		font_data.data(),
		font_data_size
	);

	if (!load_file_result)
	{
		DEBUG_PRINT(1, ("[%s] Failed to load font data into memory.", pszFontFile));
		return false;
	}

	auto ft_error = ::FT_Error{};

	auto raw_ft_library = ::FT_Library{};
	const auto ft_init_result = ::FT_Init_FreeType(&raw_ft_library);

	if (ft_error != ::FT_Err_Ok)
	{
		DEBUG_PRINT(1, ("[%s] Failed to initialize FreeType.", pszFontFile));
		return false;
	}

	auto ft_library = FtLibraryUPtr{raw_ft_library};

	auto raw_ft_face = ::FT_Face{};

	ft_error = ::FT_New_Memory_Face(
		ft_library.get(),
		font_data.data(),
		static_cast<::FT_Long>(font_data_size),
		0,
		&raw_ft_face
	);

	if (ft_error != 0)
	{
		DEBUG_PRINT(1, ("[%s] Failed to initialize FreeType face.", pszFontFile));
		return false;
	}

	auto ft_face = FtFaceUPtr{raw_ft_face};

	if (ft_face->num_faces != 1)
	{
		DEBUG_PRINT(1, ("[%s] Expected only one face.", pszFontFile));
		return false;
	}

	if ((ft_face->face_flags & FT_FACE_FLAG_SCALABLE) == 0)
	{
		DEBUG_PRINT(1, ("[%s] Expected scalable face.", pszFontFile));
		return false;
	}

	DEBUG_PRINT(1, ("[%s] Family: %s.", pszFontFile, ft_face->family_name));
	DEBUG_PRINT(1, ("[%s] Style: %s.", pszFontFile, ft_face->style_name));

	const auto char_height_px = (nHeight * 1000) / 1211;
	const auto char_width_px = (nHeight * 1000) / 1224;

	ft_error = ::FT_Set_Pixel_Sizes(
		ft_face.get(),
		char_width_px,
		char_height_px
	);

	if (ft_error != ::FT_Err_Ok)
	{
		DEBUG_PRINT(1, ("[%s] Failed to set face size.", pszFontFile));
		return false;
	}

	font_data_.swap(font_data);
	ft_library_.swap(ft_library);
	ft_face_.swap(ft_face);

	m_nHeight = nHeight;

	return true;
}

void InstalledFontFace::reset()
{
	font_data_.clear();
	ft_library_ = nullptr;
	ft_face_ = nullptr;

	m_nHeight = 0;
}


namespace
{


// Spacing between each character in font map.
constexpr auto kCharSpacing = 2;


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetPowerOfTwo
//
//  PURPOSE:	Get the smallest power of two that is greater than the
//				input value.
//
//	nValue -	Value to start with.
//
// ----------------------------------------------------------------------- //

int GetPowerOfTwo(
	int nValue)
{
	int nPowerOfTwo = 32;

	while (nPowerOfTwo < nValue)
	{
		nPowerOfTwo *= 2;
	}

	return nPowerOfTwo;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetTextureSizeFromCharSizes
//
//  PURPOSE:	Finds the texture size that will fit character glyphs.
//
//	pGlyphMetrics -	Array of GLYPHMETRICS of each glpyh in font.
//
//	sizeMaxGlyphSize - Size of largest glyph.
//
//	nLen -			Number of characters in pCharSizes.
//
//	sizeTexture -	Function fills in with texture size.
//
// ----------------------------------------------------------------------- //

void GetTextureSizeFromCharSizes(
	const Glyphs& glyphs,
	const Size& sizeMaxGlyphSize,
	Size& sizeTexture)
{
	// Get the total area of the pixels of all the characters.  We use the largest glyph size
	// rather than the exact values because this is just a rough
	// guess and if we overestimate in width, we just get a shorter texture.
	const auto nTotalPixelArea =
		(sizeMaxGlyphSize.cx + kCharSpacing) *
		static_cast<int>(glyphs.size()) *
		(sizeMaxGlyphSize.cy + kCharSpacing);

	// Use the square root of the area guess at the width.
	const auto nRawWidth = static_cast<int>(std::sqrt(static_cast<double>(nTotalPixelArea)) + 0.5);

	// Englarge the width to the nearest power of two and use that as our final width.
	sizeTexture.cx = GetPowerOfTwo(nRawWidth);

	const auto max_spaced_glyph_height = sizeMaxGlyphSize.cy + kCharSpacing;

	// Start the height off as one row.
	auto nRawHeight = max_spaced_glyph_height;

	// To find the height, keep putting characters into the rows until we reach the bottom.
	auto nXOffset = 0;

	for (const auto& glyph : glyphs)
	{
		const auto spaced_glyph_width = glyph.bitmap_width + kCharSpacing;

		nXOffset += spaced_glyph_width;

		if (nXOffset > sizeTexture.cx)
		{
			nXOffset = spaced_glyph_width;
			nRawHeight += max_spaced_glyph_height;
		}
	}

	// Enlarge the height to the nearest power of two and use that as our final height.
	sizeTexture.cy = GetPowerOfTwo(nRawHeight);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CopyGlyphBitmapToPixelData
//
//  PURPOSE:	Copies glphy bitmap to region of pixel data.
//
//	bmi	-			Bitmap info of source.
//
//	pDibBits -		Bitmap bits of glyph.
//
//	glyphMetrics -	Glyph metrics of this glyph.
//
//	pPixelData -	Pointer to beginning of pixel data region.
//
//	nPixelDataPitch -	Pitch of pixel data.
//
//	nGlyphWidth -	Width of glyph pixels.
//
//	nGlyphHeight -	Height of glyph pixels.
//
// ----------------------------------------------------------------------- //

void CopyGlyphBitmapToPixelData(
	const TextMetric& text_metric,
	const ::FT_GlyphSlot ft_glyph,
	uint8* dst_data,
	int dst_image_width)
{
	int dst_pitch = WIDTHBYTES(16, dst_image_width);

	const auto top_margin = text_metric.tmHeight - text_metric.tmDescent - ft_glyph->bitmap_top;

	if (top_margin > 0)
	{
		dst_data += dst_pitch * top_margin;
	}

	const auto& ft_bitmap = ft_glyph->bitmap;

	const auto height = ABS(ft_bitmap.rows);

	if (height == 0)
	{
		return;
	}

	auto src_pixels = ft_bitmap.buffer;

	if (ft_bitmap.rows < 0)
	{
		src_pixels += (height - 1) * ft_bitmap.pitch;
	}

	for (auto h = 0; h < height; ++h)
	{
		auto src_row = src_pixels;
		auto dst_row = reinterpret_cast<uint16*>(dst_data);

		for (auto w = 0; w < ft_bitmap.width; ++w)
		{
			const auto alpha_4 = (15 * (*src_row++)) / 255;
			auto& dst_pixel = *dst_row++;

			dst_pixel |= static_cast<uint16>(alpha_4 << 12);
		}

		src_pixels += ft_bitmap.pitch;
		dst_data += dst_pitch;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WritePixelDataBitmap
//
//  PURPOSE:	Writes pixel data texture to a bitmap file.  For debugging.
//
//	pPixelData -	pixel data bits.
//
//	sizeTexture -	size of pixel data.
//
//	pszFilename -	Filename to use.
//
// ----------------------------------------------------------------------- //

void WritePixelDataBitmap(
	uint8* src_bytes,
	Size& sizeTexture,
	char const* pszFilename)
{
	assert(src_bytes);
	assert(sizeTexture.cx >= 0);
	assert(sizeTexture.cy >= 0);
	assert(pszFilename);

	const auto sdl_pixel_format =
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		::SDL_PIXELFORMAT_ARGB8888
#else
		::SDL_PIXELFORMAT_BGRA8888
#endif
	;

	const auto sdl_surface = ltjs::SdlSurfaceUResource{::SDL_CreateRGBSurfaceWithFormat(
		0,
		sizeTexture.cx,
		sizeTexture.cy,
		32,
		sdl_pixel_format
	)};

	if (!sdl_surface)
	{
		assert(!"Failed to create SDL surface.");
		return;
	}

	if (SDL_MUSTLOCK(sdl_surface.get()))
	{
		const auto sdl_lock_result = ::SDL_LockSurface(sdl_surface.get());

		if (sdl_lock_result != 0)
		{
			assert(!"Failed to lock SDL surface.");
			return;
		}
	}

	const auto src_pitch = WIDTHBYTES(16, sizeTexture.cx);
	auto dst_bytes = static_cast<uint8*>(sdl_surface->pixels);

	for (auto h = 0; h < sizeTexture.cy; ++h)
	{
		auto src_row = reinterpret_cast<const uint16*>(src_bytes);
		auto dst_row = reinterpret_cast<::Uint32*>(dst_bytes);

		for (auto w = 0; w < sizeTexture.cx; ++w)
		{
			const auto src_pixel = (*src_row++);

			if (src_pixel == 0x00F0)
			{
				(*dst_row++) = ::SDL_MapRGB(
					sdl_surface->format,
					0,
					0xFF,
					0
				);
			}
			else
			{
				const auto src_alpha_4 = (src_pixel >> 12) & 0xF;
				const auto grey_256 = static_cast<::Uint8>((255 * src_alpha_4) / 15);

				(*dst_row++) = ::SDL_MapRGB(
					sdl_surface->format,
					grey_256,
					grey_256,
					grey_256
				);
			}
		}

		src_bytes += src_pitch;
		dst_bytes += sdl_surface->pitch;
	}

	if (SDL_MUSTLOCK(sdl_surface.get()))
	{
		::SDL_UnlockSurface(sdl_surface.get());
	}

	::SDL_SaveBMP(sdl_surface.get(), pszFilename);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WritePixelDataTGA
//
//  PURPOSE:	Writes pixel data texture to a TGA file.  For texture bitmaps
//
//	pPixelData -	pixel data bits sixteen bit ARGB pixels. 
//
//	sizeTexture -	size of pixel data.
//
//	pszFilename -	Filename to use.
//
// ----------------------------------------------------------------------- //

void WritePixelDataTGA(
	uint8* pPixelData,
	Size& sizeTexture,
	char const* pszFilename)
{
	// Save the bitmap.
	FILE* fp = fopen(pszFilename, "wb");
	if (!fp)
	{
		return;
	}


	{
		const auto m_IDLength = uint8{};
		fwrite(&m_IDLength, sizeof(m_IDLength), 1, fp);
	}
	{
		const auto m_ColorMapType = uint8{};
		fwrite(&m_ColorMapType, sizeof(m_ColorMapType), 1, fp);
	}
	{
		const auto m_ImageType = uint8{2};
		fwrite(&m_ImageType, sizeof(m_ImageType), 1, fp);
	}
	{
		const auto m_CMapStart = uint16{};
		fwrite(&m_CMapStart, sizeof(m_CMapStart), 1, fp);
	}
	{
		const auto m_CMapLength = uint16{};
		fwrite(&m_CMapLength, sizeof(m_CMapLength), 1, fp);
	}
	{
		const auto m_CMapDepth = uint8{};
		fwrite(&m_CMapDepth, sizeof(m_CMapDepth), 1, fp);
	}
	{
		const auto m_XOffset = uint16{};
		fwrite(&m_XOffset, sizeof(m_XOffset), 1, fp);
	}
	{
		const auto m_YOffset = uint16{};
		fwrite(&m_YOffset, sizeof(m_YOffset), 1, fp);
	}
	{
		const auto m_Width = SDL_SwapLE16(static_cast<uint16>(sizeTexture.cx));
		fwrite(&m_Width, sizeof(m_Width), 1, fp);
	}
	{
		const auto m_Height = SDL_SwapLE16(static_cast<uint16>(sizeTexture.cy));
		fwrite(&m_Height, sizeof(m_Height), 1, fp);
	}
	{
		const auto m_PixelDepth = uint8{32};
		fwrite(&m_PixelDepth, sizeof(m_PixelDepth), 1, fp);
	}
	{
		const auto m_ImageDescriptor = uint8{1 << 5}; // says image is flipped origin 0,1;
		fwrite(&m_ImageDescriptor, sizeof(m_ImageDescriptor), 1, fp);
	}

	const int nPixelDataPitch = WIDTHBYTES(16, sizeTexture.cx);

	for (int y = 0; y < sizeTexture.cy; y++)
	{
		auto pSrcPos = reinterpret_cast<uint16*>(pPixelData + (y * nPixelDataPitch));
		uint32 Dest;

		for (int x = 0; x < sizeTexture.cx; x++)
		{
			// Special for green dot 
			if (pSrcPos[0] == 0x00F0)
			{
				Dest = SDL_SwapLE32(0x0000FF00);
			}
			else
			{
				// Get the alpha value and scale it into 5 bits.
				uint32 nVal = ((pSrcPos[0] & 0xF000) >> 12);

				nVal = MulDiv(nVal, 0xFF, 0x0F);

				if (nVal)
				{
					Dest = SDL_SwapLE32((nVal << 24) | 0x00FFFFFF);
				}
				else
				{
					Dest = 0; // no pixel here 
				}
			}

			// write out the pixel
			fwrite(&Dest, sizeof(Dest), 1, fp);

			pSrcPos++;
		}
	}

	fclose(fp);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetGlyphSizes
//
//  PURPOSE:	Get the glyph sizes for a font.
//
//	hDC			- Device context.
//
//	pszChars	- Characters in font.
//
//	nLen		- Number of chars in pszChars.
//
//	pGlyphSizes	- Array of glyph sizes.  Must be nLen long.
//
//	sizMaxGlyphSize - Maximum size of pGlyphSizes
//
// ----------------------------------------------------------------------- //

bool ltjs_load_ft_glyph_by_index(
	::FT_Face ft_face,
	::FT_UInt ft_glyph_index)
{
	auto ft_error = ::FT_Error{};

	ft_error = ::FT_Load_Glyph(ft_face, ft_glyph_index, FT_LOAD_NO_BITMAP);

	if (ft_error != ::FT_Err_Ok)
	{
		return false;
	}

	const auto glyph = ft_face->glyph;

	if (glyph->format != ::ft_glyph_format_bitmap)
	{
		ft_error = ::FT_Render_Glyph(glyph, ::ft_render_mode_normal);

		if (ft_error != ::FT_Err_Ok)
		{
			return false;
		}
	}

	if (glyph->bitmap.pixel_mode != ::ft_pixel_mode_grays)
	{
		return false;
	}

	return true;
}

bool ltjs_load_default_ft_glyph(
	::FT_Face ft_face)
{
	return ltjs_load_ft_glyph_by_index(ft_face, 0);
}

bool ltjs_load_ft_glyph_by_code_point(
	::FT_Face ft_face,
	ltjs::ucs::CodePoint code_point)
{
	const auto ft_glyph_index = ::FT_Get_Char_Index(ft_face, static_cast<::FT_ULong>(code_point));

	if (ft_glyph_index == 0)
	{
		return false;
	}

	return ltjs_load_ft_glyph_by_index(ft_face, ft_glyph_index);
}

bool ltjs_get_glyph(
	::FT_Face ft_face,
	ltjs::ucs::CodePoint code_point,
	Glyph& glyph)
{
	if (code_point != 0)
	{
		if (!ltjs_load_ft_glyph_by_code_point(ft_face, code_point))
		{
			return false;
		}
	}
	else
	{
		if (!ltjs_load_default_ft_glyph(ft_face))
		{
			return false;
		}
	}

	glyph.code_point = code_point;

	const auto& ft_glyph = ft_face->glyph;
	const auto& ft_bitmap = ft_glyph->bitmap;

	glyph.width = ft_glyph->metrics.width / 64;
	glyph.height = ft_glyph->metrics.height / 64;

	glyph.horizontal_advance = ft_glyph->metrics.horiAdvance / 64;

	glyph.bitmap_width = ft_bitmap.width;
	glyph.bitmap_height = ABS(ft_bitmap.rows);

	glyph.bitmap_offset.x = ft_glyph->bitmap_left;
	glyph.bitmap_offset.y = ft_glyph->bitmap_top;

	return true;
}

int ltjs_get_default_char_screen_width(
	::FT_Face ft_face,
	const TextMetric& text_metric)
{
	auto default_char_screen_width = 0;

	auto glyph_space = Glyph{};

	if (ltjs_get_glyph(ft_face, ' ', glyph_space))
	{
		if (default_char_screen_width == 0)
		{
			if (glyph_space.bitmap_width > 0)
			{
				default_char_screen_width = glyph_space.bitmap_width;
			}
		}

		if (default_char_screen_width == 0)
		{
			if (glyph_space.width > 0)
			{
				default_char_screen_width = glyph_space.width;
			}
		}

		if (default_char_screen_width == 0)
		{
			if (glyph_space.horizontal_advance > 0)
			{
				default_char_screen_width = glyph_space.horizontal_advance;
			}
		}
	}

	if (default_char_screen_width == 0)
	{
		default_char_screen_width = text_metric.max_width / 4;
	}

	if (default_char_screen_width < 2)
	{
		default_char_screen_width = 2;
	}

	return default_char_screen_width;
}

ltjs::ucs::CodePoint ltjs_cp_to_ucs(
	ltjs::ShellResourceCodePage code_page,
	char cp_code_point)
{
	switch (code_page)
	{
		case ltjs::ShellResourceCodePage::windows_1252:
			return ltjs::windows_1252::to_unicode(cp_code_point);

		default:
			assert(!"Unsupported code page.");
			return 0;
	}
}

bool GetGlyphSizes(
	::FT_Face ft_face,
	const CpCodePoints& cp_code_points,
	Glyphs& glyphs,
	int& max_bitmap_width)
{
	glyphs.clear();

	max_bitmap_width = 0;

	const auto cres_mgr = ltjs::get_shared_data_mgr().get_cres_mgr();

	if (!cres_mgr)
	{
		return false;
	}

	const auto code_page = cres_mgr->get_code_page();

	glyphs.reserve(cp_code_points.size() + 1);

	auto glyph = Glyph{};

	auto is_default = true;

	const auto cp_code_point_count = static_cast<int>(cp_code_points.size());

	for (auto i = 0; i < cp_code_point_count; )
	{
		const auto cp_code_point = (is_default ? '\0' : cp_code_points[i]);
		const auto code_point = (is_default ? 0 : ltjs_cp_to_ucs(code_page, cp_code_point));
		i += (is_default ? 0 : 1);

		is_default = false;

		if (!ltjs_get_glyph(ft_face, code_point, glyph))
		{
			if (code_point == 0)
			{
				return false;
			}
			else
			{
				continue;
			}
		}

		glyph.index = static_cast<int>(glyphs.size());
		glyph.cp_code_point = cp_code_point;
		glyphs.emplace_back(glyph);

		max_bitmap_width = Max(max_bitmap_width, glyph.bitmap_width);
	}

	return true;
}


} // namespace


class CUIVectorFont::Detail
{
public:
	Detail(
		CUIVectorFont& ui_vector_font) noexcept;


	bool CreateFontTextureAndTable(
		InstalledFontFace& installedFontFace,
		char const* pszChars,
		bool bMakeMap);


private:
	CUIVectorFont& ui_vector_font_;


	static CpCodePoints make_cp_code_points(
		const char* chars);
}; // CUIVectorFont::Detail


CUIVectorFont::Detail::Detail(
	CUIVectorFont& ui_vector_font) noexcept
	:
	ui_vector_font_{ui_vector_font}
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CUIVectorFont::CreateFontTextureAndTable
//
//  PURPOSE:	Creates font pixel data and mapping and size tables.
//
//	installedFontFace - Font face to use.
//
//	pszChars -			Chars to use in font.
//
//	bMakeMap -			Make a character map.
//
// ----------------------------------------------------------------------- //

bool CUIVectorFont::Detail::CreateFontTextureAndTable(
	InstalledFontFace& installedFontFace,
	char const* pszChars,
	bool bMakeMap)
{
	// sanity check
	if (!pTexInterface)
	{
		DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable: No Interface"));
		return false;
	}

	// Check inputs.
	if (!pszChars || pszChars[0] == '\0')
	{
		DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable: Invalid parameters"));
		return false;
	}

	// Get the font to use.
	auto ft_face = installedFontFace.GetHFont();

	if (!ft_face)
	{
		DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable: Invalid font."));
		return false;
	}

	// This will hold the size of the texture used for rendering.
	auto sizeTexture = Size{};

	// This will hold the sizes of each glyph in the font.  Index 0 of aGlyphSizes is for index 0 of pszChars.
	auto glyphs = Glyphs{};
	auto sizeMaxGlyphSize = Size{};

	// This will hold the GDI font metrics.
	auto textMetric = TextMetric{};

	auto bOk = true;

	if (bOk)
	{
		const auto& ft_size = ft_face->size->metrics;
		const auto& bbox = ft_face->bbox;
		const auto upem = ft_face->units_per_EM;

		const auto max_width_em = bbox.xMax - ft_face->bbox.xMin;
		const auto max_width_px = (ft_size.x_ppem * max_width_em) / upem;

		const auto max_height_em = bbox.yMax - ft_face->bbox.yMin;
		const auto max_height_px = (ft_size.y_ppem * max_height_em) / upem;

		textMetric.tmDescent = (ft_size.y_ppem * (-ft_face->bbox.yMin)) / upem;
		textMetric.max_width = max_width_px;
		textMetric.tmHeight = max_height_px;

		const auto cp_code_points = make_cp_code_points(pszChars);

		// Get the sizes of each glyph.
		bOk = GetGlyphSizes(ft_face, cp_code_points, glyphs, sizeMaxGlyphSize.cx);

		sizeMaxGlyphSize.cy = max_height_px;
	}

	if (bOk)
	{
		// Get the size of the default character.
		ui_vector_font_.m_DefaultCharScreenWidth = static_cast<uint8>(
			ltjs_get_default_char_screen_width(ft_face, textMetric)
		);

		const auto& default_glyph = glyphs.front();
		ui_vector_font_.m_DefaultCharScreenHeight = static_cast<uint8>(default_glyph.bitmap_height);

		ui_vector_font_.m_DefaultVerticalSpacing = static_cast<uint8>(
			(ui_vector_font_.m_DefaultCharScreenHeight / 4.0) + 0.5
		);

		// Get the average info on the characters.  The width isn't used
		// for proportional fonts, so using an average is ok.
		ui_vector_font_.m_CharTexWidth = static_cast<uint8>(textMetric.max_width);
		ui_vector_font_.m_CharTexHeight = static_cast<uint8>(textMetric.tmHeight);

		// Get the size our font texture should be to hold all the characters.
		GetTextureSizeFromCharSizes(glyphs, sizeMaxGlyphSize, sizeTexture);

		// allocate the font table.  This contains the texture offsets for
		// the characters.
		LT_MEM_TRACK_ALLOC(ui_vector_font_.m_pFontTable = new uint16[glyphs.size() * 3], LT_MEM_TYPE_UI);
		bOk = ui_vector_font_.m_bAllocatedTable = (ui_vector_font_.m_pFontTable != nullptr);
	}

	if (bOk)
	{
		// allocate the font map.  This contains the character mappings.
		if (bMakeMap)
		{
			LT_MEM_TRACK_ALLOC(ui_vector_font_.m_pFontMap = new uint8[256], LT_MEM_TYPE_UI);
			std::uninitialized_fill_n(ui_vector_font_.m_pFontMap, 256, uint8{});
			bOk = ui_vector_font_.m_bAllocatedMap = (ui_vector_font_.m_pFontMap != nullptr);
		}
	}

	// This will be filled in with the pixel data of the font.
	uint8* pPixelData = nullptr;

	// Calculate the pixeldata pitch.
	const auto nPixelDataPitch = WIDTHBYTES(16, sizeTexture.cx);
	const auto nPixelDataSize = nPixelDataPitch * sizeTexture.cy;

	if (bOk)
	{
		// Allocate an array to copy the font into.
		LT_MEM_TRACK_ALLOC(pPixelData = new uint8[nPixelDataSize], LT_MEM_TYPE_UI);
		bOk = (pPixelData != nullptr);

		if (!bOk)
		{
			DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable:  Failed to create pixeldata."));
		}
	}

	if (bOk)
	{
		// set the whole font texture to pure white, with alpha of 0.  When
		// we copy the glyph from the bitmap to the pixeldata, we just
		// affect the alpha, which allows the font to antialias with any color.

		std::uninitialized_fill_n(
			reinterpret_cast<uint16*>(pPixelData),
			nPixelDataSize / 2,
			uint16{0x0FFF}
		);

		// This will hold the UV offset for the font texture.
		auto sizeOffset = Point{};

		// Iterate over the characters.
		for (const auto& glyph : glyphs)
		{
			// Get this character's width.
			const auto nCharWidthWithSpacing = glyph.bitmap_width + kCharSpacing;

			// See if this width fits in the current row.
			const auto nCharRightSide = sizeOffset.x + nCharWidthWithSpacing;

			if (nCharRightSide >= sizeTexture.cx)
			{
				// Doesn't fit in the current row.  Go to the next row.
				sizeOffset.x = 0;
				sizeOffset.y += sizeMaxGlyphSize.cy + kCharSpacing;
			}

			const auto glyph_index = glyph.index;

			// Fill in the font character map if we have one.
			if (ui_vector_font_.m_pFontMap)
			{
				const auto cp_index = static_cast<unsigned char>(glyph.cp_code_point);
				ui_vector_font_.m_pFontMap[cp_index] = static_cast<uint8>(glyph_index);
			}

			const auto font_table_index = 3 * glyph_index;

			// Char width.
			ui_vector_font_.m_pFontTable[font_table_index + 0] = static_cast<uint16>(nCharWidthWithSpacing);

			// X Offset.
			ui_vector_font_.m_pFontTable[font_table_index + 1] = static_cast<uint16>(sizeOffset.x);

			// Y Offset.
			ui_vector_font_.m_pFontTable[font_table_index + 2] = static_cast<uint16>(sizeOffset.y);

			const auto is_ft_glyph_loaded = (
				glyph.code_point != 0 ?
				ltjs_load_ft_glyph_by_code_point(ft_face, glyph.code_point) :
				ltjs_load_default_ft_glyph(ft_face)
			);

			if (is_ft_glyph_loaded)
			{
				// Find pointer to region within the pixel data to copy the glyph
				// and copy the glyph into the pixeldata.
				const auto pPixelDataRegion = pPixelData + (sizeOffset.y * nPixelDataPitch) + (sizeOffset.x * 2);
				CopyGlyphBitmapToPixelData(textMetric, ft_face->glyph, pPixelDataRegion, sizeTexture.cx);
			}

			// Update to the next offset for the next character.
			sizeOffset.x += nCharWidthWithSpacing;
		}



		// See if we should write out this bitmap.
		auto bWriteFontBitmap = false;
		auto hConVar = ilt_client->GetConsoleVar("WriteFontBitmap");
		if (hConVar)
		{
			bWriteFontBitmap = (static_cast<int>(ilt_client->GetVarValueFloat(hConVar)) != 0);
		}

		// See if we should write green dots for inputing a bitmap.
		auto bGreenDot = false;
		auto hConVar2 = ilt_client->GetConsoleVar("GreenDot");
		if (hConVar2)
		{
			bGreenDot = (static_cast<int>(ilt_client->GetVarValueFloat(hConVar2)) != 0);
		}

		// See if we should write out a TGA font
		auto bWriteFontTGA = false;
		auto hConVar3 = ilt_client->GetConsoleVar("WriteFontTGA");
		if (hConVar3)
		{
			bWriteFontTGA = (static_cast<int>(ilt_client->GetVarValueFloat(hConVar3)) != 0);
		}

		// Do they want to see green dots ?
		if ((bWriteFontBitmap || bWriteFontTGA) && bGreenDot)
		{
			// Iterate over the characters.
			for (const auto& glyph : glyphs)
			{
				// Get this character's width.
				const auto glyph_index = glyph.index;

				// Fill in the font character map if we have one.
				if (ui_vector_font_.m_pFontMap)
				{
					ui_vector_font_.m_pFontMap[glyph.code_point] = static_cast<uint8>(glyph_index);
				}

				const auto font_table_index = 3 * glyph_index;

				// Char width.
				const auto w = static_cast<uint32>(ui_vector_font_.m_pFontTable[font_table_index + 0]);

				// X Offset.
				auto x = static_cast<uint32>(ui_vector_font_.m_pFontTable[font_table_index + 1]);

				// Y Offset.
				auto y = static_cast<uint32>(ui_vector_font_.m_pFontTable[font_table_index + 2]);

				y += sizeMaxGlyphSize.cy + kCharSpacing - 1;
				x += w - 1;

				const auto pPixel = pPixelData + (y * nPixelDataPitch) + (x * 2);

				auto pData2 = reinterpret_cast<uint16*>(pPixel);
				auto pPixelDataEnd2 = reinterpret_cast<uint16*>(pPixelData + nPixelDataSize);

				if (pData2 < pPixelDataEnd2)
				{
					*pData2 = 0x00F0; // Set green pixel ( pixel format is ARGB ) 
				}
			}
		}

		// Write BMP
		if (bWriteFontBitmap)
		{
			const auto file_name = std::string{} + "Font_" + ft_face->family_name + ".bmp";
			WritePixelDataBitmap(pPixelData, sizeTexture, file_name.c_str());
		}

		// Write TGA
		if (bWriteFontTGA)
		{
			const auto file_name = std::string{} + "Font_" + ft_face->family_name + ".tga";
			WritePixelDataTGA(pPixelData, sizeTexture, file_name.c_str());
		}
	}  // end if bOk

	if (bOk)
	{
		// turn pixeldata into a texture
		pTexInterface->CreateTextureFromData(
			ui_vector_font_.m_Texture,
			TEXTURETYPE_ARGB4444,
			TEXTUREFLAG_PREFER16BIT | TEXTUREFLAG_PREFER4444,
			pPixelData,
			sizeTexture.cx,
			sizeTexture.cy);

		if (!ui_vector_font_.m_Texture)
		{
			DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable:  Couldn't create texture."));
			bOk = false;
		}
	}

	// Don't need pixel data any more.
	delete[] pPixelData;
	pPixelData = nullptr;

	// Clean up if we had an error.
	if (!bOk)
	{
		delete[] ui_vector_font_.m_pFontTable;
		ui_vector_font_.m_pFontTable = nullptr;

		delete[] ui_vector_font_.m_pFontMap;
		ui_vector_font_.m_pFontMap = nullptr;

		if (ui_vector_font_.m_Texture)
		{
			pTexInterface->ReleaseTextureHandle(ui_vector_font_.m_Texture);
			ui_vector_font_.m_Texture = nullptr;
		}
	}

	return bOk;
}

CpCodePoints CUIVectorFont::Detail::make_cp_code_points(
	const char* chars)
{
	using CpCodePointSet = std::unordered_set<char>;
	auto cp_code_point_set = CpCodePointSet{};
	cp_code_point_set.reserve(256);

	while ((*chars) != '\0')
	{
		const auto cp_char = (*chars++);
		const auto cp_code_point = static_cast<unsigned char>(cp_char);

		if (cp_code_point >= 0x21 && cp_code_point <= 0xFF)
		{
			cp_code_point_set.insert(cp_char);
		}
	}


	auto cp_code_points = CpCodePoints{};
	cp_code_points.reserve(cp_code_point_set.size());
	cp_code_points.insert(cp_code_points.end(), cp_code_point_set.cbegin(), cp_code_point_set.cend());

	std::sort(
		cp_code_points.begin(),
		cp_code_points.end(),
		[](
			char lhs,
			char rhs)
		{
			return static_cast<unsigned char>(lhs) < static_cast<unsigned char>(rhs);
		}
	);

	return cp_code_points;
}

//	--------------------------------------------------------------------------
// create a proportional bitmap font from a TrueType resource

CUIVectorFont::CUIVectorFont() = default;

CUIVectorFont::~CUIVectorFont()
{
	Term();
}

bool CUIVectorFont::Init(
	char const* pszFontFile,
	char const* pszFontFace,
	uint32 pointSize,
	uint8 asciiStart,
	uint8 asciiEnd,
	LTFontParams* fontParams)
{
	char szChars[256];

	auto i = 0;

	for (i = asciiStart; i <= asciiEnd; ++i)
	{
		szChars[i - asciiStart] = static_cast<char>(i);
	}

	szChars[i - asciiStart] = 0;

	const auto bOk = Init(pszFontFile, pszFontFace, pointSize, szChars, fontParams);

	return bOk;
}

// create a proportional font from TTF, and string
bool CUIVectorFont::Init(
	char const* pszFontFile,
	char const* pszFontFace,
	uint32 pointSize,
	char const* pszCharacters,
	LTFontParams* fontParams)
{
	// Check inputs.
	if (!pszCharacters || !pszCharacters[0] || !pszFontFace || !pszFontFace[0])
	{
		return false;
	}

	// Start fresh.
	Term();

	// set the font defaults
	m_Proportional = true;

	auto installedFontFace = InstalledFontFace{};

	if (!installedFontFace.Init(pszFontFile, pszFontFace, pointSize, fontParams))
	{
		return false;
	}

	// when the font is created, this can be
	// slightly different than pointSize
	m_PointSize = pointSize;

	// Create a Texture and a Font table
	auto detail = Detail{*this};
	const auto bOk = detail.CreateFontTextureAndTable(installedFontFace, pszCharacters, true);

	if (!bOk)
	{
		return false;
	}

	// font is valid
	m_Valid = true;
	return true;
}

void CUIVectorFont::Term()
{
	// free any used resources
	if (m_bAllocatedTable && m_pFontTable)
	{
		delete[] m_pFontTable;
		m_pFontTable = nullptr;
	}

	if (m_bAllocatedMap && m_pFontMap)
	{
		delete[] m_pFontMap;
		m_pFontMap = nullptr;
	}

	// release the HTEXTURE
	if (m_Texture)
	{
		pTexInterface->ReleaseTextureHandle(m_Texture);
		m_Texture = nullptr;
	}

	m_CharTexWidth = 0;
	m_CharTexHeight = 0;

	// font is no longer valid
	m_Valid = false;
}


#endif // LTJS_SDL_BACKEND
