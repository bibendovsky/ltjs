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
// WaveFormatEx utils.
//


#ifndef BIBENDOVSKY_SPUL_WAVE_FORMAT_UTILS_INCLUDED
#define BIBENDOVSKY_SPUL_WAVE_FORMAT_UTILS_INCLUDED


#include "bibendovsky_spul_wave_format.h"
#include "bibendovsky_spul_stream.h"


namespace bibendovsky
{
namespace spul
{


//
// Utils for WaveFormat-family classes.
//
class WaveformatUtils
{
public:
	//
	// Swaps byte order of all fields.
	//
	// Parameters:
	//    - format - a format buffer.
	//
	static void endian(
		WaveFormat& format);

	//
	// Swaps byte order of all fields.
	//
	// Parameters:
	//    - format - a format buffer.
	//
	static void endian(
		PcmWaveFormat& format);

	//
	// Swaps byte order of all fields.
	//
	// Parameters:
	//    - format - a format buffer.
	//
	static void endian(
		WaveFormatEx& format);


	//
	// Reads WaveFormat structure in little-endian format from a stream.
	//
	// Parameters:
	//    - stream_ptr - a stream to read the data from.
	//    - format - a format buffer.
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	static bool read(
		StreamPtr stream_ptr,
		WaveFormat& format);

	//
	// Reads PcmWaveFormat structure in little-endian format from a stream.
	//
	// Parameters:
	//    - stream_ptr - a stream to read the data from.
	//    - format - a format buffer.
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	static bool read(
		StreamPtr stream_ptr,
		PcmWaveFormat& format);

	//
	// Reads WaveFormatEx structure in little-endian format from a stream.
	//
	// Parameters:
	//    - stream_ptr - a stream to read the data from.
	//    - format - a format buffer.
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	static bool read(
		StreamPtr stream_ptr,
		WaveFormatEx& format);


	//
	// Writes WaveFormat structure in little-endian format into a stream.
	//
	// Parameters:
	//    - format - a format to write.
	//    - stream_ptr - a stream to write the data to.
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	static bool write(
		const WaveFormat& format,
		StreamPtr stream_ptr);

	//
	// Writes PcmWaveFormat structure in little-endian format into a stream.
	//
	// Parameters:
	//    - format - a format buffer.
	//    - stream_ptr - a stream to read the data from.
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	static bool write(
		const PcmWaveFormat& format,
		StreamPtr stream_ptr);

	//
	// Writes WaveFormatEx structure in little-endian format into a stream.
	//
	// Parameters:
	//    - format - a format buffer.
	//    - stream_ptr - a stream to read the data from.
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	static bool write(
		const WaveFormatEx& format,
		StreamPtr stream_ptr);


private:
	struct Detail;


	WaveformatUtils() = delete;
}; // WaveformatUtils


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_WAVE_FORMAT_UTILS_INCLUDED
