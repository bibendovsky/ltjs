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
// A known list of WAV format tags.
//


#ifndef BIBENDOVSKY_SPUL_WAVE_FORMAT_TAGS_INCLUDED
#define BIBENDOVSKY_SPUL_WAVE_FORMAT_TAGS_INCLUDED


#include <cstdint>


namespace bibendovsky
{
namespace spul
{


//
// A known list of WAV format tags.
//
enum class WaveFormatTag :
	std::uint16_t
{
	none = 0x0000,
	pcm = 0x0001,
	ima_adpcm = 0x0011,
	mp3 = 0x0055,
	extensible = 0xFFFF,
}; // WaveFormatTag


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_WAVE_FORMAT_TAGS_INCLUDED
