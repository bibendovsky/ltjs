/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Dec 13 13:22:19 2000
 */
/* Compiler settings for C:\raroot\src\rngcgdll\pub\irngcclientengine.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __irngcclientengine_h__
#define __irngcclientengine_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IRNGCClientEngine_FWD_DEFINED__
#define __IRNGCClientEngine_FWD_DEFINED__
typedef interface IRNGCClientEngine IRNGCClientEngine;
#endif 	/* __IRNGCClientEngine_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "irngameconsole.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IRNGCClientEngine_INTERFACE_DEFINED__
#define __IRNGCClientEngine_INTERFACE_DEFINED__

/* interface IRNGCClientEngine */
/* [helpstring][unique][uuid][object] */ 


EXTERN_C const IID IID_IRNGCClientEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEE9C326-3E00-11d4-823D-00D0B74C5265")
    IRNGCClientEngine : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InitEngine( 
            /* [in] */ BSTR bstrGameID) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CleanupEngine( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CheckConsoleStatus( 
            /* [retval][ref][out] */ ULONG __RPC_FAR *bRunning) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetErrorMessage( 
            /* [in] */ HRESULT hrError,
            /* [retval][ref][out] */ BSTR __RPC_FAR *pbstrMessage) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMessageDescription( 
            /* [in] */ RNGC_MESSAGE_TYPE nType,
            /* [in] */ IRNGCMessage __RPC_FAR *pMessage,
            /* [retval][ref][out] */ BSTR __RPC_FAR *pbstrMessage) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRNGCClientEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRNGCClientEngine __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRNGCClientEngine __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRNGCClientEngine __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitEngine )( 
            IRNGCClientEngine __RPC_FAR * This,
            /* [in] */ BSTR bstrGameID);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CleanupEngine )( 
            IRNGCClientEngine __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CheckConsoleStatus )( 
            IRNGCClientEngine __RPC_FAR * This,
            /* [retval][ref][out] */ ULONG __RPC_FAR *bRunning);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorMessage )( 
            IRNGCClientEngine __RPC_FAR * This,
            /* [in] */ HRESULT hrError,
            /* [retval][ref][out] */ BSTR __RPC_FAR *pbstrMessage);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMessageDescription )( 
            IRNGCClientEngine __RPC_FAR * This,
            /* [in] */ RNGC_MESSAGE_TYPE nType,
            /* [in] */ IRNGCMessage __RPC_FAR *pMessage,
            /* [retval][ref][out] */ BSTR __RPC_FAR *pbstrMessage);
        
        END_INTERFACE
    } IRNGCClientEngineVtbl;

    interface IRNGCClientEngine
    {
        CONST_VTBL struct IRNGCClientEngineVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRNGCClientEngine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRNGCClientEngine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRNGCClientEngine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRNGCClientEngine_InitEngine(This,bstrGameID)	\
    (This)->lpVtbl -> InitEngine(This,bstrGameID)

#define IRNGCClientEngine_CleanupEngine(This)	\
    (This)->lpVtbl -> CleanupEngine(This)

#define IRNGCClientEngine_CheckConsoleStatus(This,bRunning)	\
    (This)->lpVtbl -> CheckConsoleStatus(This,bRunning)

#define IRNGCClientEngine_GetErrorMessage(This,hrError,pbstrMessage)	\
    (This)->lpVtbl -> GetErrorMessage(This,hrError,pbstrMessage)

#define IRNGCClientEngine_GetMessageDescription(This,nType,pMessage,pbstrMessage)	\
    (This)->lpVtbl -> GetMessageDescription(This,nType,pMessage,pbstrMessage)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCClientEngine_InitEngine_Proxy( 
    IRNGCClientEngine __RPC_FAR * This,
    /* [in] */ BSTR bstrGameID);


void __RPC_STUB IRNGCClientEngine_InitEngine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCClientEngine_CleanupEngine_Proxy( 
    IRNGCClientEngine __RPC_FAR * This);


void __RPC_STUB IRNGCClientEngine_CleanupEngine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCClientEngine_CheckConsoleStatus_Proxy( 
    IRNGCClientEngine __RPC_FAR * This,
    /* [retval][ref][out] */ ULONG __RPC_FAR *bRunning);


void __RPC_STUB IRNGCClientEngine_CheckConsoleStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCClientEngine_GetErrorMessage_Proxy( 
    IRNGCClientEngine __RPC_FAR * This,
    /* [in] */ HRESULT hrError,
    /* [retval][ref][out] */ BSTR __RPC_FAR *pbstrMessage);


void __RPC_STUB IRNGCClientEngine_GetErrorMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCClientEngine_GetMessageDescription_Proxy( 
    IRNGCClientEngine __RPC_FAR * This,
    /* [in] */ RNGC_MESSAGE_TYPE nType,
    /* [in] */ IRNGCMessage __RPC_FAR *pMessage,
    /* [retval][ref][out] */ BSTR __RPC_FAR *pbstrMessage);


void __RPC_STUB IRNGCClientEngine_GetMessageDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRNGCClientEngine_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
