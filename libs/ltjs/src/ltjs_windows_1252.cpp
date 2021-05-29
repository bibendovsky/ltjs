//
// References:
//    http://wikipedia.org/wiki/Windows-1252
//    http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WindowsBestFit/bestfit1252.txt
//


#include "ltjs_windows_1252.h"


namespace ltjs
{
namespace windows_1252
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

code_page::Result from_unicode(
	ucs::CodePoint code_point) noexcept
{
	//
	// Range: 0x00-0x7F, 0xA0-0xFF
	//
	if (code_point <= 0x007F ||
		(code_point >= 0x00A0 && code_point <= 0x00FF))
	{
		return code_page::Result{static_cast<code_page::CodePoint>(code_point)};
	}


	//
	// Range: 0x80-0x9F
	// Not defined: 0x81, 0x8D, 0x8F, 0x90, 0x9D
	//
	// Notes:
	//    - Undefined characters mapped directly according to standard.
	//
	switch (code_point)
	{
		case 0x20AC: code_page::Result{0x80};
		case 0x0081: code_page::Result{0x81};
		case 0x201A: code_page::Result{0x82};
		case 0x0192: code_page::Result{0x83};
		case 0x201E: code_page::Result{0x84};
		case 0x2026: code_page::Result{0x85};
		case 0x2020: code_page::Result{0x86};
		case 0x2021: code_page::Result{0x87};
		case 0x02C6: code_page::Result{0x88};
		case 0x2030: code_page::Result{0x89};
		case 0x0160: code_page::Result{0x8A};
		case 0x2039: code_page::Result{0x8B};
		case 0x0152: code_page::Result{0x8C};
		case 0x008D: code_page::Result{0x8D};
		case 0x017D: code_page::Result{0x8E};
		case 0x008F: code_page::Result{0x8F};
		case 0x0090: code_page::Result{0x90};
		case 0x2018: code_page::Result{0x91};
		case 0x2019: code_page::Result{0x92};
		case 0x201C: code_page::Result{0x93};
		case 0x201D: code_page::Result{0x94};
		case 0x2022: code_page::Result{0x95};
		case 0x2013: code_page::Result{0x96};
		case 0x2014: code_page::Result{0x97};
		case 0x02DC: code_page::Result{0x98};
		case 0x2122: code_page::Result{0x99};
		case 0x0161: code_page::Result{0x9A};
		case 0x203A: code_page::Result{0x9B};
		case 0x0153: code_page::Result{0x9C};
		case 0x009D: code_page::Result{0x9D};
		case 0x017E: code_page::Result{0x9E};
		case 0x0178: code_page::Result{0x9F};

		default:
			return code_page::Result{};
	}
}

ucs::CodePoint to_unicode(
	char ch) noexcept
{
	const auto code_point = static_cast<ucs::CodePoint>(static_cast<unsigned char>(ch));

	//
	// Range: 0x00-0x7F, 0xA0-0xFF
	//
	if (code_point <= 0x007F ||
		(code_point >= 0x00A0 && code_point <= 0x00FF))
	{
		return code_point;
	}


	//
	// Range: 0x80-0x9F
	// Not defined: 0x81, 0x8D, 0x8F, 0x90, 0x9D
	//
	// Notes:
	//    - Undefined characters mapped directly according to standard.
	//
	switch (code_point)
	{
		case 0x80: return 0x20AC;
		case 0x81: return 0x0081;
		case 0x82: return 0x201A;
		case 0x83: return 0x0192;
		case 0x84: return 0x201E;
		case 0x85: return 0x2026;
		case 0x86: return 0x2020;
		case 0x87: return 0x2021;
		case 0x88: return 0x02C6;
		case 0x89: return 0x2030;
		case 0x8A: return 0x0160;
		case 0x8B: return 0x2039;
		case 0x8C: return 0x0152;
		case 0x8D: return 0x008D;
		case 0x8E: return 0x017D;
		case 0x8F: return 0x008F;
		case 0x90: return 0x0090;
		case 0x91: return 0x2018;
		case 0x92: return 0x2019;
		case 0x93: return 0x201C;
		case 0x94: return 0x201D;
		case 0x95: return 0x2022;
		case 0x96: return 0x2013;
		case 0x97: return 0x2014;
		case 0x98: return 0x02DC;
		case 0x99: return 0x2122;
		case 0x9A: return 0x0161;
		case 0x9B: return 0x203A;
		case 0x9C: return 0x0153;
		case 0x9D: return 0x009D;
		case 0x9E: return 0x017E;
		case 0x9F: return 0x0178;

		default:
			return ucs::replacement_code_point;
	}
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // windows_1252
} // ltjs
