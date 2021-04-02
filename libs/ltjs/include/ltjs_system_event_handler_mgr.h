#ifndef LTJS_SYSTEM_EVENT_HANDLER_MGR_INCLUDED
#define LTJS_SYSTEM_EVENT_HANDLER_MGR_INCLUDED


#include <memory>

#include "ltjs_index_type.h"
#include "ltjs_system_event_handler.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

enum class SystemEventHandlerPriority
{
	normal,
	high,
}; // SystemEventHandlerPriority

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemEventHandlerMgrIterator
{
public:
	explicit SystemEventHandlerMgrIterator(
		SystemEventHandler* const* handlers) noexcept;

	SystemEventHandler* operator*() const;

	void operator++();


	bool operator==(
		const SystemEventHandlerMgrIterator& rhs) const noexcept;

	bool operator!=(
		const SystemEventHandlerMgrIterator& rhs) const noexcept;


private:
	SystemEventHandler* const* handlers_{};
}; // SystemEventHandlerMgrIterator

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// The order of elements: from new to old.
class SystemEventHandlerMgr
{
public:
	SystemEventHandlerMgr() noexcept;

	virtual ~SystemEventHandlerMgr();


	virtual bool is_empty() const noexcept = 0;

	virtual Index get_size() const noexcept = 0;

	virtual SystemEventHandler* get(
		Index index) const = 0;


	virtual void add(
		SystemEventHandler* handler,
		SystemEventHandlerPriority priority) = 0;

	virtual void remove(
		SystemEventHandler* handler) = 0;


	virtual SystemEventHandlerMgrIterator begin() const = 0;

	virtual SystemEventHandlerMgrIterator end() const = 0;
}; // SystemEventHandlerMgr

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SystemEventHandlerMgrUPtr = std::unique_ptr<SystemEventHandlerMgr>;

SystemEventHandlerMgrUPtr make_system_event_handler_mgr();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SYSTEM_EVENT_HANDLER_MGR_INCLUDED
