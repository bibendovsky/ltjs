#include "ltjs_shared_data_mgr.h"

#include <cstdint>

#include <array>
#include <type_traits>

#include "SDL.h"

#include "ltjs_c_string.h"
#include "ltjs_char_conv.h"
#include "ltjs_exception.h"
#include "ltjs_index_type.h"
#include "ltjs_sdl_ensure_result.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SharedDataMgrImplException :
	public Exception
{
public:
	explicit SharedDataMgrImplException(
		const char* message)
		:
		Exception{"LTJS_SHARED_DATA_MGR", message}
	{
	}
}; // SharedDataMgrImplException
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SharedDataMgrImpl final :
	public SharedDataMgr
{
public:
	SharedDataMgrImpl();


	// ======================================================================
	// SharedDataMgr

	LanguageMgr* get_language_mgr() const noexcept override;

	void set_language_mgr(
		LanguageMgr* language_mgr) noexcept override;


	ShellResourceMgr* get_cres_mgr() const noexcept override;

	void set_cres_mgr(
		ShellResourceMgr* cres_mgr) noexcept override;


	ShellResourceMgr* get_ltmsg_mgr() const noexcept override;

	void set_ltmsg_mgr(
		ShellResourceMgr* ltmsg_mgr) noexcept override;

	// SharedDataMgr
	// ======================================================================


private:
	enum class KnownIndex :
		Index
	{
		language_mgr,
		cres_mgr,
		ltmsg_mgr,

		count_,
	}; // KnownIndex


	static constexpr auto env_name = "LTJS_SHARED_DATA";


	using Datas = std::array<void*, static_cast<Index>(KnownIndex::count_)>;


	class DatasUDeleter
	{
	public:
		DatasUDeleter() noexcept = default;

		explicit DatasUDeleter(
			bool is_owner) noexcept;

		DatasUDeleter(
			const DatasUDeleter& rhs) noexcept;

		void operator=(
			const DatasUDeleter& rhs) noexcept;

		void operator()(
			Datas* resource) const noexcept;


	private:
		bool is_owner_{};
	}; // DatasUDeleter

	using DatasUPtr = std::unique_ptr<Datas, DatasUDeleter>;


	DatasUPtr datas_{};


	template<
		typename T
	>
	T* get(
		KnownIndex index) const noexcept
	{
		return static_cast<T*>((*datas_)[static_cast<Index>(index)]);
	}

	template<
		typename T
	>
	void set(
		KnownIndex index,
		T* value) const noexcept
	{
		(*datas_)[static_cast<Index>(index)] = value;
	}
}; // SharedDataMgrImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SharedDataMgrImpl::DatasUDeleter::DatasUDeleter(
	bool is_owner) noexcept
	:
	is_owner_{is_owner}
{
}

SharedDataMgrImpl::DatasUDeleter::DatasUDeleter(
	const DatasUDeleter& rhs) noexcept
	:
	is_owner_{rhs.is_owner_}
{
}

void SharedDataMgrImpl::DatasUDeleter::operator=(
	const DatasUDeleter& rhs) noexcept
{
	is_owner_ = rhs.is_owner_;
}

void SharedDataMgrImpl::DatasUDeleter::operator()(
	Datas* resource) const noexcept
{
	if (is_owner_)
	{
		delete resource;
	}
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SharedDataMgrImpl::SharedDataMgrImpl()
{
	const auto env_value_string = ::SDL_getenv(env_name);

	if (env_value_string)
	{
		auto env_value = std::uintptr_t{};

		const auto env_value_string_size = c_string::get_size(env_value_string);

		const auto from_chars_result = from_chars(
			env_value_string,
			env_value_string_size,
			16,
			env_value
		);

		if (!from_chars_result)
		{
			throw SharedDataMgrImplException{"Invalid env value."};
		}

		const auto datas_ptr = reinterpret_cast<Datas*>(env_value);
		auto datas = DatasUPtr{datas_ptr, DatasUDeleter{false}};
		datas_.swap(datas);
	}
	else
	{
		auto datas = DatasUPtr{new Datas{}, DatasUDeleter{true}};
		const auto datas_ptr_as_integer = reinterpret_cast<std::uintptr_t>(datas.get());

		constexpr auto char_buffer_size = 32;
		char chars_buffer[char_buffer_size];

		const auto chars_size = to_chars(
			datas_ptr_as_integer,
			16,
			to_chars_format_default,
			chars_buffer,
			char_buffer_size
		);

		if (chars_size == 0)
		{
			throw SharedDataMgrImplException{"Failed to convert env value."};
		}

		chars_buffer[chars_size] = '\0';

		const auto sdl_result = ::SDL_setenv(env_name, chars_buffer, ::SDL_TRUE);

		if (sdl_result != 0)
		{
			throw SharedDataMgrImplException{"Failed to set env value."};
		}

		datas_.swap(datas);
	}
}

LanguageMgr* SharedDataMgrImpl::get_language_mgr() const noexcept
{
	return get<LanguageMgr>(KnownIndex::language_mgr);
}

void SharedDataMgrImpl::set_language_mgr(
	LanguageMgr* language_mgr) noexcept
{
	set<LanguageMgr>(KnownIndex::language_mgr, language_mgr);
}

ShellResourceMgr* SharedDataMgrImpl::get_cres_mgr() const noexcept
{
	return get<ShellResourceMgr>(KnownIndex::cres_mgr);
}

void SharedDataMgrImpl::set_cres_mgr(
	ShellResourceMgr* cres_mgr) noexcept
{
	set<ShellResourceMgr>(KnownIndex::cres_mgr, cres_mgr);
}

ShellResourceMgr* SharedDataMgrImpl::get_ltmsg_mgr() const noexcept
{
	return get<ShellResourceMgr>(KnownIndex::ltmsg_mgr);
}

void SharedDataMgrImpl::set_ltmsg_mgr(
	ShellResourceMgr* ltmsg_mgr) noexcept
{
	set<ShellResourceMgr>(KnownIndex::ltmsg_mgr, ltmsg_mgr);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SharedDataMgrUPtr make_shared_data_mgr()
{
	return std::make_unique<SharedDataMgrImpl>();
}

SharedDataMgr& get_shared_data_mgr()
{
	static auto result = make_shared_data_mgr();
	return *result;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
