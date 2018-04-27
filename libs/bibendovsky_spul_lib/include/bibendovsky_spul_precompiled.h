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
// Precompiled headers.
//


#ifndef BIBENDOVSKY_SPUL_PRECOMPILED_INCLUDED
#define BIBENDOVSKY_SPUL_PRECOMPILED_INCLUDED


#ifdef BIBENDOVSKY_SPUL_USE_PCH


#include "bibendovsky_spul_platform.h"


#ifdef BIBENDOVSKY_SPUL_IS_POSIX
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif // BIBENDOVSKY_SPUL_IS_POSIX

#include <cstddef>
#include <cstdint>
#include <array>
#include <algorithm>
#include <codecvt>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef BIBENDOVSKY_SPUL_IS_WIN32
#include <Windows.h>
#endif // BIBENDOVSKY_SPUL_IS_WIN32

#include "bibendovsky_spul_algorithm.h"
#include "bibendovsky_spul_ascii_utils.h"
#include "bibendovsky_spul_encoding_utils.h"
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_enum_flags.h"
#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_four_cc.h"
#include "bibendovsky_spul_memory_stream.h"
#include "bibendovsky_spul_path_utils.h"
#include "bibendovsky_spul_riff_four_ccs.h"
#include "bibendovsky_spul_riff_reader.h"
#include "bibendovsky_spul_scope_guard.h"
#include "bibendovsky_spul_stream.h"
#include "bibendovsky_spul_substream.h"
#include "bibendovsky_spul_utf8_utils.h"
#include "bibendovsky_spul_uuid.h"
#include "bibendovsky_spul_wave_format.h"
#include "bibendovsky_spul_wave_format_tag.h"
#include "bibendovsky_spul_wave_format_utils.h"
#include "bibendovsky_spul_wave_four_ccs.h"
#include "bibendovsky_spul_wchar_utils.h"


#endif // BIBENDOVSKY_SPUL_USE_PCH


#endif // !BIBENDOVSKY_SPUL_PRECOMPILED_INCLUDED
