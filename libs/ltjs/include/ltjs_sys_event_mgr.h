/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Event manager

#ifndef LTJS_SYS_EVENT_MGR_INCLUDED
#define LTJS_SYS_EVENT_MGR_INCLUDED

#include <memory>
#include "ltjs_sys_event_handler_mgr.h"

namespace ltjs::sys {

class EventMgr
{
public:
	EventMgr() = default;
	virtual ~EventMgr() = default;

	virtual EventHandlerMgr* get_handler_mgr() const noexcept = 0;
	virtual void poll_events() = 0;
	virtual void handle_events() = 0;
	virtual bool was_quit_event() const noexcept = 0;
	virtual void post_quit_event() = 0;
};

// =====================================

using EventMgrUPtr = std::unique_ptr<EventMgr>;

EventMgrUPtr make_event_mgr();

} // namespace ltjs::sys

#endif // LTJS_SYS_EVENT_MGR_INCLUDED
