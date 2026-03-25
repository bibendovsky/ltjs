/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Shared library

#ifndef LTJS_SYS_SHARED_LIBRARY_INCLUDED
#define LTJS_SYS_SHARED_LIBRARY_INCLUDED

#include <memory>

namespace ltjs::sys {

using SharedLibrarySymbol = void (*)();

class SharedLibrary
{
public:
	SharedLibrary() = default;
	virtual ~SharedLibrary() = default;

	virtual SharedLibrarySymbol find_symbol(const char* symbol_name) noexcept = 0;

	template<typename T>
	T find_symbol(const char* symbol_name) noexcept
	{
		return reinterpret_cast<T>(find_symbol(symbol_name));
	}

	static const char* get_default_suffix() noexcept;
};

// =====================================

using SharedLibraryUPtr = std::unique_ptr<SharedLibrary>;

SharedLibraryUPtr make_shared_library(const char* path);

} // namespace ltjs::sys

#endif // LTJS_SYS_SHARED_LIBRARY_INCLUDED
