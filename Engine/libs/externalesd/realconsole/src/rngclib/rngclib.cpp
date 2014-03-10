/* 
 * $Id: rngclib.cpp,v 1.7 2000/10/11 00:12:11 ingalls Exp $
 *
 * ----------------------------------------------------------
 * 
 * Author:   Epcot
 *   RealNetworks, Inc (C) 1999-2000, All rights reserved
 *
 *
 */

#include <windows.h>
#include <objbase.h>
#include "rngclib.h"

HRESULT RNInitConsole(GUID guidGameID,
                      BOOL bInitMultiThreaded,
                      IRNGameConsole** ppGameConsole
                      )
{
    HRESULT res = S_OK;
    IRNGCClientEngine * pClient = NULL;
    BSTR bstrGameID = NULL;
    LPOLESTR lpTempStr = NULL;
    
    // check and init params
    if (NULL == ppGameConsole)
    {
        return E_INVALIDARG;
    }
    *ppGameConsole = NULL;

    // initialize the COM libraries
    if (bInitMultiThreaded)
    {
        res = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    }
    else
    {
        res = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }

    // its ok if the user already has initialized com with a different setting
    if (FAILED(res) && res != RPC_E_CHANGED_MODE)
    {
        // log something
        goto RNInitConsoleCleanup;
    }

    res = ::CoCreateInstance(   CLSID_RNGCClientEngine,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_IRNGCClientEngine,
                                (void **)&pClient
                                );
    if (FAILED(res))
    {
        // log something
        goto RNInitConsoleCleanup;
    }

    // convert GUID to string
    StringFromIID(guidGameID, &lpTempStr);
    bstrGameID = SysAllocString(lpTempStr);

    res = pClient->InitEngine(bstrGameID);
    if (FAILED(res))
    {
        // log something
        goto RNInitConsoleCleanup;
    }

    res = pClient->QueryInterface(  IID_IRNGameConsole,
                                    (void **) ppGameConsole
                                    );
    if (FAILED(res))
    {
        // log something
        goto RNInitConsoleCleanup;
    }

RNInitConsoleCleanup:

    // cleanup
    RNGC_RELEASE(pClient);
    if (NULL != lpTempStr)
    {
        CoTaskMemFree(lpTempStr);
        lpTempStr = NULL;
    }
    SysFreeString(bstrGameID);

    return res;
}

HRESULT RNTermConsole(IRNGameConsole* pGameConsole)
{
    HRESULT res = S_OK;
    IRNGCClientEngine * pClient = NULL;

    // check params
    if (NULL == pGameConsole)
    {
        return E_INVALIDARG;
    }

    res = pGameConsole->QueryInterface( IID_IRNGCClientEngine,
                                        (void **)&pClient
                                        );
    if (FAILED(res))
    {
        // log something
        goto RNTermConsoleCleanup;
    }
    
    res = pClient->CleanupEngine();
    if (FAILED(res))
    {
        // log something
        goto RNTermConsoleCleanup;
    }

RNTermConsoleCleanup:        

    // cleanup
    RNGC_RELEASE(pGameConsole);
    RNGC_RELEASE(pClient);

    ::CoUninitialize();

    return res;
}

HRESULT RNGetErrorDescriptionA( IRNGameConsole * pGameConsole, 
                                HRESULT hrError, 
                                char* szBuffer, 
                                UINT32* pnSizeOfBuffer
                                )
{
    HRESULT res = S_OK;
    IRNGCClientEngine * pClient = NULL;
    BSTR bstrError = NULL;

    // check params
    if (NULL == pnSizeOfBuffer || NULL == pGameConsole)
    {
        return E_INVALIDARG;
    }

    res = pGameConsole->QueryInterface( IID_IRNGCClientEngine,
                                        (void **)&pClient
                                        );
    if (FAILED(res))
    {
        // log something
        goto RNGetErrorDescriptionACleanup;
    }
    
    res = pClient->GetErrorMessage(hrError, &bstrError);
    if (FAILED(res))
    {
        // log something
        goto RNGetErrorDescriptionACleanup;
    }

    if (SysStringLen(bstrError) > *pnSizeOfBuffer)
    {
        // add one for the null character
        res = RNGC_BUFFERTOOSMALL;
    }
    else
    {
        wcstombs(szBuffer,SAFEBSTR(bstrError),*pnSizeOfBuffer);
    }

    // return the actual[required] size
    *pnSizeOfBuffer = SysStringLen(bstrError) + 1;

RNGetErrorDescriptionACleanup:
    
    // cleanup
    SysFreeString(bstrError);
    RNGC_RELEASE(pClient);
    
    return res;
}

HRESULT RNGetErrorDescriptionW( IRNGameConsole * pGameConsole, 
                                HRESULT hrError, 
                                WCHAR* szBuffer, 
                                UINT32* pnSizeOfBuffer
                                )
{
    HRESULT res = S_OK;
    IRNGCClientEngine * pClient = NULL;
    BSTR bstrError = NULL;

    // check params
    if (NULL == pnSizeOfBuffer || NULL == pGameConsole)
    {
        return E_INVALIDARG;
    }

    res = pGameConsole->QueryInterface( IID_IRNGCClientEngine,
                                        (void **)&pClient
                                        );
    if (FAILED(res))
    {
        // log something
        goto RNGetErrorDescriptionWCleanup;
    }
    
    res = pClient->GetErrorMessage(hrError, &bstrError);
    if (FAILED(res))
    {
        // log something
        goto RNGetErrorDescriptionWCleanup;
    }

    if (SysStringByteLen(bstrError) >= *pnSizeOfBuffer)
    {
        res = RNGC_BUFFERTOOSMALL;
    }
    else
    {
        wcscpy(szBuffer, SAFEBSTR(bstrError));
    }

    // return the actual[required] size
    *pnSizeOfBuffer = SysStringByteLen(bstrError);

RNGetErrorDescriptionWCleanup:
    
    // cleanup
    SysFreeString(bstrError);
    RNGC_RELEASE(pClient);
    
    return res;
}

HRESULT RNGetMessageDescriptionA(   IRNGameConsole * pGameConsole,
                                    RNGC_MESSAGE_TYPE nType, 
                                    IRNGCMessage * pMessage, 
                                    CHAR* szBuffer, 
                                    UINT32* pnSizeOfBuffer
                                    )
{
    HRESULT res = S_OK;
    IRNGCClientEngine * pClient = NULL;
    BSTR bstrMessage = NULL;

    // check params
    if (NULL == pnSizeOfBuffer || NULL == pGameConsole)
    {
        return E_INVALIDARG;
    }

    res = pGameConsole->QueryInterface( IID_IRNGCClientEngine,
                                        (void **)&pClient
                                        );
    if (FAILED(res))
    {
        // log something
        goto RNGetMessageDescriptionACleanup;
    }
    
    res = pClient->GetMessageDescription(nType, pMessage, &bstrMessage);
    if (FAILED(res))
    {
        // log something
        goto RNGetMessageDescriptionACleanup;
    }

    if (SysStringLen(bstrMessage) > *pnSizeOfBuffer)
    {
        // add one for the null character
        res = RNGC_BUFFERTOOSMALL;
    }
    else
    {
        wcstombs(szBuffer,SAFEBSTR(bstrMessage),*pnSizeOfBuffer);
    }

    // return the actual[required] size
    *pnSizeOfBuffer = SysStringLen(bstrMessage) + 1;

RNGetMessageDescriptionACleanup:
    
    // cleanup
    SysFreeString(bstrMessage);
    RNGC_RELEASE(pClient);
    
    return res;
}

HRESULT RNGetMessageDescriptionW(   IRNGameConsole * pGameConsole,
                                    RNGC_MESSAGE_TYPE nType, 
                                    IRNGCMessage * pMessage, 
                                    WCHAR* szBuffer, 
                                    UINT32* pnSizeOfBuffer
                                    )
{
    HRESULT res = S_OK;
    IRNGCClientEngine * pClient = NULL;
    BSTR bstrMessage = NULL;

    // check params
    if (NULL == pnSizeOfBuffer || NULL == pGameConsole)
    {
        return E_INVALIDARG;
    }

    res = pGameConsole->QueryInterface( IID_IRNGCClientEngine,
                                        (void **)&pClient
                                        );
    if (FAILED(res))
    {
        // log something
        goto RNGetMessageDescriptionWCleanup;
    }
    
    res = pClient->GetMessageDescription(nType, pMessage, &bstrMessage);
    if (FAILED(res))
    {
        // log something
        goto RNGetMessageDescriptionWCleanup;
    }

    if (SysStringByteLen(bstrMessage) >= *pnSizeOfBuffer)
    {
        res = RNGC_BUFFERTOOSMALL;
    }
    else
    {
        wcscpy(szBuffer, SAFEBSTR(bstrMessage));
    }

    // return the actual[required] size
    *pnSizeOfBuffer = SysStringByteLen(bstrMessage);

RNGetMessageDescriptionWCleanup:
    
    // cleanup
    SysFreeString(bstrMessage);
    RNGC_RELEASE(pClient);
    
    return res;
}


HRESULT RNCreateMessage(IRNGCMessage** ppMessage)
{
    HRESULT res = S_OK;

    res = ::CoCreateInstance(   CLSID_RNGCClientMessage,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_IRNGCMessage,
                                (void**) ppMessage
                                );

    return res;
}

