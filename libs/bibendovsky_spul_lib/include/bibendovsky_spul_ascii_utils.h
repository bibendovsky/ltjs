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
// ASCII (7 bit) utils.
//


#ifndef BIBENDOVSKY_SPUL_ASCII_UTILS_INCLUDED
#define BIBENDOVSKY_SPUL_ASCII_UTILS_INCLUDED


#include <string>


namespace bibendovsky
{
namespace spul
{


class AsciiUtils
{
public:
	//
	// Makes a character lowercase inplace.
	//
	// Parameters:
	//    - c - a character to convert.
	//
	static void to_lower_ascii_i(
		char& c);

	//
	// Makes a string lowercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	static void to_lower_ascii_i(
		char* const string);

	//
	// Makes a string lowercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//    - string_length - a length of the string.
	//
	static void to_lower_ascii_i(
		char* const string,
		const int string_length);

	//
	// Makes a string lowercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	static void to_lower_ascii_i(
		std::string& string);

	//
	// Makes a character lowercase.
	//
	// Parameters:
	//    - c - a character to convert.
	//
	// Returns:
	//    A converted character.
	//
	static char to_lower_ascii(
		const char c);

	//
	// Makes a string lowercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	// Returns:
	//    A converted string.
	//
	static std::string to_lower_ascii(
		const char* const string);

	//
	// Makes a string lowercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//    - string_length - a length of the string.
	//
	// Returns:
	//    A converted string.
	//
	static std::string to_lower_ascii(
		const char* const string,
		const int string_length);

	//
	// Makes a string lowercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	// Returns:
	//    A converted string.
	//
	static std::string to_lower_ascii(
		const std::string& string);


	//
	// Makes a character uppercase inplace.
	//
	// Parameters:
	//    - c - a character to convert.
	//
	static void to_upper_ascii_i(
		char& c);

	//
	// Makes a string uppercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	static void to_upper_ascii_i(
		char* const string);

	// Makes a string uppercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//    - string_length - a length of the string.
	//
	static void to_upper_ascii_i(
		char* const string,
		const int string_length);

	//
	// Makes a string uppercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	static void to_upper_ascii_i(
		std::string& string);

	//
	// Makes a character uppercase.
	//
	// Parameters:
	//    - c - a character to convert.
	//
	// Returns:
	//    A converted character.
	//
	static char to_upper_ascii(
		const char c);

	//
	// Makes a string uppercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	// Returns:
	//    A converted string.
	//
	static std::string to_upper_ascii(
		const char* const string);

	//
	// Makes a string uppercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//    - string_length - a length of the string.
	//
	// Returns:
	//    A converted string.
	//
	static std::string to_upper_ascii(
		const char* const string,
		const int string_length);

	//
	// Makes a string uppercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	// Returns:
	//    A converted string.
	//
	static std::string to_upper_ascii(
		const std::string& string);


private:
	struct Detail;


	AsciiUtils() = delete;
}; // AsciiUtils


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_ASCII_UTILS_INCLUDED
