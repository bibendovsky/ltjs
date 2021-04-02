#include "ltjs_system_event_mgr.h"


#include <algorithm>
#include <memory>

#include "SDL.h"

#include "ltjs_sdl_subsystem.h"
#include "ltjs_system_event.h"
#include "ltjs_system_event_queue.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SystemEventMgr::SystemEventMgr() noexcept = default;

SystemEventMgr::~SystemEventMgr() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemEventMgrImplException :
	public Exception
{
public:
	explicit SystemEventMgrImplException(
		const char* message)
		:
		Exception{"LTJS_SYSTEM_EVENT_MGR", message}
	{
	}
}; // SystemEventMgrImplException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemEventMgrImpl final :
	public SystemEventMgr
{
public:
	SystemEventMgrImpl();


	SystemEventHandlerMgr* get_handler_mgr() const noexcept override;

	void poll_events() override;

	void handle_events() override;

	bool was_quit_event() const noexcept override;

	void post_quit_event() override;


private:
	static constexpr auto max_event_queue_size = 4'096;


	SdlSubsystem sdl_subsystem_{SDL_INIT_EVENTS};
	SystemEventHandlerMgrUPtr handler_mgr_{};
	SystemEventQueue event_queue_{};
	bool was_quit_event_{};
}; // SystemEventMgrImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SystemEventMgrImpl::SystemEventMgrImpl()
try
{
	handler_mgr_ = make_system_event_handler_mgr();
	event_queue_.set_max_size(max_event_queue_size);
}
catch (...)
{
	std::throw_with_nested(SystemEventMgrImplException{"Failed to initialize system event manager."});
}

SystemEventHandlerMgr* SystemEventMgrImpl::get_handler_mgr() const noexcept
{
	return handler_mgr_.get();
}

void SystemEventMgrImpl::poll_events()
try
{
	event_queue_.clear();

	SystemEvent event;

	while (::SDL_PollEvent(&event))
	{
		if (event.type == ::SDL_QUIT)
		{
			was_quit_event_ = true;
		}
		else
		{
			event_queue_.push(event);
		}
	}
}
catch (...)
{
	std::throw_with_nested(SystemEventMgrImplException{"Failed to poll events."});
}

void SystemEventMgrImpl::handle_events()
try
{
	if (event_queue_.is_empty() || handler_mgr_->is_empty())
	{
		return;
	}

	for (const auto& event : event_queue_)
	{
		for (const auto& handler : (*handler_mgr_))
		{
			if ((*handler)(event))
			{
				break;
			}
		}
	}
}
catch (...)
{
	std::throw_with_nested(SystemEventMgrImplException{"Failed to handle events."});
}

bool SystemEventMgrImpl::was_quit_event() const noexcept
{
	return was_quit_event_;
}

void SystemEventMgrImpl::post_quit_event()
{
	SystemEvent event;
	event.type = ::SDL_QUIT;
	::SDL_PushEvent(&event);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SystemEventMgrUPtr = std::unique_ptr<SystemEventMgr>;

SystemEventMgrUPtr make_system_event_mgr()
{
	return std::make_unique<SystemEventMgrImpl>();
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
