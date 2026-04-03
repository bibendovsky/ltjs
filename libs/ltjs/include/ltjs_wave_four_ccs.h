/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Known WAVE FourCC's

#ifndef LTJS_WAVE_FOUR_CCS_INCLUDED
#define LTJS_WAVE_FOUR_CCS_INCLUDED

#include "ltjs_four_cc.h"

namespace ltjs {

class WaveFourCcs
{
public:
	inline static constexpr FourCc wave{'W', 'A', 'V', 'E'};
	inline static constexpr FourCc data{'d', 'a', 't', 'a'};
	inline static constexpr FourCc fmt{'f', 'm', 't', ' '};
	inline static constexpr FourCc fact{'f', 'a', 'c', 't'};
};

} // namespace ltjs

#endif // LTJS_WAVE_FOUR_CCS_INCLUDED
