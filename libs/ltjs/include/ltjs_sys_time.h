#ifndef LTJS_SYS_TIME_INCLUDED
#define LTJS_SYS_TIME_INCLUDED

#include <optional>

namespace ltjs::sys {

using TimeNs = long long; // Nanoseconds since Jan 1, 1970 (UTC)
using TimeNsOpt = std::optional<TimeNs>;

struct DateTime
{
	int year;
	int month; // [1..12]
	int day; // [1..31]
	int hour; // [0..23]
	int minute; // [0..59]
	int second; // [0..59]
	int nanosecond; // [0-999999999]
	int day_of_week; // [0-6] (0 - Sunday)
	int utc_offset_s; // UTC offset (seconds)
};

using DateTimeOpt = std::optional<DateTime>;

TimeNsOpt get_current_time_ns();
DateTimeOpt time_ns_to_date_time_local(TimeNs time_ns);
DateTimeOpt time_ns_to_date_time_local(TimeNsOpt time_ns_opt);

} // namespace ltjs::sys

#endif // LTJS_SYS_TIME_INCLUDED
