// LaunchCmd.cpp : Defines the entry point for the console application.
//

static const char  RngInterstitialClientVersion[] = "Version 1.1";

#include "stdafx.h"
#include "stdio.h"
#include "RngInterstitialClient.h"
#include "../RngInterstitial/RngInterstitial.h"

typedef INTERSTITIAL_API int (CALLBACK* GAMEPROC)(UINT);

enum ClickAction {
    CLICK_IGNORE = 0,
    CLICK_PLAY,
    CLICK_NOPLAY
};

#ifdef INTERSTITIAL_STANDALONE
int main(int argc, char* argv[])
{
    int rc = 0;

    rc = RngLoadInterstitialLibrary (1);
    printf ("Begin game dialog returned [%d] '%s'\n", rc, (rc == CLICK_PLAY) ? "Play" : "No Play");

    if (rc == CLICK_PLAY)
    {
        rc = RngLoadInterstitialLibrary (0);
        printf ("Exit game dialog returned [%d] '%s'\n", rc, (rc == CLICK_PLAY) ? "Play" : "No Play");
    }


    printf ("\n------- Paid Mode ----------\n");
    rc = RngLoadInterstitialLibrary (1 + 2);
    printf ("Begin game dialog returned [%d] '%s'\n", rc, (rc == CLICK_PLAY) ? "Play" : "No Play");

    if (rc == CLICK_PLAY)
    {
        rc = RngLoadInterstitialLibrary (0 + 2);
        printf ("Exit game dialog returned [%d] '%s'\n", rc, (rc == CLICK_PLAY) ? "Play" : "No Play");
    }
    return 0;
}
#endif  // INTERSTITIAL_STANDALONE

#ifdef INTERSTITIAL_DLL
BOOL APIENTRY 
DllMain( HANDLE hModule, 
         DWORD  ul_reason_for_call, 
         LPVOID lpReserved )
{
#ifdef _DEBUG
    printf ("==== Built on %s at %s ====\n", __DATE__, __TIME__);
#endif
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;
    }
    return TRUE;
}
#endif  // INTERSTITIAL_DLL

#ifdef INTERSTITIAL_DLL
extern "C"
__declspec(dllexport) 
#endif
int _stdcall RngLoadInterstitial (int nFlags)
{
    int nResult = RngLoadInterstitialLibrary (nFlags);

    return (nResult);
}

int
RngLoadInterstitialLibrary (int nFlags)
{
    HINSTANCE           hDLL            = NULL; // Handle to DLL
    GAMEBEGINENDPROC    lpfmGameProc    = NULL;
    DWORD               dwParam1        = 0;
    UINT                uParam2         = 0;
    UINT                uReturnVal      = 0;

    HKEY                hkResult;
    ULONG               nRegType               = 0;
    UCHAR               szDllName[MAX_PATH];
    ULONG               cbDllName              = sizeof szDllName;
    LONG                rc                     = 0;

    szDllName[0] = '\0';

    rc = RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("Software\\RealGames\\Preferences"),
                      0, KEY_READ, &hkResult);
    if (rc == ERROR_SUCCESS)
    {
        rc = RegQueryValueEx (hkResult, TEXT("RngInterstitialDLL"), 0, &nRegType, &szDllName[0], &cbDllName);
    }
    if (rc != ERROR_SUCCESS || cbDllName == 0)
    {
        strcpy ((char *)szDllName, TEXT("C:\\Program Files\\Real\\RealGames\\RngInterstitial.dll"));
    }
    else {
        RegCloseKey (hkResult);
    }

    hDLL = LoadLibrary((const char *)szDllName);
    if (hDLL != NULL)
    {
       lpfmGameProc = (GAMEBEGINENDPROC) GetProcAddress(hDLL, TEXT("fnGameBeginEnd"));
       if (lpfmGameProc == NULL)
       {
          // handle the error
#ifdef _DEBUG
          printf ("Error getting fuction address of TestDLL:game() [%d]\n", GetLastError());
#endif
          FreeLibrary(hDLL);       
          return 1;
       }
       else
       {
          // call the function before the game starts
          uReturnVal = lpfmGameProc(nFlags);
          FreeLibrary(hDLL);       
       }
    }
    else
    {
#ifdef _DEBUG
        printf ("Error loading DLL file\n");
        uReturnVal = TRUE;
#endif
    }

    return uReturnVal;
}
