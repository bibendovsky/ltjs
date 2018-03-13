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


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_encoding_utils.h"
#include "bibendovsky_spul_utf8_utils.h"
#include "bibendovsky_spul_wchar_utils.h"


namespace bibendovsky
{
namespace spul
{


void Utf8Utils::to_lower_i(
	std::string& string)
{
	bool is_succeed;
	to_lower_i(string, is_succeed);
}

void Utf8Utils::to_lower_i(
	std::string& string,
	bool& is_succeed)
{
	if (string.empty())
	{
		is_succeed = true;
		return;
	}

	auto wide_string = EncodingUtils::utf8_to_wide(string, is_succeed);

	if (!is_succeed)
	{
		string.clear();
		return;
	}

	WCharUtils::to_lower_i(wide_string, is_succeed);

	if (!is_succeed)
	{
		string.clear();
		return;
	}

	string = EncodingUtils::wide_to_utf8(wide_string, is_succeed);
}

std::string Utf8Utils::to_lower(
	const std::string& string)
{
	bool is_succeed;
	return to_lower(string, is_succeed);
}

std::string Utf8Utils::to_lower(
	const std::string& string,
	bool& is_succeed)
{
	if (string.empty())
	{
		is_succeed = true;
		return {};
	}

	auto wide_string = EncodingUtils::utf8_to_wide(string, is_succeed);

	if (!is_succeed)
	{
		return {};
	}

	WCharUtils::to_lower_i(wide_string, is_succeed);

	if (!is_succeed)
	{
		return {};
	}

	return EncodingUtils::wide_to_utf8(wide_string, is_succeed);
}

void Utf8Utils::to_upper_i(
	std::string& string)
{
	bool is_succeed;
	to_upper_i(string, is_succeed);
}

void Utf8Utils::to_upper_i(
	std::string& string,
	bool& is_succeed)
{
	if (string.empty())
	{
		is_succeed = true;
		return;
	}

	auto wide_string = EncodingUtils::utf8_to_wide(string, is_succeed);

	if (!is_succeed)
	{
		string.clear();
		return;
	}

	WCharUtils::to_upper_i(wide_string, is_succeed);

	if (!is_succeed)
	{
		string.clear();
		return;
	}

	string = EncodingUtils::wide_to_utf8(wide_string, is_succeed);
}

std::string Utf8Utils::to_upper(
	const std::string& string)
{
	bool is_succeed;
	return to_upper(string, is_succeed);
}

std::string Utf8Utils::to_upper(
	const std::string& string,
	bool& is_succeed)
{
	if (string.empty())
	{
		is_succeed = true;
		return {};
	}

	auto wide_string = EncodingUtils::utf8_to_wide(string, is_succeed);

	if (!is_succeed)
	{
		return {};
	}

	WCharUtils::to_upper_i(wide_string, is_succeed);

	if (!is_succeed)
	{
		return {};
	}

	return EncodingUtils::wide_to_utf8(wide_string, is_succeed);
}


} // spul
} // bibendovsky
