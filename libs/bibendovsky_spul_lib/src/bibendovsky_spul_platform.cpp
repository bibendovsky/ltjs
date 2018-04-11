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


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_platform.h"
