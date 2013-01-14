//------------------------------------------------------------------------------
// DecklinkVideoSource.h
//
// Desc: DirectShow sample code - External push mode source filter
//       External applications, through a custom interface provide
//		 frames to the output video stream.
//		 Largely based upon DirectShow SDK push source sample.
//
// Copyright (c) Blackmagic Design 2006.  All rights reserved.
//------------------------------------------------------------------------------

#pragma once

#include "DecklinkPushSource.h"

//------------------------------------------------------------------------------
// CDecklinkVideoSourcePin class
//------------------------------------------------------------------------------
// This class defines the video pin on our filter
//
class CDecklinkVideoSourcePin : public CDecklinkPushPin
{
public:
    CDecklinkVideoSourcePin(HRESULT* phr, CSource* pFilter);

    // Override the version that offers exactly one media type
    virtual HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest);
    virtual HRESULT FillBuffer(IMediaSample* pSample);
    
	// IAMStreamConfig interface
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE** ppamt);
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE* pamt);
};

//------------------------------------------------------------------------------
// CDecklinkVideoSource
//------------------------------------------------------------------------------
// This class defines our filter
//
class CDecklinkVideoSource : public CSource
//						, public IAMFilterMiscFlags
{

private:
    // Constructor is private because you have to use CreateInstance
    CDecklinkVideoSource(IUnknown* pUnk, HRESULT* phr);
    ~CDecklinkVideoSource();

	DECLARE_IUNKNOWN;

    // Base class overrides
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IMediaFilter interface override
	STDMETHODIMP GetState(DWORD dwMilliSecsTimeout, FILTER_STATE* pState);

	// IAMFilterMiscFlags interface
//	ULONG STDMETHODCALLTYPE GetMiscFlags(void) {return AM_FILTER_MISC_FLAGS_IS_SOURCE;}

    CDecklinkVideoSourcePin* m_pPin;

public:
    static CUnknown* WINAPI CreateInstance(IUnknown* pUnk, HRESULT* phr);  

};

