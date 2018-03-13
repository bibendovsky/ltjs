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
// Path utils.
//
// Notes:
//    - All string parameters are expected in UTF-8 encoding.
//    - It's assumed there are two allowed path separators: slash and backslash.
//


#ifndef BIBENDOVSKY_SPUL_PATH_UTILS_INCLUDED
#define BIBENDOVSKY_SPUL_PATH_UTILS_INCLUDED


#include <string>


namespace bibendovsky
{
namespace spul
{


//
// Path utils.
//
class PathUtils
{
public:
	//
	// Gets a path separator for the current platform.
	//
	// Returns:
	//    - A path separator.
	//
	static char get_native_separator();


	//
	// Checks if the character is a path separator.
	//
	// Parameters:
	//    - c - a character to check.
	//
	// Returns:
	//    - "true" if character is a path separator.
	//    - "false" otherwise.
	//
	static bool is_separator(
		const char c);


	//
	// Checks if a path ends with a path separator.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - "true" if a path ends with a separator.
	//    - "false" otherwise.
	//
	static bool is_ends_with_separator(
		const char* const path_utf8);

	//
	// Checks if a path ends with a path separator.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - "true" if a path ends with a separator.
	//    - "false" otherwise.
	//
	static bool is_ends_with_separator(
		const std::string& path_utf8);


	//
	// Normalizes separator characters inplace according to the platform.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	static void normalize_i(
		char* const path_utf8);

	//
	// Normalizes separator characters inplace according to the platform.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	static void normalize_i(
		std::string& path_utf8);

	//
	// Normalizes separator characters according to the platform.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - A normalized path.
	//
	static std::string normalize(
		const char* const path_utf8);

	//
	// Normalizes separator characters according to the platform.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - A normalized path.
	//
	static std::string normalize(
		const std::string& path_utf8);


	//
	// Appends a path part inplace to the provided path.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//    - part_utf8 - a part of path to append.
	//
	static void append_i(
		std::string& path_utf8,
		const char* const part_utf8);

	//
	// Appends a path part inplace to the provided path.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//    - part_utf8 - a part of path to append.
	//
	static void append_i(
		std::string& path_utf8,
		const std::string& part_utf8);

	//
	// Appends a path part to the provided path.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//    - part_utf8 - a part of path to append.
	//
	// Returns:
	//    - A new path.
	//
	static std::string append(
		const std::string& path_utf8,
		const char* const part_utf8);

	//
	// Appends a path part to the provided path.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//    - part_utf8 - a part of path to append.
	//
	// Returns:
	//    - A new path.
	//
	static std::string append(
		const std::string& path_utf8,
		const std::string& part_utf8);


	//
	// Extracts a parent path.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - A parent path.
	//    - Empty string.
	//
	// Notes:
	//    - Does not preserve the trailing path separator.
	//
	static std::string get_parent_path(
		const char* const path_utf8);

	//
	// Extracts a parent path.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - A parent path.
	//    - Empty string.
	//
	// Notes:
	//    - Does not preserve the trailing path separator.
	//
	static std::string get_parent_path(
		const std::string& path_utf8);

	//
	// Extracts a file name.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - A file name.
	//    - Null if there is no file name.
	//
	static const char* get_file_name(
		const char* const path_utf8);

	//
	// Extracts a file name.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - A file name.
	//    - Empty string if there is no file name.
	//
	static std::string get_file_name(
		const std::string& path_utf8);

	//
	// Extracts a file extension.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - A file extension.
	//    - Null if there is no file extension.
	//
	static const char* get_file_extension(
		const char* const path_utf8);

	//
	// Extracts a file extension.
	//
	// Parameters:
	//    - path_utf8 - a path.
	//
	// Returns:
	//    - A file extension.
	//    - Empty string if there is no file extension.
	//
	static std::string get_file_extension(
		const std::string& path_utf8);


private:
	PathUtils() = delete;
}; // PathUtils


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_PATH_UTILS_INCLUDED
