/****************************************************************************
;
;	 MODULE:		AUTORUN (.CPP)
;
;	PURPOSE:		AutoRun Program
;
;	HISTORY:
;
;	COMMENT:		Copyright (c) 1996-1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "stdafx.h"
#include "regmgr32.h"
#include "resource.h"
#include "utils.h"


// Globals...

BOOL        g_bIsWinNT = FALSE;
HINSTANCE	g_hInst    = NULL;
CRegMgr32*	g_pRegMgr  = NULL;
char		g_sCurDir[256];

char		g_sAppRegVersion[] = { "1.0" };
char		g_sAppName[100]    = { "" };
char		g_sSetupName[100]    = { "" };
char		g_sAppExe[100]     = { "" };

#ifdef _TO2DEMO
char const GAME_NAME[] = "No One Lives Forever 2 (Official Demo)";
#else
char const GAME_NAME[] = "No One Lives Forever 2";
#endif


// Prototypes...

int		WINAPI WinMain(HINSTANCE hInst,	HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd);
BOOL	ExistFile(char* sPath, char* sFile);
BOOL	LaunchApp(char* sPath, char* sApp);
BOOL	ExistFile(char* sFile);
BOOL	ExistFile(char* sDir, char* sFile);
int		StringMessageBox(HWND hWnd, UINT idString, const char* sTitle, UINT uType);
BOOL	AskYesNo(HWND hWnd, UINT idString, const char* sTitle);
BOOL	OpenRegistryKey();

// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WinMain
//
//	PURPOSE:	Main entry point of the program
//
// ----------------------------------------------------------------------- //

int WINAPI WinMain(HINSTANCE hInst,	HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
    // Determine if running under Win NT/2000
    OSVERSIONINFO verInfo;
    memset(&verInfo, 0, sizeof(OSVERSIONINFO));
    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&verInfo);
    if(verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        g_bIsWinNT = TRUE;
    }

	// Load some strings...
	LoadString(hInst, IDS_APPNAME, g_sAppName, 99);
	LoadString(hInst, IDS_SETUPNAME, g_sSetupName, 99);
	LoadString(hInst, IDS_APPEXE, g_sAppExe, 99);


	// Check if main app is already running...

	if (ExistProcess(g_sAppExe))
	{
		return(0);
	}


	// Check if we are already running...

	if (ExistProcess("AUTORUN.EXE", 2))
	{
		return(0);
	}

    // ExistProcess won't work under NT/2000, look for Setup's window instead


    if(ExistWindow(g_sSetupName)) 
	{
		return(0);
	}

	// Set misc variables...

	g_hInst = hInst;

	g_sCurDir[0] = '\0';
	GetCurrentDirectory(254, g_sCurDir);
	int l = strlen(g_sCurDir);
	if (l > 0 && g_sCurDir[l-1] == '\\') g_sCurDir[l-1] = '\0';


	// Create and init the registry manager...

	g_pRegMgr = new CRegMgr32();
	if (!g_pRegMgr) return(0);
	g_pRegMgr->Init();


	// If we're installed, launch the game from the hard drive... otherwise, launch
	// it from the CD.
	char sInstallDir[256] = "";
	char sTargetDir[256] = "";
	if (OpenRegistryKey())
	{
		unsigned long nSize = sizeof(sInstallDir);
		g_pRegMgr->GetField("InstallDir",sInstallDir,nSize);
	}

	strncpy(sTargetDir,g_sCurDir,sizeof(sTargetDir));
	if (sInstallDir[0] != '\0')
	{
		if(ExistFile(sInstallDir, g_sAppExe))
		{
			// We're installed, so launch frmo the hard drive
			strncpy(sTargetDir,sInstallDir,sizeof(sTargetDir));
			SetCurrentDirectory(sInstallDir);
		}
	}


	if(!LaunchApp(sTargetDir,g_sAppExe))
	{
		StringMessageBox(NULL, IDS_CANTLAUNCHGAME, g_sAppName, MB_ICONSTOP);
	}


	// Clean up...

	if (g_pRegMgr)
	{
		delete g_pRegMgr;
		g_pRegMgr = NULL;
	}


	// Exit our program, returning our exit code...

	return(1);
}

BOOL LaunchApp(char* sPath, char* sApp)
{
	BOOL				bRet;
	PROCESS_INFORMATION processInfo;
	STARTUPINFO			startInfo;

	memset(&startInfo, 0, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);

	char sLaunch[256];

	if (sPath[0] != '\0')
	{
		wsprintf(sLaunch, "%s\\%s", sPath, sApp);
	}
	else
	{
		wsprintf(sLaunch, "%s", sApp);
	}

	if (sPath[0] == '\0') sPath = NULL;

	bRet = CreateProcess(NULL, sLaunch, NULL, NULL, FALSE, 0, NULL, sPath, &startInfo, &processInfo);

	return(bRet);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExistFile
//
//	PURPOSE:	Determines if the given file exists
//
// ----------------------------------------------------------------------- //

BOOL ExistFile(char* sDir, char* sFile)
{
	if (!sDir) return(FALSE);
	if (!sFile) return(FALSE);
	if (sFile[0] == '\0') return(FALSE);
	if (sDir[0] == '\0') return(ExistFile(sFile));

	char sTmp[256];

	strcpy(sTmp, sDir);
	int l = strlen(sTmp);
	if (l > 0 && sTmp[l-1] == '\\') sTmp[l-1] = '\0';

	strcat(sTmp, "\\");
	strcat(sTmp, sFile);

	OFSTRUCT ofs;

	HFILE hFile = OpenFile(sTmp, &ofs, OF_EXIST);

	return(hFile != HFILE_ERROR);
}

BOOL ExistFile(char* sFile)
{
	if (!sFile) return(FALSE);
	if (sFile[0] == '\0') return(FALSE);

	OFSTRUCT ofs;

	HFILE hFile = OpenFile(sFile, &ofs, OF_EXIST);

	return(hFile != HFILE_ERROR);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StringMessageBox
//
//	PURPOSE:	Message box with a string ID
//
// ----------------------------------------------------------------------- //

int StringMessageBox(HWND hWnd, UINT idString, const char* sTitle, UINT uType)
{
	// Load the string...

	char sText[256];
	strcpy(sText, "Error.");

	LoadString(g_hInst, idString, sText, 250);


	// Do the message box...

	return(MessageBox(hWnd, sText, sTitle, uType));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AskYesNo
//
//	PURPOSE:	Asks a yes/no question
//
// ----------------------------------------------------------------------- //

BOOL AskYesNo(HWND hWnd, UINT idString, const char* sTitle)
{
	// Load the string...

	char sText[256];
	LoadString(g_hInst, idString, sText, 250);


	// Do the message box...

	UINT uType = MB_YESNO | MB_ICONQUESTION;

	int nRet = MessageBox(hWnd, sText, sTitle, uType);

	return(nRet == IDYES);
}


BOOL OpenRegistryKey()
{
	return(g_pRegMgr->OpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE","Monolith Productions",(char*)GAME_NAME,g_sAppRegVersion));
}
