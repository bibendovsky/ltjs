/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Event handler

#ifndef LTJS_SYS_EVENT_HANDLER_INCLUDED
#define LTJS_SYS_EVENT_HANDLER_INCLUDED

#include "ltjs_sys_event.h"

namespace ltjs::sys {

class EventHandler
{
public:
	EventHandler() = default;
	virtual ~EventHandler() = default;

	virtual bool invoke(const Event& event) = 0;
};

} // namespace ltjs::sys

#endif // LTJS_SYS_EVENT_HANDLER_INCLUDED
