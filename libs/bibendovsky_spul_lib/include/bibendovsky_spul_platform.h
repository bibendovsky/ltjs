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
// Platform specific stuff.
//
//
// Constant defines:
//    BIBENDOVSKY_SPUL_IS_WIN32 - platform is Win32.
//    BIBENDOVSKY_SPUL_IS_POSIX - platform is POSIX (Linux, etc.).
//
// Custom defines (auto-detected or user-defined):
//    BIBENDOVSKY_SPUL_BIG_ENDIAN - platform is big-endian.
//    BIBENDOVSKY_SPUL_LITTLE_ENDIAN - platform is little-endian (default, if not defined).
//
// Notes:
//    Include the header first before any other.
//


#ifndef BIBENDOVSKY_SPUL_PLATFORM_INCLUDED
#define BIBENDOVSKY_SPUL_PLATFORM_INCLUDED


#if defined(_WIN32)

#ifndef BIBENDOVSKY_SPUL_IS_WIN32
#define BIBENDOVSKY_SPUL_IS_WIN32
#endif // !BIBENDOVSKY_SPUL_IS_WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#ifndef NTDDI_VERSION
// Windows XP with SP3
#define NTDDI_VERSION 0x05010300
#endif // !NTDDI_VERSION

#ifndef _WIN32_WINNT
// Windows XP
#define _WIN32_WINNT 0x0501
#endif // !_WIN32_WINNT

#ifndef WINVER
// Windows XP
#define WINVER 0x0501
#endif // !WINVER

#endif // _WIN32


#if !defined(_WIN32)

#ifndef BIBENDOVSKY_SPUL_IS_POSIX
#define BIBENDOVSKY_SPUL_IS_POSIX
#endif // !BIBENDOVSKY_SPUL_IS_POSIX

#endif // !_WIN32


#if !defined(BIBENDOVSKY_SPUL_LITTLE_ENDIAN) && !defined(BIBENDOVSKY_SPUL_BIG_ENDIAN)
#define BIBENDOVSKY_SPUL_LITTLE_ENDIAN
#endif // !BIBENDOVSKY_SPUL_LITTLE_ENDIAN && !BIBENDOVSKY_SPUL_BIG_ENDIAN


#endif // !BIBENDOVSKY_SPUL_PLATFORM_INCLUDED
