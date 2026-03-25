/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Code page utility

/*
 * References:
 *   - http://wikipedia.org/wiki/Windows-1252
 *   - http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WindowsBestFit/bestfit1252.txt
 */

#include "ltjs_code_page.h"

namespace ltjs {

int windows_1252_from_unicode(int code_point) noexcept
{
	/*
	 * Range: 0x00-0x7F, 0x81, 0x8D, 0x8F, 0x90, 0x9D, 0xA0-0xFF
	 */
	if (code_point <= 0x007F ||
		code_point == 0x81 || // Unused in the standard but mapped to C1 control code to fit best.
		code_point == 0x8D || // Unused in the standard but mapped to C1 control code to fit best.
		code_point == 0x8F || // Unused in the standard but mapped to C1 control code to fit best.
		code_point == 0x90 || // Unused in the standard but mapped to C1 control code to fit best.
		code_point == 0x9D || // Unused in the standard but mapped to C1 control code to fit best.
		(code_point >= 0x00A0 && code_point <= 0x00FF))
	{
		return code_point;
	}
	/*
	 * Range: 0x80-0x9F
	 */
	switch (code_point)
	{
		case 0x20AC: return 0x80;
		case 0x201A: return 0x82;
		case 0x0192: return 0x83;
		case 0x201E: return 0x84;
		case 0x2026: return 0x85;
		case 0x2020: return 0x86;
		case 0x2021: return 0x87;
		case 0x02C6: return 0x88;
		case 0x2030: return 0x89;
		case 0x0160: return 0x8A;
		case 0x2039: return 0x8B;
		case 0x0152: return 0x8C;
		case 0x017D: return 0x8E;
		case 0x2018: return 0x91;
		case 0x2019: return 0x92;
		case 0x201C: return 0x93;
		case 0x201D: return 0x94;
		case 0x2022: return 0x95;
		case 0x2013: return 0x96;
		case 0x2014: return 0x97;
		case 0x02DC: return 0x98;
		case 0x2122: return 0x99;
		case 0x0161: return 0x9A;
		case 0x203A: return 0x9B;
		case 0x0153: return 0x9C;
		case 0x017E: return 0x9E;
		case 0x0178: return 0x9F;
		default:     return -1; // No mapping.
	}
}

int windows_1252_to_unicode(char ch) noexcept
{
	constexpr int substitute_code_point = 0xFFFD;
	const int code_point = static_cast<unsigned char>(ch);
	/*
	 * Range: 0x00-0x7F, 0x81, 0x8D, 0x8F, 0x90, 0x9D, 0xA0-0xFF
	 */
	if (code_point <= 0x007F ||
		code_point == 0x81 || // Unused in the standard but mapped to C1 control code to fit best.
		code_point == 0x8D || // Unused in the standard but mapped to C1 control code to fit best.
		code_point == 0x8F || // Unused in the standard but mapped to C1 control code to fit best.
		code_point == 0x90 || // Unused in the standard but mapped to C1 control code to fit best.
		code_point == 0x9D || // Unused in the standard but mapped to C1 control code to fit best.
		(code_point >= 0x00A0 && code_point <= 0x00FF))
	{
		return code_point;
	}
	/*
	 * Range: 0x80-0x9F
	 */
	switch (code_point)
	{
		case 0x80: return 0x20AC;
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
		case 0x8E: return 0x017D;
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
		case 0x9E: return 0x017E;
		case 0x9F: return 0x0178;
		default:   return substitute_code_point; // No mapping.
	}
}

} // namespace ltjs
