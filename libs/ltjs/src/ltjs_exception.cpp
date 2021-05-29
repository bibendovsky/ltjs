#include "ltjs_exception.h"

#include <cassert>

#include <memory>

#include "ltjs_c_string.h"


namespace ltjs
{

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Exception::Exception(
	const char* message)
	:
	Exception{nullptr, message}
{
}

Exception::Exception(
	const char* context,
	const char* message)
{
	assert(message);

	constexpr auto context_prefix = "[";
	constexpr auto context_prefix_size = c_string::get_size(context_prefix);

	constexpr auto context_suffix = "] ";
	constexpr auto context_suffix_size = c_string::get_size(context_suffix);

	const auto context_size = (context ? c_string::get_size(context) : 0);
	const auto message_size = (message ? c_string::get_size(message) : 0);

	const auto what_size =
		(
			context_size > 0 ?
			context_prefix_size + context_size + context_suffix_size :
			0
		) +
		message_size +
		0
	;

	what_ = std::make_unique<char[]>(what_size + 1);
	auto what = what_.get();

	if (context_size > 0)
	{
		what = std::uninitialized_copy_n(
			context_prefix,
			context_prefix_size,
			what
		);

		what = std::uninitialized_copy_n(
			context,
			context_size,
			what
		);

		what = std::uninitialized_copy_n(
			context_suffix,
			context_suffix_size,
			what
		);
	}

	what = std::uninitialized_copy_n(
		message,
		message_size,
		what
	);

	*what = '\0';
}

Exception::Exception(
	const Exception& rhs)
	:
	Exception{rhs.what()}
{
}

const char* Exception::what() const noexcept
{
	return what_.get();
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SafeException::SafeException(
	const char* message) noexcept
	:
	message_{message}
{
	assert(message_);
}

const char* SafeException::what() const noexcept
{
	return message_;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
