/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Dec 13 12:13:04 2000
 */
/* Compiler settings for C:\raroot\src\rngcgdll\pub\rngcclientenginelib.idl:
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

#ifndef __rngcclientenginelib_h__
#define __rngcclientenginelib_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __RNGCClientEngine_FWD_DEFINED__
#define __RNGCClientEngine_FWD_DEFINED__

#ifdef __cplusplus
typedef class RNGCClientEngine RNGCClientEngine;
#else
typedef struct RNGCClientEngine RNGCClientEngine;
#endif /* __cplusplus */

#endif 	/* __RNGCClientEngine_FWD_DEFINED__ */


#ifndef __RNGCClientMessage_FWD_DEFINED__
#define __RNGCClientMessage_FWD_DEFINED__

#ifdef __cplusplus
typedef class RNGCClientMessage RNGCClientMessage;
#else
typedef struct RNGCClientMessage RNGCClientMessage;
#endif /* __cplusplus */

#endif 	/* __RNGCClientMessage_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "irngcclientengine.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __RNGCClientEngineLib_LIBRARY_DEFINED__
#define __RNGCClientEngineLib_LIBRARY_DEFINED__

/* library RNGCClientEngineLib */
/* [helpstring][uuid] */ 


EXTERN_C const IID LIBID_RNGCClientEngineLib;

EXTERN_C const CLSID CLSID_RNGCClientEngine;

#ifdef __cplusplus

class DECLSPEC_UUID("BEE9C321-3E00-11d4-823D-00D0B74C5265")
RNGCClientEngine;
#endif

EXTERN_C const CLSID CLSID_RNGCClientMessage;

#ifdef __cplusplus

class DECLSPEC_UUID("BEE9C322-3E00-11d4-823D-00D0B74C5265")
RNGCClientMessage;
#endif
#endif /* __RNGCClientEngineLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
