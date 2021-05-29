#ifndef LTJS_SHARED_DATA_MGR_INCLUDED
#define LTJS_SHARED_DATA_MGR_INCLUDED


#include <memory>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ILTClient;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


namespace ltjs
{

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class LanguageMgr;
class ShellResourceMgr;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SharedDataMgr
{
public:
	SharedDataMgr() noexcept = default;

	virtual ~SharedDataMgr() = default;


	virtual LanguageMgr* get_language_mgr() const noexcept = 0;

	virtual void set_language_mgr(
		LanguageMgr* language_mgr) noexcept = 0;


	virtual ShellResourceMgr* get_cres_mgr() const noexcept = 0;

	virtual void set_cres_mgr(
		ShellResourceMgr* cres_mgr) noexcept = 0;


	virtual ShellResourceMgr* get_ltmsg_mgr() const noexcept = 0;

	virtual void set_ltmsg_mgr(
		ShellResourceMgr* ltmsg_mgr) noexcept = 0;
}; // SharedDataMgr

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SharedDataMgrUPtr = std::unique_ptr<SharedDataMgr>;

SharedDataMgrUPtr make_shared_data_mgr();

SharedDataMgr& get_shared_data_mgr();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SHARED_DATA_MGR_INCLUDED
