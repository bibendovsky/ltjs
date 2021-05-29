#ifndef LTJS_LANGUAGE_MGR_INCLUDED
#define LTJS_LANGUAGE_MGR_INCLUDED


#include <memory>

#include "ltjs_index_type.h"
#include "ltjs_language.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using LanguageMgrLanguages = LanguageData<const Language>;


class LanguageMgr
{
public:
	LanguageMgr();

	virtual ~LanguageMgr();


	virtual void initialize(
		const char* base_path) noexcept = 0;


	virtual const Language* get_current() const noexcept = 0;

	virtual void set_current_by_id_string(
		const char* id_string) noexcept = 0;

	virtual LanguageMgrLanguages get_languages() const noexcept = 0;


	virtual void load() noexcept = 0;

	virtual void save() noexcept = 0;
}; // LanguageMgr

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using LanguageMgrUPtr = std::unique_ptr<LanguageMgr>;

LanguageMgrUPtr make_language_mgr();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_LANGUAGE_MGR_INCLUDED
