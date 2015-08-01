/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sun Jan 21 17:28:11 2001
 */
/* Compiler settings for .\pub/irngcmessage.idl:
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

#ifndef __irngcmessage_h__
#define __irngcmessage_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IRNGCMessage_FWD_DEFINED__
#define __IRNGCMessage_FWD_DEFINED__
typedef interface IRNGCMessage IRNGCMessage;
#endif 	/* __IRNGCMessage_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_irngcmessage_0000 */
/* [local] */ 





extern RPC_IF_HANDLE __MIDL_itf_irngcmessage_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_irngcmessage_0000_v0_0_s_ifspec;

#ifndef __IRNGCMessage_INTERFACE_DEFINED__
#define __IRNGCMessage_INTERFACE_DEFINED__

/* interface IRNGCMessage */
/* [helpstring][unique][uuid][object] */ 


EXTERN_C const IID IID_IRNGCMessage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEE9C329-3E00-11d4-823D-00D0B74C5265")
    IRNGCMessage : public IUnknown
    {
    public:
        virtual /* [propput][helpstring] */ HRESULT STDMETHODCALLTYPE put_NodeName( 
            /* [ref][in] */ BSTR bstrName) = 0;
        
        virtual /* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE get_NodeName( 
            /* [ref][retval][out] */ BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual /* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE get_ValueCount( 
            /* [ref][retval][out] */ long __RPC_FAR *nNumValues) = 0;
        
        virtual /* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE get_NodeCount( 
            /* [ref][retval][out] */ long __RPC_FAR *nNumNodes) = 0;
        
        virtual /* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [in] */ long nValueNum,
            /* [ref][out] */ BSTR __RPC_FAR *pbstrValueName,
            /* [ref][out] */ VARIANTARG __RPC_FAR *pValue) = 0;
        
        virtual /* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE get_Node( 
            /* [in] */ long nNodeNum,
            /* [ref][retval][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppNode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddValue( 
            /* [ref][in] */ BSTR bstrName,
            /* [in] */ VARIANTARG varValue) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddNode( 
            /* [ref][in] */ BSTR bstrName,
            /* [in] */ IRNGCMessage __RPC_FAR *pNode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveValue( 
            /* [ref][in] */ BSTR bstrName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveNode( 
            /* [ref][in] */ BSTR bstrName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FindValue( 
            /* [ref][in] */ BSTR bstrName,
            /* [ref][retval][out] */ VARIANTARG __RPC_FAR *pvarValue) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FindNode( 
            /* [ref][in] */ BSTR bstrName,
            /* [ref][retval][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppChild) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ClearAll( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRNGCMessageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRNGCMessage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRNGCMessage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRNGCMessage __RPC_FAR * This);
        
        /* [propput][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_NodeName )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][in] */ BSTR bstrName);
        
        /* [propget][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NodeName )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][retval][out] */ BSTR __RPC_FAR *pbstrName);
        
        /* [propget][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ValueCount )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][retval][out] */ long __RPC_FAR *nNumValues);
        
        /* [propget][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NodeCount )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][retval][out] */ long __RPC_FAR *nNumNodes);
        
        /* [propget][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            IRNGCMessage __RPC_FAR * This,
            /* [in] */ long nValueNum,
            /* [ref][out] */ BSTR __RPC_FAR *pbstrValueName,
            /* [ref][out] */ VARIANTARG __RPC_FAR *pValue);
        
        /* [propget][helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Node )( 
            IRNGCMessage __RPC_FAR * This,
            /* [in] */ long nNodeNum,
            /* [ref][retval][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppNode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddValue )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][in] */ BSTR bstrName,
            /* [in] */ VARIANTARG varValue);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddNode )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][in] */ BSTR bstrName,
            /* [in] */ IRNGCMessage __RPC_FAR *pNode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveValue )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][in] */ BSTR bstrName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveNode )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][in] */ BSTR bstrName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindValue )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][in] */ BSTR bstrName,
            /* [ref][retval][out] */ VARIANTARG __RPC_FAR *pvarValue);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindNode )( 
            IRNGCMessage __RPC_FAR * This,
            /* [ref][in] */ BSTR bstrName,
            /* [ref][retval][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppChild);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearAll )( 
            IRNGCMessage __RPC_FAR * This);
        
        END_INTERFACE
    } IRNGCMessageVtbl;

    interface IRNGCMessage
    {
        CONST_VTBL struct IRNGCMessageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRNGCMessage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRNGCMessage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRNGCMessage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRNGCMessage_put_NodeName(This,bstrName)	\
    (This)->lpVtbl -> put_NodeName(This,bstrName)

#define IRNGCMessage_get_NodeName(This,pbstrName)	\
    (This)->lpVtbl -> get_NodeName(This,pbstrName)

#define IRNGCMessage_get_ValueCount(This,nNumValues)	\
    (This)->lpVtbl -> get_ValueCount(This,nNumValues)

#define IRNGCMessage_get_NodeCount(This,nNumNodes)	\
    (This)->lpVtbl -> get_NodeCount(This,nNumNodes)

#define IRNGCMessage_get_Value(This,nValueNum,pbstrValueName,pValue)	\
    (This)->lpVtbl -> get_Value(This,nValueNum,pbstrValueName,pValue)

#define IRNGCMessage_get_Node(This,nNodeNum,ppNode)	\
    (This)->lpVtbl -> get_Node(This,nNodeNum,ppNode)

#define IRNGCMessage_AddValue(This,bstrName,varValue)	\
    (This)->lpVtbl -> AddValue(This,bstrName,varValue)

#define IRNGCMessage_AddNode(This,bstrName,pNode)	\
    (This)->lpVtbl -> AddNode(This,bstrName,pNode)

#define IRNGCMessage_RemoveValue(This,bstrName)	\
    (This)->lpVtbl -> RemoveValue(This,bstrName)

#define IRNGCMessage_RemoveNode(This,bstrName)	\
    (This)->lpVtbl -> RemoveNode(This,bstrName)

#define IRNGCMessage_FindValue(This,bstrName,pvarValue)	\
    (This)->lpVtbl -> FindValue(This,bstrName,pvarValue)

#define IRNGCMessage_FindNode(This,bstrName,ppChild)	\
    (This)->lpVtbl -> FindNode(This,bstrName,ppChild)

#define IRNGCMessage_ClearAll(This)	\
    (This)->lpVtbl -> ClearAll(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propput][helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_put_NodeName_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][in] */ BSTR bstrName);


void __RPC_STUB IRNGCMessage_put_NodeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_get_NodeName_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][retval][out] */ BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IRNGCMessage_get_NodeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_get_ValueCount_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][retval][out] */ long __RPC_FAR *nNumValues);


void __RPC_STUB IRNGCMessage_get_ValueCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_get_NodeCount_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][retval][out] */ long __RPC_FAR *nNumNodes);


void __RPC_STUB IRNGCMessage_get_NodeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_get_Value_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [in] */ long nValueNum,
    /* [ref][out] */ BSTR __RPC_FAR *pbstrValueName,
    /* [ref][out] */ VARIANTARG __RPC_FAR *pValue);


void __RPC_STUB IRNGCMessage_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_get_Node_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [in] */ long nNodeNum,
    /* [ref][retval][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppNode);


void __RPC_STUB IRNGCMessage_get_Node_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_AddValue_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][in] */ BSTR bstrName,
    /* [in] */ VARIANTARG varValue);


void __RPC_STUB IRNGCMessage_AddValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_AddNode_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][in] */ BSTR bstrName,
    /* [in] */ IRNGCMessage __RPC_FAR *pNode);


void __RPC_STUB IRNGCMessage_AddNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_RemoveValue_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][in] */ BSTR bstrName);


void __RPC_STUB IRNGCMessage_RemoveValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_RemoveNode_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][in] */ BSTR bstrName);


void __RPC_STUB IRNGCMessage_RemoveNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_FindValue_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][in] */ BSTR bstrName,
    /* [ref][retval][out] */ VARIANTARG __RPC_FAR *pvarValue);


void __RPC_STUB IRNGCMessage_FindValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_FindNode_Proxy( 
    IRNGCMessage __RPC_FAR * This,
    /* [ref][in] */ BSTR bstrName,
    /* [ref][retval][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppChild);


void __RPC_STUB IRNGCMessage_FindNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGCMessage_ClearAll_Proxy( 
    IRNGCMessage __RPC_FAR * This);


void __RPC_STUB IRNGCMessage_ClearAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRNGCMessage_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
