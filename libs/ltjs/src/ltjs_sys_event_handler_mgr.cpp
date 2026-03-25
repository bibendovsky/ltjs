/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Event handler manager

#include "ltjs_sys_event_handler_mgr.h"
#include "ltjs_exception.h"
#include <cassert>
#include <algorithm>
#include <array>
#include <memory>
#include <string_view>

namespace ltjs::sys {

EventHandlerMgrIterator::EventHandlerMgrIterator(EventHandler* const* handlers) noexcept
	:
	handlers_{handlers}
{
	assert(handlers_ != nullptr);
}

EventHandler* EventHandlerMgrIterator::operator*() const
{
	assert(handlers_ != nullptr);
	return *handlers_;
}

void EventHandlerMgrIterator::operator++()
{
	assert(handlers_ != nullptr);
	handlers_ += 1;
}

bool EventHandlerMgrIterator::operator==(const EventHandlerMgrIterator& rhs) const noexcept
{
	return handlers_ == rhs.handlers_;
}

bool EventHandlerMgrIterator::operator!=(const EventHandlerMgrIterator& rhs) const noexcept
{
	return !((*this) == rhs);
}

// =====================================

namespace {

class EventHandlerMgrImpl final : public EventHandlerMgr
{
public:
	EventHandlerMgrImpl() = default;

	bool is_empty() const noexcept override;
	int get_size() const noexcept override;
	EventHandler* get(int index) const override;
	void add(EventHandler* handler, EventHandlerPriority priority) override;
	void remove(EventHandler* handler) override;
	EventHandlerMgrIterator begin() const override;
	EventHandlerMgrIterator end() const override;

private:
	static constexpr int max_handlers = 16;

	using Handlers = std::array<EventHandler*, max_handlers>;

	Handlers handlers_{};
	int size_{};
	int high_priority_size_{};

	[[noreturn]] static void fail(std::string_view message);
	static void ensure_handler_not_null(EventHandler* handler);
};

// -------------------------------------

bool EventHandlerMgrImpl::is_empty() const noexcept
{
	return get_size() == 0;
}

int EventHandlerMgrImpl::get_size() const noexcept
{
	return size_;
}

EventHandler* EventHandlerMgrImpl::get(int index) const
{
	assert(index >= 0 && index < get_size());
	return handlers_[index];
}

void EventHandlerMgrImpl::add(EventHandler* handler, EventHandlerPriority priority)
{
	ensure_handler_not_null(handler);
	int is_high_priority = false;
	switch (priority)
	{
		case EventHandlerPriority::normal:
			break;
		case EventHandlerPriority::high:
			is_high_priority = true;
			break;
		default:
			fail("Unsupported priority value.");
	}
	if (get_size() >= max_handlers)
	{
		fail("Full list.");
	}
	if (!is_empty())
	{
		const auto handlers_begin_iter = handlers_.begin();
		std::move(handlers_begin_iter, handlers_begin_iter + get_size(), handlers_begin_iter + 1);
	}
	if (is_high_priority)
	{
		handlers_.front() = handler;
		high_priority_size_ += 1;
	}
	else
	{
		handlers_[high_priority_size_] = handler;
	}
	size_ += 1;
}

void EventHandlerMgrImpl::remove(EventHandler* handler)
{
	ensure_handler_not_null(handler);
	if (is_empty())
	{
		fail("Empty list.");
	}
	bool is_high_priority = false;
	const auto handlers_begin_iter = handlers_.begin();
	if (high_priority_size_ > 0)
	{
		const auto high_priority_handlers_end_iter = handlers_begin_iter + high_priority_size_;
		const auto high_priority_handler_iter = std::find(handlers_begin_iter, high_priority_handlers_end_iter, handler);
		is_high_priority = (high_priority_handler_iter != high_priority_handlers_end_iter);
	}
	const auto handlers_end_iter = handlers_begin_iter + get_size();
	const auto next_handler_iter = std::remove(handlers_begin_iter, handlers_end_iter, handler);
	if (next_handler_iter == handlers_end_iter)
	{
		fail("Handler not found.");
	}
	size_ -= 1;
	high_priority_size_ -= is_high_priority;
}

EventHandlerMgrIterator EventHandlerMgrImpl::begin() const
{
	return EventHandlerMgrIterator{handlers_.data()};
}

EventHandlerMgrIterator EventHandlerMgrImpl::end() const
{
	return EventHandlerMgrIterator{handlers_.data() + get_size()};
}

[[noreturn]] void EventHandlerMgrImpl::fail(std::string_view message)
{
	throw Exception{"LTJS_EVENT_HANDLER_MGR", message};
}

void EventHandlerMgrImpl::ensure_handler_not_null(EventHandler* handler)
{
	if (handler == nullptr)
	{
		fail("Null handler.");
	}
}

} // namespace

// =====================================

EventHandlerMgrUPtr make_event_handler_mgr()
{
	return std::make_unique<EventHandlerMgrImpl>();
}

} // namespace ltjs::sys
