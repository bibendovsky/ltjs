/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Event handler manager

#ifndef LTJS_SYS_EVENT_HANDLER_MGR_INCLUDED
#define LTJS_SYS_EVENT_HANDLER_MGR_INCLUDED

#include <memory>
#include "ltjs_sys_event_handler.h"

namespace ltjs::sys {

enum class EventHandlerPriority
{
	normal,
	high
};

// =====================================

class EventHandlerMgrIterator
{
public:
	explicit EventHandlerMgrIterator(EventHandler* const* handlers) noexcept;

	EventHandler* operator*() const;
	void operator++();
	bool operator==(const EventHandlerMgrIterator& rhs) const noexcept;
	bool operator!=(const EventHandlerMgrIterator& rhs) const noexcept;

private:
	EventHandler* const* handlers_{};
};

// =====================================

// The order of elements: from new to old.
class EventHandlerMgr
{
public:
	EventHandlerMgr() = default;
	virtual ~EventHandlerMgr() = default;

	virtual bool is_empty() const noexcept = 0;
	virtual int get_size() const noexcept = 0;
	virtual EventHandler* get(int index) const = 0;
	virtual void add(EventHandler* handler, EventHandlerPriority priority) = 0;
	virtual void remove(EventHandler* handler) = 0;
	virtual EventHandlerMgrIterator begin() const = 0;
	virtual EventHandlerMgrIterator end() const = 0;
};

// =====================================

using EventHandlerMgrUPtr = std::unique_ptr<EventHandlerMgr>;

EventHandlerMgrUPtr make_event_handler_mgr();

} // namespace ltjs::sys

#endif // LTJS_SYS_EVENT_HANDLER_MGR_INCLUDED
