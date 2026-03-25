/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Shared data manager

#ifndef LTJS_SHARED_DATA_MGR_INCLUDED
#define LTJS_SHARED_DATA_MGR_INCLUDED

#include <memory>

namespace ltjs {

class LanguageMgr;
class ShellResourceMgr;

// =====================================

struct SharedDataMgrInitParam
{
	LanguageMgr* language_mgr;
	ShellResourceMgr* cres_mgr;
	ShellResourceMgr* ltmsg_mgr;
};

class SharedDataMgr
{
public:
	SharedDataMgr() = default;
	virtual ~SharedDataMgr() = default;

	virtual LanguageMgr* get_language_mgr() const = 0;
	virtual ShellResourceMgr* get_cres_mgr() const = 0;
	virtual ShellResourceMgr* get_ltmsg_mgr() const = 0;
};

// =====================================

using SharedDataMgrUPtr = std::unique_ptr<SharedDataMgr>;

SharedDataMgr& get_shared_data_mgr();
void initialize_shared_data_mgr(const SharedDataMgrInitParam& param);

} // namespace ltjs

#endif // LTJS_SHARED_DATA_MGR_INCLUDED
