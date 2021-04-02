#ifndef LTJS_IUNKNOWN_URESOURCE_INCLUDED
#define LTJS_IUNKNOWN_URESOURCE_INCLUDED


#include <memory>

#include <windows.h>


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct IUnknownUDeleter
{
	void operator()(
		::IUnknown* iunknown);
}; // IUnknownUDeleter

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TResource
>
using IUnknownUResource = std::unique_ptr<TResource, IUnknownUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // LTJS_IUNKNOWN_URESOURCE_INCLUDED
