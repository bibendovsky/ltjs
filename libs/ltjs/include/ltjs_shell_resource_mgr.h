/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Shell resource manager

#ifndef LTJS_SHELL_RESOURCE_MGR_INCLUDED
#define LTJS_SHELL_RESOURCE_MGR_INCLUDED

#include <memory>

namespace ltjs {

enum class ShellResourceCodePage
{
	none = 0,
	utf_8,
	windows_1252
};

// =====================================

template<typename T>
struct ShellResourceData
{
	const T* data;
	int size;
};

struct ShellCursorResource
{
	using Data = ShellResourceData<void>;

	int hot_spot_x;
	int hot_spot_y;
	Data data;
};

struct ShellStringResource
{
	using Data = ShellResourceData<char>;

	Data data;
};

struct ShellTextResource
{
	using Data = ShellResourceData<char>;

	Data data;
};

// =====================================

class ShellResourceMgr
{
public:
	ShellResourceMgr() = default;
	virtual ~ShellResourceMgr() = default;

	virtual void initialize(const char* base_path) noexcept = 0;
	virtual void set_language(const char* language_id_name) noexcept = 0;
	virtual ShellResourceCodePage get_code_page() const noexcept = 0;
	virtual const ShellCursorResource* find_cursor(int number) const noexcept = 0;
	virtual const ShellCursorResource* find_cursor_by_number_ptr(const char* number_ptr) const noexcept = 0;
	virtual const ShellStringResource* find_string(int number) const noexcept = 0;
	virtual int load_string(int number, char* buffer, int buffer_size) noexcept = 0;
	virtual const ShellTextResource* find_text(const char* name) const noexcept = 0;
};

// =====================================

using ShellResourceMgrUPtr = std::unique_ptr<ShellResourceMgr>;

ShellResourceMgrUPtr make_shell_resource_mgr();

} // namespace ltjs

#endif // LTJS_SHELL_RESOURCE_MGR_INCLUDED
