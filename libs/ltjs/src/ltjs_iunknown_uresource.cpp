#include "ltjs_iunknown_uresource.h"

#include <cassert>


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void IUnknownUDeleter::operator()(
	::IUnknown* iunknown)
{
	assert(iunknown);

	const auto reference_count = iunknown->Release();

	assert(reference_count == 0);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
