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
// UTF-8 utils.
//


#ifndef BIBENDOVSKY_SPUL_UTF8_UTILS_INCLUDED
#define BIBENDOVSKY_SPUL_UTF8_UTILS_INCLUDED


#include <string>


namespace bibendovsky
{
namespace spul
{


class Utf8Utils
{
public:
	//
	// Makes a string lowercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	// Notes:
	//    - Sets an empty string on error.
	//
	static void to_lower_i(
		std::string& string);

	//
	// Makes a string lowercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//    - is_succeed - a result of conversion flag.
	//
	// Notes:
	//    - Sets an empty string on error.
	//
	static void to_lower_i(
		std::string& string,
		bool& is_succeed);

	//
	// Makes a string lowercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	// Returns:
	//    A string in lower case.
	//    An empty string on error.
	//
	static std::string to_lower(
		const std::string& string);

	//
	// Makes a string lowercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//    - is_succeed - a result of conversion flag.
	//
	// Returns:
	//    A string in lower case.
	//    An empty string on error.
	//
	static std::string to_lower(
		const std::string& string,
		bool& is_succeed);


	//
	// Makes a string uppercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	// Notes:
	//    - Sets an empty string on error.
	//
	static void to_upper_i(
		std::string& string);

	//
	// Makes a string uppercase inplace.
	//
	// Parameters:
	//    - string - a string to convert.
	//    - is_succeed - a result of conversion flag.
	//
	// Notes:
	//    - Sets an empty string on error.
	//
	static void to_upper_i(
		std::string& string,
		bool& is_succeed);

	//
	// Makes a string uppercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//
	// Returns:
	//    A string in upper case.
	//    An empty string on error.
	//
	static std::string to_upper(
		const std::string& string);

	//
	// Makes a string uppercase.
	//
	// Parameters:
	//    - string - a string to convert.
	//    - is_succeed - a result of conversion flag.
	//
	// Returns:
	//    A string in upper case.
	//    An empty string on error.
	//
	static std::string to_upper(
		const std::string& string,
		bool& is_succeed);


private:
	Utf8Utils() = delete;
}; // Utf8Utils


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_UTF8_UTILS_INCLUDED
