#ifndef LTJS_EXCEPTION_INCLUDED
#define LTJS_EXCEPTION_INCLUDED


#include <exception>


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SafeException :
	public std::exception
{
public:
	SafeException() noexcept;

	explicit SafeException(
		const char* message) noexcept;

	const char* what() const noexcept override;


private:
	const char* message_;
}; // SafeException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_EXCEPTION_INCLUDED
