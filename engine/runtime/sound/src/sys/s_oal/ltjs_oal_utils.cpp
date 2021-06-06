#include "ltjs_oal_utils.h"

#include <cassert>

#include "al.h"


namespace ltjs
{
namespace oal
{


void clear_error()
{
	static_cast<void>(::alGetError());
}

void clear_error_debug()
{
#if _DEBUG
	static_cast<void>(::alGetError());
#endif // _DEBUG
}

void ensure_no_error_debug()
{
	assert(::alGetError() == AL_NO_ERROR);
}

bool is_succeed()
{
	return ::alGetError() == AL_NO_ERROR;
}


} // oal
} // ltjs
