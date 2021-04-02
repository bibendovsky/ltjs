#ifndef LTJS_SYSTEM_EVENT_HANDLER_INCLUDED
#define LTJS_SYSTEM_EVENT_HANDLER_INCLUDED


#include "ltjs_system_event.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemEventHandler
{
public:
	SystemEventHandler() noexcept;

	virtual ~SystemEventHandler();


	virtual bool operator()(
		const SystemEvent& event) = 0;
}; // SystemEventHandler

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SYSTEM_EVENT_HANDLER_INCLUDED
