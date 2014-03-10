#pragma once
/****************************************************************************
 * 
 *  $Id: rngclib.h,v 1.7 2000/10/11 00:12:10 ingalls Exp $
 *  RealNetworks Game Console Interface
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary 
 *  information of RealNetworks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *  This file contains the definitions for the helper library
 *  for game developers working in C++;
 */

#include "rngc.h"

#ifdef __cplusplus
extern "C"{
#endif 


/**
* RNInitConsole: Initialize the client connection to the game console.
*
* @param bInitMultiThreaded [in] does the game support multiple threads
* @param ppGameConsole [out] main game interface
*
* @return S_OK on success, HRESULT for failure
* 
*/
HRESULT RNInitConsole(  GUID guidGameID,
                        BOOL bInitMultiThreaded,
                        IRNGameConsole** ppGameConsole
                        );

/**
* RNTermConsole: Shuts down the client connection and releases resources.
*
* @param pGameConsole [in] main game interface
*
* @return S_OK on success, HRESULT for failure
*
*/
HRESULT RNTermConsole(IRNGameConsole* pGameConsole);

/**
* RNGetErrorDescription: Returns the string associated with an HRESULT error code
*
* @param pGameConsole   [in] the game console interface to query
* @param hrError        [in] the HRESULT error code to look up
* @param szBuffer       [in/out] the string buffer to fill
* @param nSizeOfBuffer  [in/out] the size of the buffer to fill in BYTES, 
*                       the amount filled, or the required amount if too small
*
* @return S_OK on success, 
*         RNGC_BUFFERTOOSMALL if the buffer won't fit the message
*
*/
HRESULT RNGetErrorDescriptionA( IRNGameConsole * pGameConsole,
                                HRESULT hrError, 
                                CHAR* szBuffer, 
                                UINT32* pnSizeOfBuffer
                                );

HRESULT RNGetErrorDescriptionW( IRNGameConsole * pGameConsole,
                                HRESULT hrError, 
                                WCHAR* szBuffer, 
                                UINT32* pnSizeOfBuffer
                                );
#ifdef  _UNICODE
#define RNGetErrorDescription RNGetErrorDescriptionW
#else
#define RNGetErrorDescription RNGetErrorDescriptionA
#endif


/**
* RNGetMessageDescription: Returns the string description associated 
*                           with a specific message
*
* @param pGameConsole   [in] the game console interface to query
* @param nType          [in] the message type to look up
* @param pMessage       [in] the message specifics.  Will be dumped to buffer if not null
* @param szBuffer       [in/out] the string buffer to fill
* @param nSizeOfBuffer  [in/out] [in]the size of the buffer to fill in BYTES, 
*                                [out]the amount filled, or the required amount if too small
*
* @return S_OK on success, 
*         RNGC_BUFFERTOOSMALL if the buffer won't fit the message
*
*/
HRESULT RNGetMessageDescriptionA(   IRNGameConsole * pGameConsole,
                                    RNGC_MESSAGE_TYPE nType, 
                                    IRNGCMessage * pMessage, 
                                    CHAR* szBuffer, 
                                    UINT32* pnSizeOfBuffer
                                    );

HRESULT RNGetMessageDescriptionW(   IRNGameConsole * pGameConsole,
                                    RNGC_MESSAGE_TYPE nType, 
                                    IRNGCMessage * pMessage, 
                                    WCHAR* szBuffer, 
                                    UINT32* pnSizeOfBuffer
                                    );

#ifdef  _UNICODE
#define RNGetMessageDescription RNGetMessageDescriptionW
#else
#define RNGetMessageDescription RNGetMessageDescriptionA
#endif


/**
* RNCreateMessage: Creates a generic message object.
*
* @param ppMessage [out] the new, empty message object
*
* @return S_OK on success, HRESULT for failure
*
*/
HRESULT RNCreateMessage(IRNGCMessage** ppMessage);

#ifdef __cplusplus
}
#endif
