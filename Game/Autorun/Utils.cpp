// *********************************************************************** //
//
//	MODULE:		UTILS (.CPP)
//
//	PURPOSE:	Gernal purpose utilities
//
//	HISTORY:	02/15/95 [blg] This file was created
//
//	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
//
// *********************************************************************** //


// Includes...

#include "stdafx.h"
#include "utils.h"
#include "malloc.h"
#include "tlhelp32.h"


// Typedefs...

typedef BOOL	(WINAPI *PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef HANDLE	(WINAPI *CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL	(WINAPI *MODULEWALK)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);


// Prototypes...

BOOL	GetProcessModule(DWORD dwPID, DWORD dwModuleID, LPMODULEENTRY32 lpMe32, DWORD cbMe32);
BOOL    ExistWindow(void);


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExistProcess
//
//	PURPOSE:	Determines if the given EXE (or full path, with EXE)
//				is a currently active process.
//
// ----------------------------------------------------------------------- //

BOOL ExistProcess(const char* sExe, int thresh, HANDLE* phProcess)
{
	// Sanity checks..

	ASSERT(sExe);
	if (sExe == NULL) return(FALSE);
	if (sExe[0] == '\0') return(FALSE);

	if (phProcess) *phProcess = NULL;

	// Determine if we were given a full path or just an exe...

	BOOL bFullPath = FALSE;

	if (strstr(sExe, "\\"))
	{
		bFullPath = TRUE;
	}


    // Obtain a module handle to KERNEL so that we can get the addresses of
    // the 32-bit Toolhelp functions we need.

    HMODULE hKernel = GetModuleHandle("KERNEL32.DLL");
	if (!hKernel) return(FALSE);

	CREATESNAPSHOT	pCreateToolhelp32Snapshot = NULL;
	PROCESSWALK		pProcess32First = NULL;
	PROCESSWALK		pProcess32Next  = NULL;

	pCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
	if (!pCreateToolhelp32Snapshot) return(FALSE);

	pProcess32First = (PROCESSWALK)GetProcAddress(hKernel, "Process32First");
	if (!pProcess32First) return(FALSE);

	pProcess32Next = (PROCESSWALK)GetProcAddress(hKernel, "Process32Next");
	if (!pProcess32Next) return(FALSE);


    // Take a snapshot of all processes currently in the system...

    HANDLE hProcessSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == (HANDLE)-1) return(FALSE);


    // Size of the PROCESSENTRY32 structure must be filled out before use...

	PROCESSENTRY32 pe32 = {0};
    pe32.dwSize = sizeof(PROCESSENTRY32);


    // Walk the snapshot of processes and for each process and get information...

	int count = 0;

    if (pProcess32First(hProcessSnap, &pe32))
    {
        MODULEENTRY32 me32 = {0};

        do
        {
            if (GetProcessModule(pe32.th32ProcessID, pe32.th32ModuleID, &me32, sizeof(MODULEENTRY32)))
			{
				if (bFullPath)
				{
					if (stricmp(me32.szExePath, sExe) == 0)
					{
						count++;

						if (count == 1 && phProcess)
						{
							*phProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, me32.th32ProcessID);
						}

						if (count >= thresh)
						{
							return(TRUE);
						}
					}
				}
				else
				{
					if (stricmp(me32.szModule, sExe) == 0)
					{
						count++;

						if (count == 1 && phProcess)
						{
							*phProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, me32.th32ProcessID);
						}

						if (count >= thresh)
						{
							return(TRUE);
						}
					}
				}
            }
        }

        while (pProcess32Next(hProcessSnap, &pe32));
    }


	// If we get here, the process doesn't seem to exist...

	CloseHandle (hProcessSnap);
	return(FALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetProcessModule
//
//	PURPOSE:	Given a Process ID and module ID, return its module info
//
// ----------------------------------------------------------------------- //

BOOL GetProcessModule(DWORD dwPID, DWORD dwModuleID, LPMODULEENTRY32 lpMe32, DWORD cbMe32)
{
    BOOL          bRet        = FALSE;
    BOOL          bFound      = FALSE;
    HANDLE        hModuleSnap = NULL;
    MODULEENTRY32 me32        = {0};

    // Obtain a module handle to KERNEL so that we can get the addresses of
    // the 32-bit Toolhelp functions we need.

    HMODULE hKernel = GetModuleHandle("KERNEL32.DLL");
	if (!hKernel) return(FALSE);

	CREATESNAPSHOT	pCreateToolhelp32Snapshot = NULL;
	MODULEWALK		pModule32First = NULL;
	MODULEWALK		pModule32Next = NULL;

	pCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
	if (!pCreateToolhelp32Snapshot) return(FALSE);

	pModule32First = (MODULEWALK)GetProcAddress(hKernel, "Module32First");
	if (!pModule32First) return(FALSE);

	pModule32Next = (MODULEWALK)GetProcAddress(hKernel, "Module32Next");
	if (!pModule32Next) return(FALSE);


    // Take a snapshot of all modules in the specified process...

    hModuleSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == (HANDLE)-1) return(FALSE);

    // Size of the MODULEENTRY32 structure must be initialized before use!

    me32.dwSize = sizeof(MODULEENTRY32);


    // Walk the module list of the process and find the module we are
    // interested in.  Then, copy the information to the buffer pointed to
    // by lpMe32 so that we can return it to the caller.

    if (pModule32First(hModuleSnap, &me32))
    {
        do
        {
            if (me32.th32ModuleID == dwModuleID)
            {
                CopyMemory (lpMe32, &me32, cbMe32);
                bFound = TRUE;
            }
        }
        while (!bFound && pModule32Next(hModuleSnap, &me32));

        bRet = bFound;   // If this sets bRet to FALSE, then dwModuleID
                         // no longer exsists in the specified process.
    }
    else
        bRet = FALSE;           // Couldn't walk module list.

    // Don't forget to clean up the snapshot object...
    CloseHandle (hModuleSnap);

    return (bRet);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnumWindowsProc
//
//	PURPOSE:	Enumeration function for each window
//
// ----------------------------------------------------------------------- //
static char *winToSearchFor = NULL;
static BOOL  bFoundWindow   = FALSE;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    char lpName[256];

    GetWindowText(hwnd, lpName, 255);

	if (!strlen(lpName) )
		return TRUE;

    // Stop on a match
    if(!stricmp(winToSearchFor, lpName)) 
	{
        bFoundWindow = TRUE;
        return(FALSE);
    }

    return(TRUE);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    ExistsWindow
//
//  PURPOSE:    Searches for the named window
//
// ----------------------------------------------------------------------- //
BOOL ExistWindow(char *winName)
{
    winToSearchFor = winName;
    bFoundWindow   = FALSE;

    EnumWindows(EnumWindowsProc, 0);

    return(bFoundWindow);
}