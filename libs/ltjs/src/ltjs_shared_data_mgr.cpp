/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Shared data manager

#include "ltjs_shared_data_mgr.h"
#include "ltjs_exception.h"
#include <cassert>
#include <cstdint>
#include <charconv>
#include <memory>
#include <string_view>
#include "SDL3/SDL_stdinc.h"

namespace ltjs {

namespace {

class SharedDataMgrImpl final : public SharedDataMgr
{
public:
	SharedDataMgrImpl(const SharedDataMgrInitParam* param);
	~SharedDataMgrImpl() override;

	LanguageMgr* get_language_mgr() const override;
	ShellResourceMgr* get_cres_mgr() const override;
	ShellResourceMgr* get_ltmsg_mgr() const override;

private:
	enum KnownIndex
	{
		language_mgr_index = 0,
		cres_mgr_index,
		ltmsg_mgr_index,
		//
		known_index_count_
	};

	static constexpr const char* env_name = "LTJS_SHARED_DATA_8GWFRPEVHDUJTKZF";

	void** pointers_;
	bool owns_pointers_;

	[[noreturn]] static void fail(std::string_view message);

	template<typename T>
	T* get(KnownIndex index) const
	{
		assert(index >= 0 || index < known_index_count_);
		return static_cast<T*>(pointers_[index]);
	}
};

// -------------------------------------

SharedDataMgrImpl::SharedDataMgrImpl(const SharedDataMgrInitParam* param)
{
	if (param != nullptr)
	{
		assert(param->language_mgr != nullptr);
		assert(param->cres_mgr != nullptr);
		assert(param->ltmsg_mgr != nullptr);
		auto pointers = std::make_unique<void*[]>(known_index_count_);
		const std::uintptr_t datas_ptr_as_integer = reinterpret_cast<std::uintptr_t>(pointers.get());
		constexpr int char_buffer_size = 32;
		char chars_buffer[char_buffer_size];
		const auto [char_buffer_end, ec] = std::to_chars(
			std::begin(chars_buffer),
			std::end(chars_buffer),
			datas_ptr_as_integer,
			16);
		if (ec != std::errc{})
		{
			fail("Failed to format env value.");
		}
		*char_buffer_end = '\0';
		const int sdl_result = SDL_setenv_unsafe(env_name, chars_buffer, true);
		if (sdl_result != 0)
		{
			fail("Failed to set env value.");
		}
		pointers[language_mgr_index] = param->language_mgr;
		pointers[cres_mgr_index] = param->cres_mgr;
		pointers[ltmsg_mgr_index] = param->ltmsg_mgr;
		pointers_ = pointers.release();
		owns_pointers_ = true;
	}
	else
	{
		const std::string_view env_value_string{SDL_getenv(env_name)};
		if (env_value_string.empty())
		{
			fail("No env value.");
		}
		std::uintptr_t env_value = 0;
		const auto [env_value_string_iter, ec] = std::from_chars(
			env_value_string.data(),
			env_value_string.data() + env_value_string.size(),
			env_value,
			16);
		if (ec != std::errc{})
		{
			fail("Invalid env value.");
		}
		pointers_ = reinterpret_cast<void**>(env_value);
		owns_pointers_ = false;
	}
}

SharedDataMgrImpl::~SharedDataMgrImpl()
{
	if (owns_pointers_)
	{
		delete [] pointers_;
	}
}

LanguageMgr* SharedDataMgrImpl::get_language_mgr() const
{
	return get<LanguageMgr>(language_mgr_index);
}

ShellResourceMgr* SharedDataMgrImpl::get_cres_mgr() const
{
	return get<ShellResourceMgr>(cres_mgr_index);
}

ShellResourceMgr* SharedDataMgrImpl::get_ltmsg_mgr() const
{
	return get<ShellResourceMgr>(ltmsg_mgr_index);
}

[[noreturn]] void SharedDataMgrImpl::fail(std::string_view message)
{
	throw Exception{"LTJS_SHARED_DATA_MGR", message};
}

// =====================================

SharedDataMgr& get_shared_data_mgr(const SharedDataMgrInitParam* param)
{
	static SharedDataMgrUPtr result = std::make_unique<SharedDataMgrImpl>(param);
	return *result;
}

} // namespace

// =====================================

SharedDataMgr& get_shared_data_mgr()
{
	return get_shared_data_mgr(nullptr);
}

void initialize_shared_data_mgr(const SharedDataMgrInitParam& param)
{
	[[maybe_unused]] SharedDataMgr& shared_data_mgr = get_shared_data_mgr(&param);
}

} // namespace ltjs
