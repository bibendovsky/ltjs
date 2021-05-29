//
// References:
//    http://wikipedia.org/wiki/Windows-1252
//    http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WindowsBestFit/bestfit1252.txt
//


#ifndef LTJS_WINDOWS_1252_INCLUDED
#define LTJS_WINDOWS_1252_INCLUDED


#include "ltjs_code_page.h"
#include "ltjs_ucs.h"


namespace ltjs
{
namespace windows_1252
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

code_page::Result from_unicode(
	ucs::CodePoint code_point) noexcept;

ucs::CodePoint to_unicode(
	char ch) noexcept;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // windows_1252
} // ltjs


#endif // !LTJS_WINDOWS_1252_INCLUDED
