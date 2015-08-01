//=============================================================================
//
//	SimpleSample.cpp :	MSVC Auto generated Hello World App with changes to show
//						RealArcade integration.
//
// 	Author:		Andrew Johnston, Paul Ingalls
//				RealNetworks 2001
//
//	History:
//
//	$Log: /SimpleSample/SimpleSample.cpp $
//	
//	5     3/06/01 3:06p Ajohnston
//	Make sure to ReleaseDC after EndPaint
//	
//	4     3/04/01 5:08p Ajohnston
//	Change interstitial call to only take place after check for Arcade
//	
//	3     3/03/01 4:22p Ajohnston
//	Add comments to the code
//	
//	2     3/02/01 9:04p Ajohnston
//	Add RealNetworks Interstitial manager calls
//
//	1     2/25/01 5:04p Paul Ingalls
//	Add displaying of Arcade messages in HelloWorld app to WndProc 
//
//=============================================================================

#include "stdafx.h"
#include "resource.h"
#include "crtdebug.h"

// RealNetworks Interstitial DLL
#include "rnginterstitialclient.h"

// RealArcade client lib
#include "rngclib.h"

// RealNetworks RealArcade SDK		
#include "rngcclientenginelib_i.c"
#include "irngameconsole_i.c"
#include "irngcclientengine_i.c"
#include "irngcmessage_i.c"

#define MAX_LOADSTRING 100
#define MAX_MESSAGESTRING 2048

// Global Variables:
HINSTANCE g_hInst;                              // current instance
TCHAR g_szTitle[MAX_LOADSTRING];	        // The title bar text
TCHAR g_szWindowClass[MAX_LOADSTRING];          // The window class
TCHAR g_szLastMessageDesc[MAX_MESSAGESTRING];   // The window class
IRNGameConsole * g_pConsole = NULL;             // RealArcade Console object pointer					

// Foward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
BOOL AppIdle (const HWND hwndGame);
HRESULT CheckRNMessages(const HWND hWndGame);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow
                     )
{
    //
    //	Adjust the behavior of the debug heap manager
    //
    
    CLEAR_CRT_DEBUG_FIELD(_CRTDBG_CHECK_ALWAYS_DF);
    //	SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_CRT_DF );
    SET_CRT_DEBUG_FIELD( _CRTDBG_ALLOC_MEM_DF );
    SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
    
    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_SIMPLESAMPLE, g_szWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDS_HELLO, g_szLastMessageDesc, MAX_MESSAGESTRING);

    MyRegisterClass(hInstance);

    //
    // init RealArcade
    //
    {
        HRESULT res = S_OK;
    
		//
        // Initialize the console connection
		//

		// {69457320-0433-11d5-92FC-0002B306C586}
        static const GUID SimpleSampleID = 
        { 0x69457320, 0x433, 0x11d5, { 0x92, 0xfc, 0x0, 0x2, 0xb3, 0x6, 0xc5, 0x86 } };
    
        res = RNInitConsole(SimpleSampleID, TRUE, &g_pConsole);
        if (FAILED(res))
        {
	    //
	    // NOTE;	RNInitConsole will FAIL if Arcade cannot find the game!
	    //			Make sure you have run the SimpleSample.rgi to "install" the game
	    //			before trying to run the game.
	    //
	    //			When debugging make sure that the installed game name matches the
	    //			debug game name (SimpleSample.exe) 
	    //
		    TRACE("RNInitConsole Failed with code 0x%X\n", res);
	    //
 	    //	RealNetworks interstitialial dialog
	    //	Really should only be called for free (demo) versions of the game
	    //
	    res = RngLoadInterstitialLibrary(1);   // Show dialog before game
	    if(res == 0 ) 
            {
		    return E_FAIL;
	    }

            // return res; // if you want RealArcade to be REQUIRED to run then exit the game here
        }
    }    
    //
    //	end  RealArcade initialization
    //
    

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)) 
    {
        return FALSE;
    }
    
    // Main message loop:
    MSG msg;
    msg.wParam = 0;
    
    for (;;) 
    {			
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))  
        {
            if (msg.message != WM_QUIT) 
            {
                TranslateMessage (&msg);
                DispatchMessage  (&msg);
            }
            else 
            {
                break;
            }
        }
        else if (AppIdle (NULL) ) 
        {
            WaitMessage ();
        }
    }
    
    // shutdown our console connection
    if(g_pConsole)
    {
        RNTermConsole(g_pConsole);
	g_pConsole = NULL;
    } 
    else 
    {
	//
  	//	RealNetworks interstitialial dialog
	//	Really should only be called for free (demo) versions of the game
	//
	RngLoadInterstitialLibrary(0);   // Show dialog after game
    }
   
    return msg.wParam;
}

//====================================================================================
//	AppIdle
//
//	Description:
//====================================================================================
BOOL AppIdle (const HWND hwndGame)
{
    // Check to see if the active app
    // if so do something (like appPaint), else if not active return TRUE
    CheckRNMessages(hwndGame);
    return FALSE;
} // AppIdle


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    
    wcex.cbSize = sizeof(WNDCLASSEX); 
    
    wcex.style		= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)WndProc;
    wcex.cbClsExtra	= 0;
    wcex.cbWndExtra	= 0;
    wcex.hInstance	= hInstance;
    wcex.hIcon		= LoadIcon(hInstance, (LPCTSTR)IDI_SIMPLESAMPLE);
    wcex.hCursor	= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= (LPCSTR)IDC_SIMPLESAMPLE;
    wcex.lpszClassName	= g_szWindowClass;
    wcex.hIconSm	= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
    
    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    
    g_hInst = hInstance; // Store instance handle in our global variable
    
    hWnd = CreateWindow(g_szWindowClass, 
                        g_szTitle, 
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 
                        0, 
                        CW_USEDEFAULT, 
                        0, 
                        NULL, 
                        NULL, 
                        hInstance, 
                        NULL
                        );
    
    if (!hWnd)
    {
        return FALSE;
    }
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent, err = 0;
    PAINTSTRUCT ps;
    HDC hdc;
    
    switch (message) 
    {
    case WM_COMMAND:
        {
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                {
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
                }
                break;
            case IDM_EXIT:
                {
                    DestroyWindow(hWnd);
                }
                break;
            default:
                {
                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
                break;
            }
        }
        break;

    case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code here...
            RECT rt;
            GetClientRect(hWnd, &rt);
            DrawText(   hdc, 
                        g_szLastMessageDesc, 
                        strlen(g_szLastMessageDesc), 
                        &rt, 
                        DT_LEFT | DT_WORDBREAK
                        );
            EndPaint(hWnd, &ps);
	    err = ReleaseDC(hWnd, hdc);
        }
        break;

    case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;

    default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    
    CheckRNMessages(hWnd);

    return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            return TRUE;
        }
        break;

    case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
        }
        break;
    }
    return FALSE;
}

//========================================================================
//	CheckRNMessages - Check for messages from RealArcade
//
//	Currently only responds to RealArcade heartbeat ping
//
//	This allows RealArcade to keep in touch with the game and tell if it 
//	has hung. RealArcade periodically sends a "PING" message and expects 
//	to get a "PONG" message back to tell it's alive. Otherwise, after a few
//	minutes of silence RealArcade will kill the "game" assuming that it has
//	hung.
//
//	This is not an high priority message in the sense that you don't need
//	to check for it every tick of the game (frame, etc). You must put this 
//	PING/PONG checking somewhere that will get checked at least every 30 seconds
//	since RealArcade will kill you if you quit responding.
//========================================================================
HRESULT CheckRNMessages(const HWND hwndGame)
{
    HRESULT hResult = S_OK;
    IRNGCMessage * pMessage = NULL;
    RNGC_MESSAGE_TYPE nType = RNGC_RESPONSE_END;
    UINT32 nSizeOfBuffer = MAX_MESSAGESTRING;
    
    if (g_pConsole == NULL)
    {
        return E_FAIL;
    }
    
    do 
    {
        hResult = g_pConsole->GetNextMessage(&nType, &pMessage, 0);
        
        if (S_OK == hResult)
        {
            // get a text version of the message so we can display it
            RNGetMessageDescription(	g_pConsole,
                                        nType, 
                                        pMessage, 
                                        g_szLastMessageDesc, 
                                        &nSizeOfBuffer
                                        );
            switch(nType) 
            {
            case RNGC_REQUEST_PING:
                {
                    // RealArcade game heartbeat check
                    // send a response pong
 		    TRACE("CheckRNMessages: %s\n", g_szLastMessageDesc);

		    g_pConsole->PostMessage(RNGC_RESPONSE_PONG, NULL);

		    TRACE("CheckRNMessages: Posting PONG message in reply.\n");
                }
                break;
                
            default:
                // All other RealArcade messages...
		TRACE("CheckRNMessages: %s\n", g_szLastMessageDesc);
		break;
            }

            // paint the new message
	    // NOTE: The InvalidateRect call is only needed for the SimpleSample app
	    //       in order for the WM_PAINT message to properly paint the 
	    //		 last message in the window. For a more general game
	    //		 there would be no reason to call InvalidateRect.
            InvalidateRect(hwndGame, NULL, TRUE);
        }
        
        RNGC_RELEASE(pMessage);
        
    } while(hResult != RNGC_QUEUE_EMPTY);
    
    return S_OK;
}
