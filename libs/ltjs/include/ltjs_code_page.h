#ifndef LTJS_CODE_PAGE_RESULT_INCLUDED
#define LTJS_CODE_PAGE_RESULT_INCLUDED


namespace ltjs
{
namespace code_page
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using CodePoint = int;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class Result
{
public:
	Result() noexcept;

	explicit Result(
		CodePoint code_point) noexcept;


	explicit operator bool() const noexcept;

	operator char() const;


private:
	CodePoint code_point_{};
}; // Result

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // code_page
} // ltjs


#endif // !LTJS_CODE_PAGE_RESULT_INCLUDED
