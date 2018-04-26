/*

Source Port Utility Library

Copyright (c) 2018 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

*/


//
// Encoding utils.
//


#ifndef BIBENDOVSKY_SPUL_ENCODING_UTILS_INCLUDED
#define BIBENDOVSKY_SPUL_ENCODING_UTILS_INCLUDED


#include <string>


namespace bibendovsky
{
namespace spul
{


class EncodingUtils
{
public:
	//
	// Converts UTF-8 string to a wide string.
	//
	// Parameters:
	//    - utf8_string - UTF-8 string.
	//
	// Returns:
	//    Wide string.
	//    Empty string on error.
	//
	static std::wstring utf8_to_wide(
		const std::string& utf8_string);

	//
	// Converts UTF-8 string to a wide string.
	//
	// Parameters:
	//    - utf8_string - UTF-8 string.
	//    - is_succeed - success flag.
	//
	// Returns:
	//    Wide string.
	//    Empty string on error.
	//
	static std::wstring utf8_to_wide(
		const std::string& utf8_string,
		bool& is_succeed);

	//
	// Converts a wide string to UTF-8 string.
	//
	// Parameters:
	//    - wide_string - a wide string.
	//
	// Returns:
	//    UTF-8 string.
	//    Empty string on error.
	//
	static std::string wide_to_utf8(
		const std::wstring& wide_string);

	//
	// Converts a wide string to UTF-8 string.
	//
	// Parameters:
	//    - wide_string - a wide string.
	//    - is_succeed - success flag.
	//
	// Returns:
	//    UTF-8 string.
	//    Empty string on error.
	//
	static std::string wide_to_utf8(
		const std::wstring& wide_string,
		bool& is_succeed);


	//
	// Converts UTF-8 string to UTF-16 string.
	//
	// Parameters:
	//    - utf8_string - UTF-8 string.
	//
	// Returns:
	//    UTF-16 string.
	//    Empty string on error.
	//
	static std::u16string utf8_to_utf16(
		const std::string& utf8_string);

	//
	// Converts UTF-8 string to UTF-16 string.
	//
	// Parameters:
	//    - utf8_string - UTF-8 string.
	//    - is_succeed - success flag.
	//
	// Returns:
	//    UTF-16 string.
	//    Empty string on error.
	//
	static std::u16string utf8_to_utf16(
		const std::string& utf8_string,
		bool& is_succeed);

	//
	// Converts UTF-16 string to UTF-8 string.
	//
	// Parameters:
	//    - utf16_string - UTF-16 string.
	//
	// Returns:
	//    UTF-8 string.
	//    Empty string on error.
	//
	static std::string utf16_to_utf8(
		const std::u16string& utf16_string);

	//
	// Converts UTF-16 string to UTF-8 string.
	//
	// Parameters:
	//    - utf16_string - a wide string.
	//    - is_succeed - success flag.
	//
	// Returns:
	//    UTF-8 string.
	//    Empty string on error.
	//
	static std::string utf16_to_utf8(
		const std::u16string& utf16_string,
		bool& is_succeed);


private:
	struct Detail;


	EncodingUtils() = delete;
}; // EncodingUtils


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_ENCODING_UTILS_INCLUDED
