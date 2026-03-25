/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Exception utility

#include "ltjs_exception.h"

namespace ltjs {

Exception::Exception(std::string_view message)
	:
	Exception(std::string_view(), message)
{}

Exception::Exception(std::string_view context, std::string_view message)
{
	constexpr std::string_view context_prefix = "[";
	constexpr std::string_view context_suffix = "] ";
	const std::size_t what_size =
		(context.empty() ? 0 : context_prefix.size() + context.size() + context_suffix.size()) +
		message.size() +
		1;
	what_.reserve(what_size);
	if (!context.empty())
	{
		what_ += context_prefix;
		what_ += context;
		what_ += context_suffix;
	}
	what_ += message;
}

const char* Exception::what() const noexcept
{
	return what_.data();
}

} // namespace ltjs
