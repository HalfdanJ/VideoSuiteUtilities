//------------------------------------------------------------------------------
// DecklinkAudioSource.h
//
// Desc: DirectShow sample code - External push mode source filter
//       External applications, through a custom interface provide
//		 frames to the output audio stream.
//		 Largely based upon DirectShow SDK push source sample.
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//------------------------------------------------------------------------------

#pragma once

#include "DecklinkPushSource.h"

//------------------------------------------------------------------------------
// CDecklinkAudioSourcePin class
//------------------------------------------------------------------------------
// This class defines the pin on our filter
//
class CDecklinkAudioSourcePin : public CDecklinkPushPin
{
public:
    CDecklinkAudioSourcePin(HRESULT* phr, CSource* pFilter);

    // Override the version that offers exactly one media type
    virtual HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest);
    virtual HRESULT FillBuffer(IMediaSample* pSample);
	virtual HRESULT OnThreadStartPlay(void);

	// IAMStreamConfig interface
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE** ppamt);
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE* pamt);

private:
    REFERENCE_TIME m_rtMediaTime;
};

//------------------------------------------------------------------------------
// CDecklinkAudioSource
//------------------------------------------------------------------------------
// This class defines our filter
//
class CDecklinkAudioSource : public CSource
						, public IAMFilterMiscFlags
{

private:
    // Constructor is private because you have to use CreateInstance
    CDecklinkAudioSource(IUnknown* pUnk, HRESULT* phr);
    ~CDecklinkAudioSource();

	DECLARE_IUNKNOWN;

    // Base class overrides
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IMediaFilter interface override
	STDMETHODIMP GetState(DWORD dwMilliSecsTimeout, FILTER_STATE* pState);

	// IAMFilterMiscFlags interface
	ULONG STDMETHODCALLTYPE GetMiscFlags(void) {return AM_FILTER_MISC_FLAGS_IS_SOURCE;}

    CDecklinkAudioSourcePin* m_pPin;

public:
    static CUnknown* WINAPI CreateInstance(IUnknown* pUnk, HRESULT* phr);  

};

