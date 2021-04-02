#ifndef LTJS_SYSTEM_EVENT_MGR_INCLUDED
#define LTJS_SYSTEM_EVENT_MGR_INCLUDED


#include <memory>

#include "ltjs_system_event_handler_mgr.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemEventMgr
{
public:
	SystemEventMgr() noexcept;

	virtual ~SystemEventMgr();


	virtual SystemEventHandlerMgr* get_handler_mgr() const noexcept = 0;

	virtual void poll_events() = 0;

	virtual void handle_events() = 0;

	virtual bool was_quit_event() const noexcept = 0;

	virtual void post_quit_event() = 0;
}; // SystemEventMgr

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SystemEventMgrUPtr = std::unique_ptr<SystemEventMgr>;

SystemEventMgrUPtr make_system_event_mgr();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SYSTEM_EVENT_MGR_INCLUDED
