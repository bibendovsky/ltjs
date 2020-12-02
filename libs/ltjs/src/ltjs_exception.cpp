#include "ltjs_exception.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SafeException::SafeException() noexcept
	:
	message_{}
{
}

SafeException::SafeException(
	const char* message) noexcept
	:
	message_{message}
{
}

const char* SafeException::what() const noexcept
{
	return message_;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
