#ifndef LTJS_SHARED_LIBRARY_INCLUDED
#define LTJS_SHARED_LIBRARY_INCLUDED


#include <memory>


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class SharedLibrary
{
public:
	SharedLibrary() noexcept;

	virtual ~SharedLibrary();


	virtual void* find_symbol(
		const char* symbol_name) noexcept = 0;


	template<
		typename T
	>
	T find_symbol(
		const char* symbol_name) noexcept
	{
		return reinterpret_cast<T>(find_symbol(symbol_name));
	}

	static const char* get_default_suffix() noexcept;
}; // SharedLibrary

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

using SharedLibraryUPtr = std::unique_ptr<SharedLibrary>;

SharedLibraryUPtr make_shared_library(
	const char* path);

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs


#endif // !LTJS_SHARED_LIBRARY_INCLUDED
