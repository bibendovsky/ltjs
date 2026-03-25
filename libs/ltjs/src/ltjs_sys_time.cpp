/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Time utility

#include "ltjs_sys_time.h"
#include <cassert>
#include "SDL3/SDL_time.h"

namespace ltjs::sys {

TimeNsOpt get_current_time_ns()
{
	if (SDL_Time sdl_time;
		SDL_GetCurrentTime(&sdl_time))
	{
		return TimeNsOpt{sdl_time};
	}
	return TimeNsOpt{};
}

DateTimeOpt time_ns_to_date_time_local(TimeNs time_ns)
{
	if (SDL_DateTime sdl_date_time;
		SDL_TimeToDateTime(time_ns, &sdl_date_time, true))
	{
		return DateTime{
			.year = sdl_date_time.year,
			.month = sdl_date_time.month,
			.day = sdl_date_time.day,
			.hour = sdl_date_time.hour,
			.minute = sdl_date_time.minute,
			.second = sdl_date_time.second,
			.nanosecond = sdl_date_time.nanosecond,
			.day_of_week = sdl_date_time.day_of_week,
			.utc_offset_s = sdl_date_time.utc_offset};
	}
	else
	{
		assert(false && "SDL_TimeToDateTime");
		return DateTimeOpt{};
	}
}

DateTimeOpt time_ns_to_date_time_local(TimeNsOpt time_ns_opt)
{
	if (!time_ns_opt.has_value())
	{
		return DateTimeOpt{};
	}
	return time_ns_to_date_time_local(time_ns_opt.value());	
}

} // namespace ltjs::sys
