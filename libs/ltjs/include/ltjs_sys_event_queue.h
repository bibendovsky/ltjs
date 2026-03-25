/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Event queue

#ifndef LTJS_SYS_EVENT_QUEUE_INCLUDED
#define LTJS_SYS_EVENT_QUEUE_INCLUDED

#include "ltjs_circular_queue.h"
#include "ltjs_sys_event.h"

namespace ltjs::sys {

using EventQueue = CircularQueue<Event>;

} // namespace ltjs::sys

#endif // LTJS_SYS_EVENT_QUEUE_INCLUDED
