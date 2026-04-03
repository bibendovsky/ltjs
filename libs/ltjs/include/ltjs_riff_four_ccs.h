/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Known list of RIFF FourCC's

#ifndef LTJS_RIFF_FOUR_CCS_INCLUDED
#define LTJS_RIFF_FOUR_CCS_INCLUDED

#include "ltjs_four_cc.h"

namespace ltjs {

class RiffFourCcs
{
public:
	inline static constexpr FourCc riff{'R', 'I', 'F', 'F'};
	inline static constexpr FourCc list{'L', 'I', 'S', 'T'};
};

} // namespace ltjs

#endif // LTJS_RIFF_FOUR_CCS_INCLUDED
