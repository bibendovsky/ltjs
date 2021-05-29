#ifndef LTJS_SHELL_STRING_FORMATTER_INCLUDED
#define LTJS_SHELL_STRING_FORMATTER_INCLUDED


#include <array>
#include <utility>

#include "ltjs_index_type.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ShellStringFormatter
{
public:
	template<
		typename ...TArgs
	>
	explicit ShellStringFormatter(
		TArgs... args)
	{
		collect_args(0, std::forward<TArgs>(args)...);
	}


	void format(
		const char* src_string,
		ltjs::Index src_size,
		char* dst_string,
		ltjs::Index dst_size);


private:
	static constexpr auto max_args = 5;
	static_assert(max_args > 0 && max_args <= 9, "Max args out of range.");

	static constexpr auto min_arg_char = '1';
	static constexpr auto max_arg_char = static_cast<char>('0' + max_args);


	enum class ArgType
	{
		none,

		integer,
		c_string,
	}; // ArgType

	struct Arg
	{
		ArgType type{};

		union
		{
			const char* cstr_value{};
			int int_value;
		};

		ltjs::Index size{};


		void set(
			int value) noexcept;

		void set(
			const char* value) noexcept;
	}; // Arg

	using Args = std::array<Arg, max_args>;

	Args args_{};


	void collect_args(
		int level) noexcept;

	template<
		typename TArgN,
		typename ...TArgs
	>
	void collect_args(
		int level,
		TArgN&& arg_1,
		TArgs... args) noexcept
	{
		if (level >= max_args)
		{
			return;
		}

		args_[level].set(arg_1);

		collect_args(level + 1, std::forward<TArgs>(args)...);
	}
}; // ShellStringFormatter

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SHELL_STRING_FORMATTER_INCLUDED
