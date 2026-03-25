/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Shell string formatter

#ifndef LTJS_SHELL_STRING_FORMATTER_INCLUDED
#define LTJS_SHELL_STRING_FORMATTER_INCLUDED

#include <array>
#include <utility>

namespace ltjs {

class ShellStringFormatter
{
public:
	template<typename ...TArgs>
	explicit ShellStringFormatter(TArgs... args)
	{
		collect_args(0, std::forward<TArgs>(args)...);
	}

	void format(const char* src_string, int src_size, char* dst_string, int dst_size);

private:
	static constexpr int max_args = 5;
	static_assert(max_args > 0 && max_args <= 9, "Max args out of range.");

	static constexpr char min_arg_char = '1';
	static constexpr char max_arg_char = static_cast<char>('0' + max_args);

	enum class ArgType
	{
		none,

		integer,
		c_string
	};

	class Arg
	{
	public:
		ArgType type;
		union
		{
			const char* cstr_value{};
			int int_value;
		};
		int size;

		Arg();
		void set(int value) noexcept;
		void set(const char* value) noexcept;
	};

	using Args = std::array<Arg, max_args>;

	Args args_{};

	void collect_args(int level) noexcept;

	template<typename TArgN, typename ...TArgs>
	void collect_args(int level, TArgN&& arg_1, TArgs... args) noexcept
	{
		if (level < max_args)
		{
			args_[level].set(arg_1);
			collect_args(level + 1, std::forward<TArgs>(args)...);
		}
	}
};

} // namespace ltjs

#endif // LTJS_SHELL_STRING_FORMATTER_INCLUDED
