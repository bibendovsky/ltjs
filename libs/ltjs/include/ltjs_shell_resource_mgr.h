#ifndef LTJS_SHELL_RESOURCE_MGR_INCLUDED
#define LTJS_SHELL_RESOURCE_MGR_INCLUDED


#include <memory>

#include "ltjs_index_type.h"
#include "ltjs_ucs.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

enum class ShellResourceCodePage
{
	none,

	utf_8,
	windows_1252,
}; // ShellResourceCodePage

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename T
>
struct ShellResourceData
{
	const T* data{};
	Index size{};
}; // ShellCursorResource


struct ShellCursorResource
{
	using Data = ShellResourceData<void>;


	int hot_spot_x{};
	int hot_spot_y{};

	Data data{};
}; // ShellCursorResource

struct ShellStringResource
{
	using Data = ShellResourceData<char>;

	Data data{};
}; // ShellStringResource

struct ShellTextResource
{
	using Data = ShellResourceData<char>;

	Data data{};
}; // ShellTextResource

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ShellResourceMgr
{
public:
	ShellResourceMgr() noexcept;

	virtual ~ShellResourceMgr();


	virtual void initialize(
		const char* base_path) noexcept = 0;

	virtual void set_language(
		const char* language_id_name) noexcept = 0;


	virtual ShellResourceCodePage get_code_page() const noexcept = 0;


	virtual const ShellCursorResource* find_cursor(
		int number) const noexcept = 0;

	virtual const ShellCursorResource* find_cursor_by_number_ptr(
		const char* number_ptr) const noexcept = 0;


	virtual const ShellStringResource* find_string(
		int number) const noexcept = 0;

	virtual Index load_string(
		int number,
		char* buffer,
		Index buffer_size) noexcept = 0;


	virtual const ShellTextResource* find_text(
		const char* name) const noexcept = 0;
}; // ShellResourceMgr

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using ShellResourceMgrUPtr = std::unique_ptr<ShellResourceMgr>;

ShellResourceMgrUPtr make_shell_resource_mgr();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SHELL_RESOURCE_MGR_INCLUDED
