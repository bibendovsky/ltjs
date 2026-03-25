/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Event manager

#include "ltjs_sys_event_mgr.h"
#include "ltjs_sdl_subsystem.h"
#include "ltjs_sys_event.h"
#include "ltjs_sys_event_queue.h"
#include <algorithm>
#include <memory>
#include "SDL3/SDL.h"

namespace ltjs::sys {

namespace {

class EventMgrImpl final : public EventMgr
{
public:
	EventMgrImpl();
	~EventMgrImpl() override = default;

	EventHandlerMgr* get_handler_mgr() const noexcept override;
	void poll_events() override;
	void handle_events() override;
	bool was_quit_event() const noexcept override;
	void post_quit_event() override;

private:
	static constexpr int max_event_queue_size = 4096;

	sdl::Subsystem sdl_subsystem_{SDL_INIT_EVENTS};
	EventHandlerMgrUPtr handler_mgr_{};
	EventQueue event_queue_{};
	bool was_quit_event_{};
};

// -------------------------------------

EventMgrImpl::EventMgrImpl()
{
	handler_mgr_ = make_event_handler_mgr();
	event_queue_.set_max_size(max_event_queue_size);
}

EventHandlerMgr* EventMgrImpl::get_handler_mgr() const noexcept
{
	return handler_mgr_.get();
}

void EventMgrImpl::poll_events()
{
	event_queue_.clear();
	Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_QUIT)
		{
			was_quit_event_ = true;
		}
		else
		{
			event_queue_.push(event);
		}
	}
}

void EventMgrImpl::handle_events()
{
	if (event_queue_.is_empty() || handler_mgr_->is_empty())
	{
		return;
	}
	for (const Event& event : event_queue_)
	{
		for (EventHandler* handler : *handler_mgr_)
		{
			if (handler->invoke(event))
			{
				break;
			}
		}
	}
}

bool EventMgrImpl::was_quit_event() const noexcept
{
	return was_quit_event_;
}

void EventMgrImpl::post_quit_event()
{
	Event event;
	event.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&event);
}

} // namespace

// =====================================

using EventMgrUPtr = std::unique_ptr<EventMgr>;

EventMgrUPtr make_event_mgr()
{
	return std::make_unique<EventMgrImpl>();
}

} // namespace ltjs::sys
