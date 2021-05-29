#include "bdefs.h"


#if LTJS_SDL_BACKEND


#include "SDL.h"

#include "iltcursor.h"

#include "ltjs_sdl_uresources.h"
#include "ltjs_shared_data_mgr.h"
#include "ltjs_shell_resource_mgr.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);


//
//Internal implementation class for ILTCursorInst.
//

class CLTCursorInst final :
	public ILTCursorInst
{
public:
	using Resource = ::SDL_Cursor*;


	LTRESULT IsValid() override 
	{
		return (m_hCursor ? LT_YES : LT_NO);
	};

	Resource GetCursor() const
	{
		return m_hCursor;
	};

	void SetData(
		const void* pData) override
	{
		m_hCursor = static_cast<Resource>(const_cast<void*>(pData));
	};

	void* GetData() override
	{
		return m_hCursor;
	};


private:
	Resource m_hCursor{};
};


//
//Our implementation class for the ILTCursor interface.
//

class CLTCursor final :
	public ILTCursor
{
public:
	declare_interface(CLTCursor);

	CLTCursor()
	{
		m_hCurrentCursor = LTNULL;
		m_eCursorMode = CM_None;
	}

	// Enable/disable hardware cursor.
	LTRESULT SetCursorMode(
		CursorMode cMode,
		bool bForce = false) override;

	// Get current cursor mode.  Always returns LT_OK and always fills in cMode.
	LTRESULT GetCursorMode(
		CursorMode& cMode) override;

	// Returns LT_YES if a hardware cursor can be used, LT_NO otherwise.
	// Since we can't detect this, just return LT_YES for now.
	LTRESULT IsCursorModeAvailable(
		CursorMode cMode) override;

	// Set the current hardware cursor bitmap.  The bitmap comes from cshell.dll.
	LTRESULT LoadCursorBitmapResource(
		const char* pName,
		HLTCURSOR& hCursor) override;

	// Free a cursor.
	LTRESULT FreeCursor(
		const HLTCURSOR hCursor) override;

	// Set the current cursor.
	LTRESULT SetCursor(
		HLTCURSOR hCursor) override;

	// Check if an HLTCURSOR is a valid one; returns LT_YES or LT_NO
	LTRESULT IsValidCursor(
		HLTCURSOR hCursor) override;

	// Refresh the cursor
	LTRESULT RefreshCursor() override;


protected:
	LTRESULT PreSetMode(
		CursorMode eNewMode);

	LTRESULT PostSetMode(
		CursorMode eOldMode);

	CursorMode m_eCursorMode;
	HLTCURSOR m_hCurrentCursor;
};

//instantiate our implementation class.
define_interface(CLTCursor, ILTCursor);



LTRESULT CLTCursor::PreSetMode(
	CursorMode eNewMode)
{
	auto cursor_mode = -1;

	switch (eNewMode)
	{
		case CM_Hardware:
			cursor_mode = 1;
			break;

		case CM_None:
			cursor_mode = 0;
			break;

		default:
			break;
	}

	if (cursor_mode >= 0)
	{
		::SDL_ShowCursor(cursor_mode);
	}

	return LT_OK;
}

LTRESULT CLTCursor::PostSetMode(
	CursorMode eOldMode)
{
	return LT_OK;
}

LTRESULT CLTCursor::SetCursorMode(
	CursorMode cMode,
	bool bForce)
{
	CursorMode eOldMode;

	// Saaaaaanity check
	if (!IsCursorModeAvailable(cMode))
	{
		return LT_UNSUPPORTED;
	}

	// If we're already in this mode, let's just save some cycles, shall we?
	if (cMode == m_eCursorMode && !bForce)
	{
		return LT_OK;
	}

	/* In the future, as more things (such as software cursor support) are added,
	 * this routine will become more expensive, likely. */

	PreSetMode(cMode);
	eOldMode = m_eCursorMode;
	m_eCursorMode = cMode;
	PostSetMode(eOldMode);

	return LT_OK;
}

LTRESULT CLTCursor::IsValidCursor(
	HLTCURSOR hCursor)
{
	return hCursor->IsValid();
}

LTRESULT CLTCursor::SetCursor(
	HLTCURSOR hCursor)
{
	if (!hCursor->IsValid())
	{
		return LT_INVALIDPARAMS;
	}

	const auto sdl_cursor = static_cast<CLTCursorInst*>(hCursor)->GetCursor();

	::SDL_SetCursor(sdl_cursor);

	return LT_OK;
}

LTRESULT CLTCursor::GetCursorMode(
	CursorMode& cMode)
{
	cMode = m_eCursorMode;

	return LT_OK;
}

LTRESULT CLTCursor::FreeCursor(
	const HLTCURSOR hCursor)
{
	const auto sdl_cursor = static_cast<CLTCursorInst*>(hCursor)->GetCursor();
	::SDL_FreeCursor(sdl_cursor);

	delete static_cast<CLTCursorInst*>(hCursor);

	return LT_OK;
}

LTRESULT CLTCursor::IsCursorModeAvailable(
	CursorMode cMode)
{
	/* Eventually there may be more checks here */

	return LT_YES;
}

LTRESULT CLTCursor::LoadCursorBitmapResource(
	const char* pName,
	HLTCURSOR& hCursor)
{
	auto cres_mgr = ltjs::get_shared_data_mgr().get_cres_mgr();

	if (!cres_mgr)
	{
		return LT_MISSINGCURSORRESOURCE;
	}

	const auto shell_cursor = cres_mgr->find_cursor_by_number_ptr(pName);

	if (!shell_cursor)
	{
		return LT_MISSINGCURSORRESOURCE;
	}

	const auto sdl_rwops = ltjs::SdlRwOpsUResource{::SDL_RWFromConstMem(
		shell_cursor->data.data,
		shell_cursor->data.size
	)};

	if (!sdl_rwops)
	{
		return LT_MISSINGCURSORRESOURCE;
	}

	const auto sdl_surface = ltjs::SdlSurfaceUResource{::SDL_LoadBMP_RW(sdl_rwops.get(), 0)};

	if (!sdl_surface)
	{
		return LT_MISSINGCURSORRESOURCE;
	}

	const auto sdl_cursor = ::SDL_CreateColorCursor(
		sdl_surface.get(),
		shell_cursor->hot_spot_x,
		shell_cursor->hot_spot_y
	);

	if (!sdl_cursor)
	{
		return LT_MISSINGCURSORRESOURCE;
	}

	auto hNew = HLTCURSOR{};
	LT_MEM_TRACK_ALLOC(hNew = new CLTCursorInst(), LT_MEM_TYPE_MISC);

	hNew->SetData(sdl_cursor);
	hCursor = hNew;

	return LT_OK;
}

LTRESULT CLTCursor::RefreshCursor()
{
	auto cursor_mode = -1;

	switch (m_eCursorMode)
	{
		case CM_Hardware:
			cursor_mode = 1;
			break;

		case CM_None:
			cursor_mode = 0;
			break;

		default:
			break;
	}

	if (cursor_mode >= 0)
	{
		::SDL_ShowCursor(cursor_mode);
	}

	return LT_OK;
}


#endif // LTJS_SDL_BACKEND
