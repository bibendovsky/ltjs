#include "ltjs_system_event_handler_mgr.h"


#include <cassert>

#include <algorithm>
#include <array>
#include <memory>

#include "ltjs_index_type.h"
#include "ltjs_exception.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SystemEventHandlerMgrIterator::SystemEventHandlerMgrIterator(
	SystemEventHandler* const* handlers) noexcept
	:
	handlers_{handlers}
{
	assert(handlers_);
}

SystemEventHandler* SystemEventHandlerMgrIterator::operator*() const
{
	assert(handlers_);

	return *handlers_;
}

void SystemEventHandlerMgrIterator::operator++()
{
	assert(handlers_);

	handlers_ += 1;
}

bool SystemEventHandlerMgrIterator::operator==(
	const SystemEventHandlerMgrIterator& rhs) const noexcept
{
	return handlers_ == rhs.handlers_;
}

bool SystemEventHandlerMgrIterator::operator!=(
	const SystemEventHandlerMgrIterator& rhs) const noexcept
{
	return !((*this) == rhs);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SystemEventHandlerMgr::SystemEventHandlerMgr() noexcept = default;

SystemEventHandlerMgr::~SystemEventHandlerMgr() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemEventHandlerMgrException :
	public Exception
{
public:
	explicit SystemEventHandlerMgrException(
		const char* message)
		:
		Exception{"LTJS_SYSTEM_EVENT_HANDLER_MGR", message}
	{
	}
}; // SystemEventHandlerMgrException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemEventHandlerMgrImpl :
	public SystemEventHandlerMgr
{
public:
	SystemEventHandlerMgrImpl();


	// ======================================================================
	// SystemEventHandlerMgr

	bool is_empty() const noexcept override;

	Index get_size() const noexcept override;

	SystemEventHandler* get(
		Index index) const override;


	void add(
		SystemEventHandler* handler,
		SystemEventHandlerPriority priority) override;

	void remove(
		SystemEventHandler* handler) override;


	SystemEventHandlerMgrIterator begin() const override;

	SystemEventHandlerMgrIterator end() const override;

	// SystemEventHandlerMgr
	// ======================================================================


private:
	static constexpr auto max_handlers = 16;


	using Handlers = std::array<SystemEventHandler*, max_handlers>;


	Handlers handlers_{};
	Index size_{};
	Index high_priority_size_{};


	static void ensure_handler_not_null(
		SystemEventHandler* handler);
}; // SystemEventHandlerMgrImpl

// ==========================================================================

SystemEventHandlerMgrImpl::SystemEventHandlerMgrImpl() = default;

bool SystemEventHandlerMgrImpl::is_empty() const noexcept
{
	return get_size() == 0;
}

Index SystemEventHandlerMgrImpl::get_size() const noexcept
{
	return size_;
}

SystemEventHandler* SystemEventHandlerMgrImpl::get(
	Index index) const
{
	if (index < 0 || index >= get_size())
	{
		throw SystemEventHandlerMgrException{"Index out of range."};
	}

	return handlers_[index];
}

void SystemEventHandlerMgrImpl::add(
	SystemEventHandler* handler,
	SystemEventHandlerPriority priority)
{
	ensure_handler_not_null(handler);

	auto is_high_priority = false;

	switch (priority)
	{
		case SystemEventHandlerPriority::normal:
			break;

		case SystemEventHandlerPriority::high:
			is_high_priority = true;
			break;

		default:
			throw SystemEventHandlerMgrException{"Unsupported priority value."};
	}

	if (get_size() >= max_handlers)
	{
		throw SystemEventHandlerMgrException{"Full list."};
	}

	if (!is_empty())
	{
		const auto handlers_begin_it = handlers_.begin();

		std::move(handlers_begin_it, handlers_begin_it + get_size(), handlers_begin_it + 1);
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

void SystemEventHandlerMgrImpl::remove(
	SystemEventHandler* handler)
{
	ensure_handler_not_null(handler);

	if (is_empty())
	{
		throw SystemEventHandlerMgrException{"Empty list."};
	}

	auto is_high_priority = false;

	const auto handlers_begin_it = handlers_.begin();

	if (high_priority_size_ > 0)
	{
		const auto high_priority_handlers_end_it = handlers_begin_it + high_priority_size_;
		const auto high_priority_handler_it = std::find(handlers_begin_it, high_priority_handlers_end_it, handler);
		is_high_priority = (high_priority_handler_it != high_priority_handlers_end_it);
	}

	const auto handlers_end_it = handlers_begin_it + get_size();
	const auto next_handler_it = std::remove(handlers_begin_it, handlers_end_it, handler);

	if (next_handler_it == handlers_end_it)
	{
		throw SystemEventHandlerMgrException{"Handler not found."};
	}

	size_ -= 1;
	high_priority_size_ -= is_high_priority;
}

SystemEventHandlerMgrIterator SystemEventHandlerMgrImpl::begin() const
{
	return SystemEventHandlerMgrIterator{handlers_.data()};
}

SystemEventHandlerMgrIterator SystemEventHandlerMgrImpl::end() const
{
	return SystemEventHandlerMgrIterator{handlers_.data() + get_size()};
}

void SystemEventHandlerMgrImpl::ensure_handler_not_null(
	SystemEventHandler* handler)
{
	if (!handler)
	{
		throw SystemEventHandlerMgrException{"Null handler."};
	}
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SystemEventHandlerMgrUPtr make_system_event_handler_mgr()
{
	return std::make_unique<SystemEventHandlerMgrImpl>();
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
