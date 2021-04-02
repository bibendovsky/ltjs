#ifndef LTJS_SYSTEM_EVENT_QUEUE_INCLUDED
#define LTJS_SYSTEM_EVENT_QUEUE_INCLUDED


#include "SDL.h"

#include "ltjs_circular_queue.h"
#include "ltjs_system_event.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SystemEventQueue = CircularQueue<SystemEvent>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SYSTEM_EVENT_QUEUE_INCLUDED
