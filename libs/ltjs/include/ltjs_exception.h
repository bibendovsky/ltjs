/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Exception utility

#ifndef LTJS_EXCEPTION_INCLUDED
#define LTJS_EXCEPTION_INCLUDED

#include <exception>
#include <string>
#include <string_view>

namespace ltjs {

/*
 * Base class for exceptions.
 */
class Exception : public std::exception
{
public:
	explicit Exception(std::string_view message);
	Exception(std::string_view context, std::string_view message);
	~Exception() override = default;

	const char* what() const noexcept override;

private:
	std::string what_;
};

} // namespace ltjs

#endif // LTJS_EXCEPTION_INCLUDED
