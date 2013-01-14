

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Mon Sep 10 13:12:15 2012
 */
/* Compiler settings for .\DecklinkFilters.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __DecklinkFilters_h_h__
#define __DecklinkFilters_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __DecklinkPushSource_FWD_DEFINED__
#define __DecklinkPushSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class DecklinkPushSource DecklinkPushSource;
#else
typedef struct DecklinkPushSource DecklinkPushSource;
#endif /* __cplusplus */

#endif 	/* __DecklinkPushSource_FWD_DEFINED__ */


#ifndef __DecklinkVideoSource_FWD_DEFINED__
#define __DecklinkVideoSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class DecklinkVideoSource DecklinkVideoSource;
#else
typedef struct DecklinkVideoSource DecklinkVideoSource;
#endif /* __cplusplus */

#endif 	/* __DecklinkVideoSource_FWD_DEFINED__ */


#ifndef __DecklinkAudioSource_FWD_DEFINED__
#define __DecklinkAudioSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class DecklinkAudioSource DecklinkAudioSource;
#else
typedef struct DecklinkAudioSource DecklinkAudioSource;
#endif /* __cplusplus */

#endif 	/* __DecklinkAudioSource_FWD_DEFINED__ */


#ifndef __CustomMemAllocator_FWD_DEFINED__
#define __CustomMemAllocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class CustomMemAllocator CustomMemAllocator;
#else
typedef struct CustomMemAllocator CustomMemAllocator;
#endif /* __cplusplus */

#endif 	/* __CustomMemAllocator_FWD_DEFINED__ */


#ifndef __DecklinkFieldSwap_FWD_DEFINED__
#define __DecklinkFieldSwap_FWD_DEFINED__

#ifdef __cplusplus
typedef class DecklinkFieldSwap DecklinkFieldSwap;
#else
typedef struct DecklinkFieldSwap DecklinkFieldSwap;
#endif /* __cplusplus */

#endif 	/* __DecklinkFieldSwap_FWD_DEFINED__ */


#ifndef __DecklinkStillSource_FWD_DEFINED__
#define __DecklinkStillSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class DecklinkStillSource DecklinkStillSource;
#else
typedef struct DecklinkStillSource DecklinkStillSource;
#endif /* __cplusplus */

#endif 	/* __DecklinkStillSource_FWD_DEFINED__ */


#ifndef __DecklinkComposite_FWD_DEFINED__
#define __DecklinkComposite_FWD_DEFINED__

#ifdef __cplusplus
typedef class DecklinkComposite DecklinkComposite;
#else
typedef struct DecklinkComposite DecklinkComposite;
#endif /* __cplusplus */

#endif 	/* __DecklinkComposite_FWD_DEFINED__ */


#ifndef __DecklinkCompositeProperties_FWD_DEFINED__
#define __DecklinkCompositeProperties_FWD_DEFINED__

#ifdef __cplusplus
typedef class DecklinkCompositeProperties DecklinkCompositeProperties;
#else
typedef struct DecklinkCompositeProperties DecklinkCompositeProperties;
#endif /* __cplusplus */

#endif 	/* __DecklinkCompositeProperties_FWD_DEFINED__ */


#ifndef __WavDest_FWD_DEFINED__
#define __WavDest_FWD_DEFINED__

#ifdef __cplusplus
typedef class WavDest WavDest;
#else
typedef struct WavDest WavDest;
#endif /* __cplusplus */

#endif 	/* __WavDest_FWD_DEFINED__ */


#ifndef __DecklinkToneSource_FWD_DEFINED__
#define __DecklinkToneSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class DecklinkToneSource DecklinkToneSource;
#else
typedef struct DecklinkToneSource DecklinkToneSource;
#endif /* __cplusplus */

#endif 	/* __DecklinkToneSource_FWD_DEFINED__ */


#ifndef __IDecklinkPushSource_FWD_DEFINED__
#define __IDecklinkPushSource_FWD_DEFINED__
typedef interface IDecklinkPushSource IDecklinkPushSource;
#endif 	/* __IDecklinkPushSource_FWD_DEFINED__ */


#ifndef __IDecklinkPushSource2_FWD_DEFINED__
#define __IDecklinkPushSource2_FWD_DEFINED__
typedef interface IDecklinkPushSource2 IDecklinkPushSource2;
#endif 	/* __IDecklinkPushSource2_FWD_DEFINED__ */


#ifndef __IDecklinkPushSource3_FWD_DEFINED__
#define __IDecklinkPushSource3_FWD_DEFINED__
typedef interface IDecklinkPushSource3 IDecklinkPushSource3;
#endif 	/* __IDecklinkPushSource3_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __DecklinkSamplesLib_LIBRARY_DEFINED__
#define __DecklinkSamplesLib_LIBRARY_DEFINED__

/* library DecklinkSamplesLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DecklinkSamplesLib;

EXTERN_C const CLSID CLSID_DecklinkPushSource;

#ifdef __cplusplus

class DECLSPEC_UUID("7188158d-52d9-473e-bfaf-07f502d2b29f")
DecklinkPushSource;
#endif

EXTERN_C const CLSID CLSID_DecklinkVideoSource;

#ifdef __cplusplus

class DECLSPEC_UUID("03d64d10-1ca5-4cce-a3cf-2ed87350bfd8")
DecklinkVideoSource;
#endif

EXTERN_C const CLSID CLSID_DecklinkAudioSource;

#ifdef __cplusplus

class DECLSPEC_UUID("7d49f8ae-8aae-442b-ad0d-93394cd08aa5")
DecklinkAudioSource;
#endif

EXTERN_C const CLSID CLSID_CustomMemAllocator;

#ifdef __cplusplus

class DECLSPEC_UUID("260ae29a-30ea-470d-910e-08a47d4eb58a")
CustomMemAllocator;
#endif

EXTERN_C const CLSID CLSID_DecklinkFieldSwap;

#ifdef __cplusplus

class DECLSPEC_UUID("a64cfd94-940b-4165-9d45-cb2e1127a864")
DecklinkFieldSwap;
#endif

EXTERN_C const CLSID CLSID_DecklinkStillSource;

#ifdef __cplusplus

class DECLSPEC_UUID("175e188f-80d3-4ea9-aa65-80a2194520d9")
DecklinkStillSource;
#endif

EXTERN_C const CLSID CLSID_DecklinkComposite;

#ifdef __cplusplus

class DECLSPEC_UUID("6b255b74-7d85-40af-97b0-73b4de64bdca")
DecklinkComposite;
#endif

EXTERN_C const CLSID CLSID_DecklinkCompositeProperties;

#ifdef __cplusplus

class DECLSPEC_UUID("437fb073-062e-454a-a8c8-bfa7a2f63964")
DecklinkCompositeProperties;
#endif

EXTERN_C const CLSID CLSID_WavDest;

#ifdef __cplusplus

class DECLSPEC_UUID("3c78b8e2-6c4d-11d1-ade2-0000f8754b99")
WavDest;
#endif

EXTERN_C const CLSID CLSID_DecklinkToneSource;

#ifdef __cplusplus

class DECLSPEC_UUID("3E2CE551-8699-47fe-B94D-BB275E663473")
DecklinkToneSource;
#endif

#ifndef __IDecklinkPushSource_INTERFACE_DEFINED__
#define __IDecklinkPushSource_INTERFACE_DEFINED__

/* interface IDecklinkPushSource */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDecklinkPushSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BC13BB43-B681-451b-82C8-8B66A9FF3848")
    IDecklinkPushSource : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetFrameBuffer( 
            /* [out] */ unsigned char **ppBuffer,
            /* [out] */ unsigned long *pSize) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Deliver( 
            /* [in] */ unsigned char *pBuffer) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDecklinkPushSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDecklinkPushSource * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDecklinkPushSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDecklinkPushSource * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetFrameBuffer )( 
            IDecklinkPushSource * This,
            /* [out] */ unsigned char **ppBuffer,
            /* [out] */ unsigned long *pSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Deliver )( 
            IDecklinkPushSource * This,
            /* [in] */ unsigned char *pBuffer);
        
        END_INTERFACE
    } IDecklinkPushSourceVtbl;

    interface IDecklinkPushSource
    {
        CONST_VTBL struct IDecklinkPushSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDecklinkPushSource_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDecklinkPushSource_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDecklinkPushSource_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDecklinkPushSource_GetFrameBuffer(This,ppBuffer,pSize)	\
    ( (This)->lpVtbl -> GetFrameBuffer(This,ppBuffer,pSize) ) 

#define IDecklinkPushSource_Deliver(This,pBuffer)	\
    ( (This)->lpVtbl -> Deliver(This,pBuffer) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDecklinkPushSource_INTERFACE_DEFINED__ */


#ifndef __IDecklinkPushSource2_INTERFACE_DEFINED__
#define __IDecklinkPushSource2_INTERFACE_DEFINED__

/* interface IDecklinkPushSource2 */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDecklinkPushSource2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C109963-8DA9-4eef-9F2E-D935559103CE")
    IDecklinkPushSource2 : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetFrameBuffer( 
            /* [out] */ IMediaSample **ppSample) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Deliver( 
            /* [in] */ IMediaSample *pSample) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDecklinkPushSource2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDecklinkPushSource2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDecklinkPushSource2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDecklinkPushSource2 * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetFrameBuffer )( 
            IDecklinkPushSource2 * This,
            /* [out] */ IMediaSample **ppSample);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Deliver )( 
            IDecklinkPushSource2 * This,
            /* [in] */ IMediaSample *pSample);
        
        END_INTERFACE
    } IDecklinkPushSource2Vtbl;

    interface IDecklinkPushSource2
    {
        CONST_VTBL struct IDecklinkPushSource2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDecklinkPushSource2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDecklinkPushSource2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDecklinkPushSource2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDecklinkPushSource2_GetFrameBuffer(This,ppSample)	\
    ( (This)->lpVtbl -> GetFrameBuffer(This,ppSample) ) 

#define IDecklinkPushSource2_Deliver(This,pSample)	\
    ( (This)->lpVtbl -> Deliver(This,pSample) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDecklinkPushSource2_INTERFACE_DEFINED__ */


#ifndef __IDecklinkPushSource3_INTERFACE_DEFINED__
#define __IDecklinkPushSource3_INTERFACE_DEFINED__

/* interface IDecklinkPushSource3 */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDecklinkPushSource3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FA94DA67-AF74-4389-9E64-33239BA9A1D6")
    IDecklinkPushSource3 : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFrameBuffer( 
            /* [out] */ IUnknown **ppSample) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Deliver( 
            /* [in] */ IUnknown *pSample) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDecklinkPushSource3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDecklinkPushSource3 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDecklinkPushSource3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDecklinkPushSource3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDecklinkPushSource3 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDecklinkPushSource3 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDecklinkPushSource3 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDecklinkPushSource3 * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetFrameBuffer )( 
            IDecklinkPushSource3 * This,
            /* [out] */ IUnknown **ppSample);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Deliver )( 
            IDecklinkPushSource3 * This,
            /* [in] */ IUnknown *pSample);
        
        END_INTERFACE
    } IDecklinkPushSource3Vtbl;

    interface IDecklinkPushSource3
    {
        CONST_VTBL struct IDecklinkPushSource3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDecklinkPushSource3_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDecklinkPushSource3_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDecklinkPushSource3_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDecklinkPushSource3_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IDecklinkPushSource3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IDecklinkPushSource3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IDecklinkPushSource3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IDecklinkPushSource3_GetFrameBuffer(This,ppSample)	\
    ( (This)->lpVtbl -> GetFrameBuffer(This,ppSample) ) 

#define IDecklinkPushSource3_Deliver(This,pSample)	\
    ( (This)->lpVtbl -> Deliver(This,pSample) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDecklinkPushSource3_INTERFACE_DEFINED__ */

#endif /* __DecklinkSamplesLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


