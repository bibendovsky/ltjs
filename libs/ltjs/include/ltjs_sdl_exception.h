#ifndef LTJS_SDL_EXCEPTION_INCLUDED
#define LTJS_SDL_EXCEPTION_INCLUDED


#include "ltjs_exception.h"


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class SdlException :
	public Exception
{
public:
	SdlException();


private:
	static const char* make_message();
}; // SdlException

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs


#endif // !LTJS_SDL_EXCEPTION_INCLUDED
