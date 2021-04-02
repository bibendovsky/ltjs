#ifndef LTJS_EXCEPTION_INCLUDED
#define LTJS_EXCEPTION_INCLUDED


#include <exception>
#include <memory>


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class Exception :
	public std::exception
{
public:
	explicit Exception(
		const char* message);

	Exception(
		const char* context,
		const char* message);

	Exception(
		const Exception& rhs);

	const char* what() const noexcept override;


private:
	using What = std::unique_ptr<char[]>;


	What what_{};
}; // Exception

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SafeException :
	public std::exception
{
public:
	explicit SafeException(
		const char* message) noexcept;

	const char* what() const noexcept override;


private:
	const char* message_{};
}; // SafeException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_EXCEPTION_INCLUDED
