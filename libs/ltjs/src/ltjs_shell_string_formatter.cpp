#include "ltjs_shell_string_formatter.h"

#include <algorithm>
#include <memory>

#include "ltjs_c_string.h"
#include "ltjs_char_conv.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void ShellStringFormatter::Arg::set(
	int value) noexcept
{
	type = ArgType::integer;
	int_value = value;
	size = 0;
}

void ShellStringFormatter::Arg::set(
	const char* value) noexcept
{
	type = ArgType::c_string;
	cstr_value = value;

	if (cstr_value)
	{
		size = ltjs::c_string::get_size(cstr_value);
	}
	else
	{
		size = 0;
	}
}

void ShellStringFormatter::collect_args(
	int level) noexcept
{
	static_cast<void>(level);
};

void ShellStringFormatter::format(
	const char* src_string,
	ltjs::Index src_size,
	char* dst_string,
	ltjs::Index dst_size)
{
	if (!dst_string || dst_size <= 0)
	{
		return;
	}

	if (!src_string)
	{
		src_size = 0;
	}

	dst_size -= 1;

	const auto has_args = std::any_of(
		args_.cbegin(),
		args_.cend(),
		[](
			const Arg& format_arg)
		{
			return format_arg.type != ArgType::none;
		}
	);

	if (!has_args)
	{
		const auto copy_size = std::min(src_size, dst_size);

		std::uninitialized_copy_n(
			src_string,
			copy_size,
			dst_string
		);

		dst_string[copy_size] = '\0';

		return;
	}

	auto src_offset = ltjs::Index{};
	auto dst_offset = ltjs::Index{};

	const auto peek_src = [src_string, src_size, &src_offset](
		ltjs::Index offset)
	{
		const auto new_src_offset = src_offset + offset;

		if (new_src_offset < 0 || new_src_offset >= src_size)
		{
			return '\0';
		}

		return src_string[new_src_offset];
	};

	while (true)
	{
		if (src_offset >= src_size || dst_offset >= dst_size)
		{
			break;
		}

		const auto ch = peek_src(0);

		auto is_insert_sequence = false;

		if (ch == '%')
		{
			const auto ch_1 = peek_src(1);
			const auto ch_2 = peek_src(2);
			const auto ch_3 = peek_src(3);
			const auto ch_4 = peek_src(4);

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
				const auto arg_index = ch_1 - '1';
				const auto& format_arg = args_[arg_index];

				auto is_inserted = false;

				if (false)
				{
				}
				else if (format_arg.type == ArgType::integer)
				{
					const auto written_size = to_chars(
						format_arg.int_value,
						10,
						to_chars_format_default,
						dst_string + dst_offset,
						dst_size - dst_offset
					);

					dst_offset += written_size;

					is_inserted = true;
				}
				else if (format_arg.type == ArgType::c_string)
				{
					const auto remain_dst_size = dst_size - dst_offset;
					const auto copy_size = std::min(remain_dst_size, format_arg.size);

					std::uninitialized_copy_n(
						format_arg.cstr_value,
						copy_size,
						dst_string + dst_offset
					);

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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
