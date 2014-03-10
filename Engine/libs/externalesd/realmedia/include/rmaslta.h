/****************************************************************************
 * 
 *  $Id: rmaslta.h,v 1.5 1999/01/15 16:01:02 brad Exp $
 *
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary nformation of RealNetworks, Inc, 
 *  and is licensed subject to restrictions on use and distribution.
 *
 *
 *  RealMedia Architecture Interfaces for Simulated Live Transfer Agent.
 *
 */

#ifndef _RMASLTA_H
#define _RMASLTA_H

/****************************************************************************
 * 
 *  Interface:
 * 
 *	IRMASLTA
 * 
 *  Purpose:
 * 
 *	Slta that works with RMA.  Simulates a live stream from a file.
 * 
 *  IID_IRMASLTA
 * 
 *	{00000D00-b4c8-11d0-9995-00a0248da5f0}
 * 
 */
DEFINE_GUID(IID_IRMASLTA,   0x00000D00, 0xb4c8, 0x11d0, 0x99, 
			    0x95, 0x0, 0xa0, 0x24, 0x8d, 0xa5, 0xf0);

DECLARE_INTERFACE_(IRMASLTA, IUnknown)
{
    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj) PURE;

    STDMETHOD_(ULONG32,AddRef)		(THIS) PURE;

    STDMETHOD_(ULONG32,Release)		(THIS) PURE;

    /*
     *	IRMASLTA methods
     */

    /************************************************************************
     *	Method:
     *	    IRMASLTA::Connect
     *	Purpose:
     *	    Connects the slta to a server.
     */
    STDMETHOD(Connect)	(THIS_
			    const char* host,
			    UINT16 uPort,
			    const char* username,
			    const char* passwd,
			    const char* livefile
			) PURE;

    /************************************************************************
    * Method:
    *	    IRMASLTA::SetTAC
    * Purpose:
    *	    Set the TAC info for the stream. This method MUST be called
    *	before Encode to have any effect.
    */
    STDMETHOD(SetTAC)	(THIS_
			    const char* Title,
			    const char* Author,
			    const char* Copyright) PURE;

    /************************************************************************
    *	Method:
    *	    IRMASLTA:Encode
    *	Purpose:
    *	    Start encoding the file to the server.
    */
    STDMETHOD(Encode)	
			(THIS_
			    const char* filename
			) PURE;

    /************************************************************************
    *	Method:
    *	    IRMASLTA:Disconnect
    *	Purpose:
    *	    Disconnect the slta from the server.
    */
    STDMETHOD(Disconnect)   (THIS) PURE;


    /************************************************************************
    *	Method:
    *	    IRMASLTA::SetTargetBandwidth
    *	Purpose:
    *	    Sets the target bw for rule subscription.
    */
    STDMETHOD(SetTargetBandwidth)   (THIS_
					UINT32 ulTargetBW)  PURE;


};

#endif
