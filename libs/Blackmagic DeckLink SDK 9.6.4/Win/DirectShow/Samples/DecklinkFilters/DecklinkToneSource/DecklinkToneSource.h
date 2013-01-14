//------------------------------------------------------------------------------
// DecklinkToneSource.h
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//------------------------------------------------------------------------------

#pragma once

//#include "DecklinkPushSource.h"

//------------------------------------------------------------------------------
// CDecklinkToneSourcePin class
//------------------------------------------------------------------------------
// This class defines the pin on our filter
//
class CDecklinkToneSourcePin : public CSourceStream
						, public IAMStreamConfig
						, public IAMBufferNegotiation
						, public CBaseStreamControl
						, public CSourceSeeking
{
public:
    CDecklinkToneSourcePin(TCHAR* pObjectName, HRESULT* phr, CSource* pFilter, LPCWSTR pName);
    ~CDecklinkToneSourcePin();

    // Base class overrides
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return CSourceStream::GetOwner()->QueryInterface(riid,ppv); };
    STDMETHODIMP_(ULONG) AddRef() { return CSourceStream::GetOwner()->AddRef(); };
    STDMETHODIMP_(ULONG) Release() { return CSourceStream::GetOwner()->Release(); };
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // Override the version that offers exactly one media type
    virtual HRESULT GetMediaType(CMediaType* pMediaType);
    virtual HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest);
    virtual HRESULT FillBuffer(IMediaSample* pSample);
	virtual HRESULT OnThreadStartPlay(void);

	// for the IAMStreamControl interface
	STDMETHODIMP BeginFlush(void);
	STDMETHODIMP EndFlush(void);
    
    // Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter* pSelf, Quality q)
    {
        return E_FAIL;
    }

	// IAMStreamConfig interface
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE** ppamt);
    STDMETHODIMP GetNumberOfCapabilities(int* piCount, int* piSize);
    STDMETHODIMP GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppamt, BYTE* pSCC);
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE* pamt);

	// IAMBufferNegotiation interface
	STDMETHODIMP SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES* pprop);
	STDMETHODIMP GetAllocatorProperties(ALLOCATOR_PROPERTIES* pprop);

	// CSourceSeeking implementation
	virtual HRESULT ChangeStart(void);
	virtual HRESULT ChangeStop(void);
	virtual HRESULT ChangeRate(void);

protected:
    REFERENCE_TIME m_rtNextValidStart;	// Stream start time of the next sample.  This gets reset when during a media seek operation.
    REFERENCE_TIME m_rtMediaTime;		// This audio push source is a virtual audio file reader.  This field is the current position within the virtual file.

    CCritSec m_cSharedState;            // Protects our internal state.

	CMediaType m_MediaType;	// media type of the desired connection

	// CBaseStreamControl implementation
	BOOL m_bLastSampleDiscarded;

	// IAMBufferNegotiation interface support
	ALLOCATOR_PROPERTIES m_allocProp;

	// CSourceSeeking implementation
	void UpdateFromSeek(void);
};

//------------------------------------------------------------------------------
// CDecklinkToneSource
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// This class defines our filter
//
class CDecklinkToneSource : public CSource
					, public IAMFilterMiscFlags
{
private:
    // Constructor is private because you have to use CreateInstance
    CDecklinkToneSource(IUnknown* pUnk, HRESULT* phr);
    ~CDecklinkToneSource();

	DECLARE_IUNKNOWN;

    // Base class overrides
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// Overrides for CBaseStreamControl implementation
	STDMETHODIMP SetSyncSource(IReferenceClock* pClock);
    STDMETHODIMP JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName);
	STDMETHODIMP Stop();
	STDMETHODIMP Pause();
	STDMETHODIMP Run(REFERENCE_TIME tStart);

	// IMediaFilter interface override
	STDMETHODIMP GetState(DWORD dwMilliSecsTimeout, FILTER_STATE* pState);

	// IAMFilterMiscFlags interface
	ULONG STDMETHODCALLTYPE GetMiscFlags(void) {return AM_FILTER_MISC_FLAGS_IS_SOURCE;}

    CDecklinkToneSourcePin* m_pPin;

public:
    static CUnknown* WINAPI CreateInstance(IUnknown* pUnk, HRESULT* phr);  

};
