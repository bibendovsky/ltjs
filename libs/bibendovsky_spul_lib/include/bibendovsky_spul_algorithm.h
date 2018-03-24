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
// Miscellaneous algorithms.
//


#ifndef BIBENDOVSKY_SPUL_ALGORITHM_INCLUDED
#define BIBENDOVSKY_SPUL_ALGORITHM_INCLUDED


#include <algorithm>


namespace bibendovsky
{
namespace spul
{


class Algorithm
{
public:
	//
	// Clams a value between the specified ones.
	//
	// Parameters:
	//    - value - a value to modify.
	//    - min_value - a minimum allowed value.
	//    - max_value - a maximum allowed value.
	//
	// Returns:
	//    A clamped value.
	//
	template<typename T>
	static constexpr T clamp(
		const T& value,
		const T& min_value,
		const T& max_value)
	{
		return std::min(max_value, std::max(min_value, value));
	}


private:
	Algorithm() = delete;
}; // Algorithm


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_ALGORITHM_INCLUDED
