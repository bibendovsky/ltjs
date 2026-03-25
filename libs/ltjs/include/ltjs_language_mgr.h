/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Language resource manager

#ifndef LTJS_LANGUAGE_MGR_INCLUDED
#define LTJS_LANGUAGE_MGR_INCLUDED

#include "ltjs_language.h"
#include <memory>
#include <span>
#include <string_view>

namespace ltjs {

using LanguageMgrLanguages = std::span<const Language>;

class LanguageMgr
{
public:
	LanguageMgr() = default;
	virtual ~LanguageMgr() = default;

	virtual void initialize(std::string_view base_path) noexcept = 0;
	virtual const Language* get_current() const noexcept = 0;
	virtual void set_current_by_id_string(std::string_view id_string) noexcept = 0;
	virtual LanguageMgrLanguages get_languages() const noexcept = 0;
	virtual void load() noexcept = 0;
	virtual void save() noexcept = 0;
};

// =====================================

using LanguageMgrUPtr = std::unique_ptr<LanguageMgr>;

LanguageMgrUPtr make_language_mgr();

} // namespace ltjs

#endif // LTJS_LANGUAGE_MGR_INCLUDED
