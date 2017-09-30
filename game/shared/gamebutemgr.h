// ----------------------------------------------------------------------- //
//
// MODULE  : GameButeMgr.h
//
// PURPOSE : GameButeMgr definition - Base class for all bute mgrs
//
// CREATED : 3/30/99
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BUTE_MGR_H__
#define __GAME_BUTE_MGR_H__

#include "butemgr.h"

#pragma warning( disable : 4786 )
#include <unordered_map>


void GBM_DisplayError(const char* szMsg);


// ----------------------------------------------------------------------- //
//
// Character pointer hash map definition...
//
// ----------------------------------------------------------------------- //

typedef std::unordered_map< const char *, int, ButeMgrHashCompare, ButeMgrHashCompare > IndexTable;



class CGameButeMgr
{
	public :

        virtual LTBOOL Init(const char* szAttributeFile="") = 0;
		virtual void Term( ) { m_buteMgr.Term( ); m_strAttributeFile.Empty( ); }

		CGameButeMgr()
		{
			m_buteMgr.Init(GBM_DisplayError);
            m_pCryptKey = LTNULL;
            m_bInRezFile = LTTRUE;
		}

		virtual ~CGameButeMgr() { Term( ); }

        inline void SetInRezFile(LTBOOL bInRezFile) { m_bInRezFile = bInRezFile; }

		char const* GetAttributeFile( ) { return m_strAttributeFile; }

		virtual void Save();

	protected :

		CString		m_strAttributeFile;
		CButeMgr	m_buteMgr;

		char*		m_pCryptKey;
        LTBOOL       m_bInRezFile;

        LTBOOL       Parse(const char* sButeFile);
};


#endif // __GAME_BUTE_MGR_H__
