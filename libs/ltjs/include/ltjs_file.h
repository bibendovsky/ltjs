#ifndef LTJS_FILE_INCLUDED
#define LTJS_FILE_INCLUDED


#include <cstdint>

#include "ltjs_index_type.h"


namespace ltjs
{
namespace file
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Index get_size(
	const char* path);

Index load(
	const char* path,
	void* buffer,
	Index buffer_size) noexcept;

bool save(
	const char* path,
	const void* buffer,
	Index buffer_size) noexcept;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // file
} // ltjs


#endif // !LTJS_FILE_INCLUDED
