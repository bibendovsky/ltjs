/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sun Jan 21 17:28:12 2001
 */
/* Compiler settings for .\pub/irngameconsole.idl:
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

#ifndef __irngameconsole_h__
#define __irngameconsole_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IRNGameConsole_FWD_DEFINED__
#define __IRNGameConsole_FWD_DEFINED__
typedef interface IRNGameConsole IRNGameConsole;
#endif 	/* __IRNGameConsole_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "irngcmessage.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_irngameconsole_0000 */
/* [local] */ 

typedef 
enum ConsoleType
    {	RNGC_DEMO_CONSOLE	= 0,
	RNGC_PLUS_CONSOLE	= RNGC_DEMO_CONSOLE + 1,
	RNGC_SUBSCRIPTION_CONSOLE	= RNGC_PLUS_CONSOLE + 1,
	RNGC_OEM_CONSOLE	= RNGC_SUBSCRIPTION_CONSOLE + 1
    }	RNGC_CONSOLE_TYPE;

typedef 
enum ConsoleErrorTypes
    {	RNGC_ERROR_SYSTEM	= 0,
	RNGC_ERROR_INTERNAL	= RNGC_ERROR_SYSTEM + 1,
	RNGC_ERROR_CONSOLE	= RNGC_ERROR_INTERNAL + 1,
	RNGC_ERROR_GAME	= RNGC_ERROR_CONSOLE + 1,
	RNGC_ERROR_ENGINE	= RNGC_ERROR_GAME + 1
    }	RNGC_CONSOLE_ERROR_TYPE;

typedef 
enum ConsoleErrorSeverity
    {	RNGC_SEVERITY_CRITICAL	= 0,
	RNGC_SEVERITY_HIGH	= RNGC_SEVERITY_CRITICAL + 1,
	RNGC_SEVERITY_MEDUIM	= RNGC_SEVERITY_HIGH + 1,
	RNGC_SEVERITY_LOW	= RNGC_SEVERITY_MEDUIM + 1,
	RNGC_SEVERITY_WARNING	= RNGC_SEVERITY_LOW + 1,
	RNGC_SEVERITY_INFO	= RNGC_SEVERITY_WARNING + 1,
	RNGC_SEVERITY_LOGGED	= RNGC_SEVERITY_INFO + 1
    }	RNGC_CONSOLE_ERROR_SEVERITY;

typedef 
enum GameStartupMode
    {	RNGC_START_GAME_SINGLE_PLAYER	= 0,
	RNGC_START_GAME_MULTI_PLAYER	= RNGC_START_GAME_SINGLE_PLAYER + 1,
	RNGC_START_GAME_SINGLE_TOURNAMENT	= RNGC_START_GAME_MULTI_PLAYER + 1,
	RNGC_START_GAME_MULTI_TOURNAMENT	= RNGC_START_GAME_SINGLE_TOURNAMENT + 1
    }	RNGC_GAME_START_MODE;

typedef 
enum StatsRequestTypes
    {	RNGC_STATS_REQUEST_METASTAT	= 0,
	RNGC_STATS_REQUEST_GAME	= RNGC_STATS_REQUEST_METASTAT + 1,
	RNGC_STATS_REQUEST_TOURNAMENT	= RNGC_STATS_REQUEST_GAME + 1,
	RNGC_STATS_REQUEST_ALL_PLAYERS	= RNGC_STATS_REQUEST_TOURNAMENT + 1,
	RNGC_STATS_REQUEST_TOP_X_STATS	= RNGC_STATS_REQUEST_ALL_PLAYERS + 1
    }	RNGC_STATS_REQUEST_TYPES;

typedef 
enum StatsStatusTypes
    {	RNGC_STATS_STATUS1	= 0,
	RNGC_STATS_STATUS2	= RNGC_STATS_STATUS1 + 1,
	RNGC_STATS_STATUS3	= RNGC_STATS_STATUS2 + 1
    }	RNGC_STATS_STATUS_TYPES;

typedef 
enum UpdateTypes
    {	RNGC_UPDATED_TO_SERVER	= 0,
	RNGC_UPDATED_TYPE2	= RNGC_UPDATED_TO_SERVER + 1,
	RNGC_UPDATED_TYPE3	= RNGC_UPDATED_TYPE2 + 1
    }	RNGC_UPDATE_TYPE;

typedef 
enum MessageTypes
    {	RNGC_MESSAGE_TYPE_BEGIN	= 0,
	RNGC_NOTIFY_BEGIN	= 1,
	RNGC_NOTIFY_GAME_EXITING	= RNGC_NOTIFY_BEGIN + 1,
	RNGC_NOTIFY_CONSOLE_EXITING	= RNGC_NOTIFY_GAME_EXITING + 1,
	RNGC_NOTIFY_PAUSED	= RNGC_NOTIFY_CONSOLE_EXITING + 1,
	RNGC_NOTIFY_RESUMED	= RNGC_NOTIFY_PAUSED + 1,
	RNGC_NOTIFY_RESTARTED	= RNGC_NOTIFY_RESUMED + 1,
	RNGC_NOTIFY_ACTIVATED	= RNGC_NOTIFY_RESTARTED + 1,
	RNGC_NOTIFY_DEACTIVATED	= RNGC_NOTIFY_ACTIVATED + 1,
	RNGC_NOTIFY_END	= 2047,
	RNGC_DO_BEGIN	= 2048,
	RNGC_DO_ACTIVATE	= RNGC_DO_BEGIN + 1,
	RNGC_DO_PAUSE	= RNGC_DO_ACTIVATE + 1,
	RNGC_DO_RESUME	= RNGC_DO_PAUSE + 1,
	RNGC_DO_RESTART	= RNGC_DO_RESUME + 1,
	RNGC_DO_GAME_EXIT	= RNGC_DO_RESTART + 1,
	RNGC_DO_REPORT_STATS	= RNGC_DO_GAME_EXIT + 1,
	RNGC_DO_END	= 4095,
	RNGC_INFO_BEGIN	= 4096,
	RNGC_INFO_PLAYER_ENTERED	= RNGC_INFO_BEGIN + 1,
	RNGC_INFO_PLAYER_EXITED	= RNGC_INFO_PLAYER_ENTERED + 1,
	RNGC_INFO_MULTIPLAYER_STATE_CHANGED	= RNGC_INFO_PLAYER_EXITED + 1,
	RNGC_INFO_ERROR	= RNGC_INFO_MULTIPLAYER_STATE_CHANGED + 1,
	RNGC_INFO_INSTANT_MESSAGE	= RNGC_INFO_ERROR + 1,
	RNGC_INFO_DISPLAY_MSG	= RNGC_INFO_INSTANT_MESSAGE + 1,
	RNGC_INFO_MUSIC_LIST_CHANGED	= RNGC_INFO_DISPLAY_MSG + 1,
	RNGC_INFO_STATS_UPDATE	= RNGC_INFO_MUSIC_LIST_CHANGED + 1,
	RNGC_INFO_END	= 6143,
	RNGC_REQUEST_BEGIN	= 6144,
	RNGC_REQUEST_PING	= RNGC_REQUEST_BEGIN + 1,
	RNGC_REQUEST_CONSOLE_SETTINGS	= RNGC_REQUEST_PING + 1,
	RNGC_REQUEST_START_GAME	= RNGC_REQUEST_CONSOLE_SETTINGS + 1,
	RNGC_REQUEST_VALIDATE	= RNGC_REQUEST_START_GAME + 1,
	RNGC_REQUEST_NAV_TO_URL	= RNGC_REQUEST_VALIDATE + 1,
	RNGC_REQUEST_STATS	= RNGC_REQUEST_NAV_TO_URL + 1,
	RNGC_REQUEST_PROCESS_STATS	= RNGC_REQUEST_STATS + 1,
	RNGC_REQUEST_REPORT_SPONSOR_DATA	= RNGC_REQUEST_PROCESS_STATS + 1,
	RNGC_REQUEST_SYS_INFO	= RNGC_REQUEST_REPORT_SPONSOR_DATA + 1,
	RNGC_REQUEST_PERF_INFO	= RNGC_REQUEST_SYS_INFO + 1,
	RNGC_REQUEST_PAUSE_CONFIG	= RNGC_REQUEST_PERF_INFO + 1,
	RNGC_REQUEST_PLAYER_PROFILE	= RNGC_REQUEST_PAUSE_CONFIG + 1,
	RNGC_REQUEST_SPONSOR_INFO	= RNGC_REQUEST_PLAYER_PROFILE + 1,
	RNGC_REQUEST_STATS_FREQ	= RNGC_REQUEST_SPONSOR_INFO + 1,
	RNGC_REQUEST_URLS	= RNGC_REQUEST_STATS_FREQ + 1,
	RNGC_REQUEST_SAVED_GAME_LIST	= RNGC_REQUEST_URLS + 1,
	RNGC_REQUEST_SAVED_GAME_INFO	= RNGC_REQUEST_SAVED_GAME_LIST + 1,
	RNGC_REQUEST_ADD_SAVED_GAME	= RNGC_REQUEST_SAVED_GAME_INFO + 1,
	RNGC_REQUEST_DELETE_SAVED_GAME	= RNGC_REQUEST_ADD_SAVED_GAME + 1,
	RNGC_REQUEST_MUSIC_LIST_NAMES	= RNGC_REQUEST_DELETE_SAVED_GAME + 1,
	RNGC_REQUEST_MUSIC_LIST	= RNGC_REQUEST_MUSIC_LIST_NAMES + 1,
	RNGC_REQUEST_BUDDY_LIST	= RNGC_REQUEST_MUSIC_LIST + 1,
	RNGC_REQUEST_USER_ACTIVITY	= RNGC_REQUEST_BUDDY_LIST + 1,
	RNGC_REQUEST_PAUSE_ACTIVITY	= RNGC_REQUEST_USER_ACTIVITY + 1,
	RNGC_REQUEST_JOIN_GAME	= RNGC_REQUEST_PAUSE_ACTIVITY + 1,
	RNGC_REQUEST_STOP_GAMEPLAY	= RNGC_REQUEST_JOIN_GAME + 1,
	RNGC_REQUEST_NETWORK_STATUS	= RNGC_REQUEST_STOP_GAMEPLAY + 1,
	RNGC_REQUEST_END	= 8191,
	RNGC_RESPONSE_BEGIN	= 8192,
	RNGC_RESPONSE_PONG	= RNGC_RESPONSE_BEGIN + 1,
	RNGC_RESPONSE_CONSOLE_SETTINGS	= RNGC_RESPONSE_PONG + 1,
	RNGC_RESPONSE_START_GAME	= RNGC_RESPONSE_CONSOLE_SETTINGS + 1,
	RNGC_RESPONSE_VALIDATE	= RNGC_RESPONSE_START_GAME + 1,
	RNGC_RESPONSE_NAV_TO_URL	= RNGC_RESPONSE_VALIDATE + 1,
	RNGC_RESPONSE_STATS	= RNGC_RESPONSE_NAV_TO_URL + 1,
	RNGC_RESPONSE_PROCESS_STATS	= RNGC_RESPONSE_STATS + 1,
	RNGC_RESPONSE_REPORT_SPONSOR_DATA	= RNGC_RESPONSE_PROCESS_STATS + 1,
	RNGC_RESPONSE_SYS_INFO	= RNGC_RESPONSE_REPORT_SPONSOR_DATA + 1,
	RNGC_RESPONSE_PERF_INFO	= RNGC_RESPONSE_SYS_INFO + 1,
	RNGC_RESPONSE_PAUSE_CONFIG	= RNGC_RESPONSE_PERF_INFO + 1,
	RNGC_RESPONSE_PLAYER_PROFILE	= RNGC_RESPONSE_PAUSE_CONFIG + 1,
	RNGC_RESPONSE_SPONSOR_INFO	= RNGC_RESPONSE_PLAYER_PROFILE + 1,
	RNGC_RESPONSE_STATS_FREQ	= RNGC_RESPONSE_SPONSOR_INFO + 1,
	RNGC_RESPONSE_URLS	= RNGC_RESPONSE_STATS_FREQ + 1,
	RNGC_RESPONSE_SAVED_GAME_LIST	= RNGC_RESPONSE_URLS + 1,
	RNGC_RESPONSE_SAVED_GAME_INFO	= RNGC_RESPONSE_SAVED_GAME_LIST + 1,
	RNGC_RESPONSE_ADD_SAVED_GAME	= RNGC_RESPONSE_SAVED_GAME_INFO + 1,
	RNGC_RESPONSE_DELETE_SAVED_GAME	= RNGC_RESPONSE_ADD_SAVED_GAME + 1,
	RNGC_RESPONSE_MUSIC_LIST_NAMES	= RNGC_RESPONSE_DELETE_SAVED_GAME + 1,
	RNGC_RESPONSE_MUSIC_LIST	= RNGC_RESPONSE_MUSIC_LIST_NAMES + 1,
	RNGC_RESPONSE_BUDDY_LIST	= RNGC_RESPONSE_MUSIC_LIST + 1,
	RNGC_RESPONSE_USER_ACTIVITY	= RNGC_RESPONSE_BUDDY_LIST + 1,
	RNGC_RESPONSE_PAUSE_ACTIVITY	= RNGC_RESPONSE_USER_ACTIVITY + 1,
	RNGC_RESPONSE_JOIN_GAME	= RNGC_RESPONSE_PAUSE_ACTIVITY + 1,
	RNGC_RESPONSE_STOP_GAMEPLAY	= RNGC_RESPONSE_JOIN_GAME + 1,
	RNGC_RESPONSE_NETWORK_STATUS	= RNGC_RESPONSE_STOP_GAMEPLAY + 1,
	RNGC_RESPONSE_END	= 10239,
	RNGC_MESSAGE_TYPE_END	= 0xffffffff
    }	RNGC_MESSAGE_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_irngameconsole_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_irngameconsole_0000_v0_0_s_ifspec;

#ifndef __IRNGameConsole_INTERFACE_DEFINED__
#define __IRNGameConsole_INTERFACE_DEFINED__

/* interface IRNGameConsole */
/* [helpstring][unique][uuid][object] */ 


EXTERN_C const IID IID_IRNGameConsole;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEE9C325-3E00-11d4-823D-00D0B74C5265")
    IRNGameConsole : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PostMessage( 
            /* [in] */ RNGC_MESSAGE_TYPE nType,
            /* [unique][in] */ IRNGCMessage __RPC_FAR *pMessage) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SendMessage( 
            /* [in] */ RNGC_MESSAGE_TYPE nType,
            /* [unique][in] */ IRNGCMessage __RPC_FAR *pMessageToSend,
            /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessageResponse,
            /* [in] */ long nTimeout) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetNumMessages( 
            /* [ref][retval][out] */ long __RPC_FAR *pNumMessages) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetNextMessage( 
            /* [ref][out] */ RNGC_MESSAGE_TYPE __RPC_FAR *pType,
            /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
            /* [in] */ long nTimeout) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSpecificMessage( 
            /* [in] */ RNGC_MESSAGE_TYPE nRequestedType,
            /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
            /* [in] */ long nTimeout) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMessageInRange( 
            /* [in] */ RNGC_MESSAGE_TYPE nFilterMin,
            /* [in] */ RNGC_MESSAGE_TYPE nFilterMax,
            /* [ref][out] */ RNGC_MESSAGE_TYPE __RPC_FAR *pType,
            /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
            /* [in] */ long nTimeout) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRNGameConsoleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRNGameConsole __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRNGameConsole __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRNGameConsole __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PostMessage )( 
            IRNGameConsole __RPC_FAR * This,
            /* [in] */ RNGC_MESSAGE_TYPE nType,
            /* [unique][in] */ IRNGCMessage __RPC_FAR *pMessage);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendMessage )( 
            IRNGameConsole __RPC_FAR * This,
            /* [in] */ RNGC_MESSAGE_TYPE nType,
            /* [unique][in] */ IRNGCMessage __RPC_FAR *pMessageToSend,
            /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessageResponse,
            /* [in] */ long nTimeout);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNumMessages )( 
            IRNGameConsole __RPC_FAR * This,
            /* [ref][retval][out] */ long __RPC_FAR *pNumMessages);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextMessage )( 
            IRNGameConsole __RPC_FAR * This,
            /* [ref][out] */ RNGC_MESSAGE_TYPE __RPC_FAR *pType,
            /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
            /* [in] */ long nTimeout);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSpecificMessage )( 
            IRNGameConsole __RPC_FAR * This,
            /* [in] */ RNGC_MESSAGE_TYPE nRequestedType,
            /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
            /* [in] */ long nTimeout);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMessageInRange )( 
            IRNGameConsole __RPC_FAR * This,
            /* [in] */ RNGC_MESSAGE_TYPE nFilterMin,
            /* [in] */ RNGC_MESSAGE_TYPE nFilterMax,
            /* [ref][out] */ RNGC_MESSAGE_TYPE __RPC_FAR *pType,
            /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
            /* [in] */ long nTimeout);
        
        END_INTERFACE
    } IRNGameConsoleVtbl;

    interface IRNGameConsole
    {
        CONST_VTBL struct IRNGameConsoleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRNGameConsole_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRNGameConsole_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRNGameConsole_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRNGameConsole_PostMessage(This,nType,pMessage)	\
    (This)->lpVtbl -> PostMessage(This,nType,pMessage)

#define IRNGameConsole_SendMessage(This,nType,pMessageToSend,ppMessageResponse,nTimeout)	\
    (This)->lpVtbl -> SendMessage(This,nType,pMessageToSend,ppMessageResponse,nTimeout)

#define IRNGameConsole_GetNumMessages(This,pNumMessages)	\
    (This)->lpVtbl -> GetNumMessages(This,pNumMessages)

#define IRNGameConsole_GetNextMessage(This,pType,ppMessage,nTimeout)	\
    (This)->lpVtbl -> GetNextMessage(This,pType,ppMessage,nTimeout)

#define IRNGameConsole_GetSpecificMessage(This,nRequestedType,ppMessage,nTimeout)	\
    (This)->lpVtbl -> GetSpecificMessage(This,nRequestedType,ppMessage,nTimeout)

#define IRNGameConsole_GetMessageInRange(This,nFilterMin,nFilterMax,pType,ppMessage,nTimeout)	\
    (This)->lpVtbl -> GetMessageInRange(This,nFilterMin,nFilterMax,pType,ppMessage,nTimeout)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGameConsole_PostMessage_Proxy( 
    IRNGameConsole __RPC_FAR * This,
    /* [in] */ RNGC_MESSAGE_TYPE nType,
    /* [unique][in] */ IRNGCMessage __RPC_FAR *pMessage);


void __RPC_STUB IRNGameConsole_PostMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGameConsole_SendMessage_Proxy( 
    IRNGameConsole __RPC_FAR * This,
    /* [in] */ RNGC_MESSAGE_TYPE nType,
    /* [unique][in] */ IRNGCMessage __RPC_FAR *pMessageToSend,
    /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessageResponse,
    /* [in] */ long nTimeout);


void __RPC_STUB IRNGameConsole_SendMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGameConsole_GetNumMessages_Proxy( 
    IRNGameConsole __RPC_FAR * This,
    /* [ref][retval][out] */ long __RPC_FAR *pNumMessages);


void __RPC_STUB IRNGameConsole_GetNumMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGameConsole_GetNextMessage_Proxy( 
    IRNGameConsole __RPC_FAR * This,
    /* [ref][out] */ RNGC_MESSAGE_TYPE __RPC_FAR *pType,
    /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
    /* [in] */ long nTimeout);


void __RPC_STUB IRNGameConsole_GetNextMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGameConsole_GetSpecificMessage_Proxy( 
    IRNGameConsole __RPC_FAR * This,
    /* [in] */ RNGC_MESSAGE_TYPE nRequestedType,
    /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
    /* [in] */ long nTimeout);


void __RPC_STUB IRNGameConsole_GetSpecificMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRNGameConsole_GetMessageInRange_Proxy( 
    IRNGameConsole __RPC_FAR * This,
    /* [in] */ RNGC_MESSAGE_TYPE nFilterMin,
    /* [in] */ RNGC_MESSAGE_TYPE nFilterMax,
    /* [ref][out] */ RNGC_MESSAGE_TYPE __RPC_FAR *pType,
    /* [ref][out] */ IRNGCMessage __RPC_FAR *__RPC_FAR *ppMessage,
    /* [in] */ long nTimeout);


void __RPC_STUB IRNGameConsole_GetMessageInRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRNGameConsole_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
