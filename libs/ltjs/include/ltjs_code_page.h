/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Code page utility

#ifndef LTJS_CODE_PAGE_INCLUDED
#define LTJS_CODE_PAGE_INCLUDED

namespace ltjs {

/*
 * Maps Unicode code point to Windows-1252 one.
 *
 * Returns mapped Windows-1252 code point, or -1 otherwise.
*/
int windows_1252_from_unicode(int code_point) noexcept;
/*
 * Maps Windows-1252 code point to Unicode one.
 *
 * Returns mapped Unicode code point, or SUBSTITUTE (U+FFFD) otherwise.
 */
int windows_1252_to_unicode(char ch) noexcept;

} // namespace ltjs

#endif // LTJS_CODE_PAGE_INCLUDED
