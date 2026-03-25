/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Shell string formatter

#include "ltjs_shell_string_formatter.h"
#include <cassert>
#include <algorithm>
#include <charconv>
#include <memory>
#include <string_view>

namespace ltjs {

ShellStringFormatter::Arg::Arg()
	:
	type{ArgType::none}
{}

void ShellStringFormatter::Arg::set(int value) noexcept
{
	type = ArgType::integer;
	int_value = value;
	size = 0;
}

void ShellStringFormatter::Arg::set(const char* value) noexcept
{
	type = ArgType::c_string;
	cstr_value = value;
	if (cstr_value != nullptr)
	{
		size = static_cast<int>(std::string_view(cstr_value).size());
	}
	else
	{
		size = 0;
	}
}

void ShellStringFormatter::collect_args([[maybe_unused]] int level) noexcept
{}

void ShellStringFormatter::format(const char* src_string, int src_size, char* dst_string, int dst_size)
{
	assert(src_string != nullptr);
	assert(src_size >= 0);
	assert(dst_string != nullptr);
	assert(dst_size >= 0);
	if (dst_size == 0)
	{
		return;
	}
	dst_size -= 1;
	const bool has_args = std::any_of(
		args_.cbegin(),
		args_.cend(),
		[](const Arg& format_arg)
		{
			return format_arg.type != ArgType::none;
		});
	if (!has_args)
	{
		const int copy_size = std::min(src_size, dst_size);
		std::copy_n(src_string, copy_size, dst_string);
		dst_string[copy_size] = '\0';
		return;
	}
	int src_offset = 0;
	int dst_offset = 0;
	const auto peek_src = [src_string, src_size, &src_offset](int offset)
	{
		const int new_src_offset = src_offset + offset;
		if (new_src_offset < 0 || new_src_offset >= src_size)
		{
			return '\0';
		}
		return src_string[new_src_offset];
	};
	for (;;)
	{
		if (src_offset >= src_size || dst_offset >= dst_size)
		{
			break;
		}
		const char ch = peek_src(0);
		bool is_insert_sequence = false;
		if (ch == '%')
		{
			const char ch_1 = peek_src(1);
			const char ch_2 = peek_src(2);
			const char ch_3 = peek_src(3);
			const char ch_4 = peek_src(4);
			if (ch_1 == '%')
			{
				src_offset += 2;
				dst_string[dst_offset++] = '%';
				is_insert_sequence = true;
			}
			else if (
				ch_1 >= min_arg_char &&
				ch_1 <= max_arg_char &&
				ch_2 == '!' &&
				(ch_3 == 'd' || ch_3 == 's') &&
				ch_4 == '!')
			{
				const int arg_index = ch_1 - '1';
				const Arg& format_arg = args_[arg_index];
				bool is_inserted = false;
				if (format_arg.type == ArgType::integer)
				{
					char* const dst_first = dst_string + dst_offset;
					if (const auto [dst_end, ec] = std::to_chars(dst_first, dst_string + dst_size, format_arg.int_value);
						ec == std::errc{})
					{
						dst_offset += static_cast<int>(dst_end - dst_first);
					}
					is_inserted = true;
				}
				else if (format_arg.type == ArgType::c_string)
				{
					const int remain_dst_size = dst_size - dst_offset;
					const int copy_size = std::min(remain_dst_size, format_arg.size);
					std::copy_n(format_arg.cstr_value, copy_size, dst_string + dst_offset);
					dst_offset += copy_size;
					is_inserted = true;
				}
				if (is_inserted)
				{
					src_offset += 5;
					is_insert_sequence = true;
				}
			}
			else
			{
				is_insert_sequence = true;
			}
		}
		if (!is_insert_sequence)
		{
			dst_string[dst_offset++] = src_string[src_offset++];
		}
	}
	dst_string[dst_offset] = '\0';
}

} // namespace ltjs
