/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Language resource info

#ifndef LTJS_LANGUAGE_INCLUDED
#define LTJS_LANGUAGE_INCLUDED

#include <string_view>

namespace ltjs {

struct Language
{
	using IdString = std::string_view;
	using Name = std::string_view;

	IdString id_string;
	Name name;
};

} // namespace ltjs

#endif // LTJS_LANGUAGE_INCLUDED
