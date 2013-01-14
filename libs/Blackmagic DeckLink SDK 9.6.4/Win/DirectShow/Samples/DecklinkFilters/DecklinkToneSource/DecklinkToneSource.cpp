//------------------------------------------------------------------------------
// DecklinkToneSource.cpp
//
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "DecklinkToneSource.h"
#include "DecklinkFilters_h.h"

//------------------------------------------------------------------------------
static const int DBG_ERR = 1;
static const int DBG_AUD = 2;
static const int DBG_WRN = 2;

//------------------------------------------------------------------------------
// CDecklinkToneSourcePin Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Constructor
//
CDecklinkToneSourcePin::CDecklinkToneSourcePin(TCHAR* pObjectName, HRESULT* phr, CSource* pFilter, LPCWSTR pName)
	: CSourceStream(pObjectName, phr, pFilter, pName)
	, CSourceSeeking(pObjectName, (IPin*)this, phr, &m_cSharedState)
	, m_rtNextValidStart(0)
	, m_rtMediaTime(0)
	, m_bLastSampleDiscarded(FALSE)
{
	// set default/preferred media type
	// the app can use IAMStreamConfig interface to change this connection media type
	m_MediaType.InitMediaType();
	m_MediaType.SetType(&MEDIATYPE_Audio);
	m_MediaType.SetSubtype(&MEDIASUBTYPE_PCM);
	m_MediaType.SetFormatType(&FORMAT_WaveFormatEx);
	WAVEFORMATEX* pwfex = (WAVEFORMATEX*)m_MediaType.AllocFormatBuffer(sizeof(WAVEFORMATEX));
	ZeroMemory(pwfex, sizeof(WAVEFORMATEX));
    pwfex->wFormatTag = WAVE_FORMAT_PCM;
    pwfex->nChannels = 2;
    pwfex->nSamplesPerSec = 48000;
    pwfex->wBitsPerSample = 16;
    pwfex->nBlockAlign = pwfex->nChannels * pwfex->wBitsPerSample / 8;
    pwfex->nAvgBytesPerSec = pwfex->nSamplesPerSec * pwfex->nBlockAlign;
    m_MediaType.SetSampleSize(pwfex->nBlockAlign);

	// initialise to "don't care"
	m_allocProp.cBuffers = m_allocProp.cbBuffer = m_allocProp.cbAlign = m_allocProp.cbPrefix = -1;
}

//------------------------------------------------------------------------------
// Destructor
//
CDecklinkToneSourcePin::~CDecklinkToneSourcePin()
{   
    m_MediaType.ResetFormatBuffer();
}

//------------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
STDMETHODIMP CDecklinkToneSourcePin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IAMStreamConfig)
	{
		return GetInterface(static_cast<IAMStreamConfig*>(this), ppv);
	}

	if (riid == IID_IAMBufferNegotiation)
	{
		return GetInterface(static_cast<IAMBufferNegotiation*>(this), ppv);
	}

	if (riid == IID_IAMStreamControl)
	{
		return GetInterface(static_cast<IAMStreamControl*>(this), ppv);
	}

	if (riid == IID_IMediaSeeking)
	{
		return CSourceSeeking::NonDelegatingQueryInterface(riid, ppv);
	}

	return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

//------------------------------------------------------------------------------
// GetMediaType: This method tells the downstream pin what types we support.
// Here is how CSourceStream deals with media types:
//
// If you support exactly one type, override GetMediaType(CMediaType*). It will then be
// called when (a) our filter proposes a media type, (b) the other filter proposes a
// type and we have to check that type.
//
// If you support > 1 type, override GetMediaType(int, CMediaType*) AND CheckMediaType.
//
// In this case we support only one type, which we obtain from the bitmap file.
HRESULT CDecklinkToneSourcePin::GetMediaType(CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pMediaType, E_POINTER);

    pMediaType->Set(m_MediaType);

    return S_OK;
}

//------------------------------------------------------------------------------
// DecideBufferSize
// Put in a request to the allocator for number of buffers to allocate, their size,
// alignment, etc.
HRESULT CDecklinkToneSourcePin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
    HRESULT hr = S_OK;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pRequest, E_POINTER);

    WAVEFORMATEX* pwfex = (WAVEFORMATEX*)m_mt.Format();
	if (pwfex)
	{
		if (pRequest->cBuffers == 0)
		{
			pRequest->cBuffers = 1;
		}
		pRequest->cbBuffer = pwfex->nAvgBytesPerSec;

		// let the application influence the buffer negotiation
		if (0 < m_allocProp.cBuffers)
		{
			pRequest->cBuffers = m_allocProp.cBuffers;
		}

		if (0 < m_allocProp.cbBuffer)
		{
			pRequest->cbBuffer = m_allocProp.cbBuffer;
		}

		if (0 < m_allocProp.cbAlign)
		{
			pRequest->cbAlign = m_allocProp.cbAlign;
		}

		if (0 < m_allocProp.cbPrefix)
		{
			pRequest->cbPrefix = m_allocProp.cbPrefix;
		}

		ALLOCATOR_PROPERTIES Actual;
		hr = pAlloc->SetProperties(pRequest, &Actual);
		if (SUCCEEDED(hr)) 
		{
			m_allocProp = Actual;	// for external apps to query the allocator properties used in the connection
			// Is this allocator unsuitable?
			if (Actual.cbBuffer < pRequest->cbBuffer) 
			{
				hr = E_FAIL;
			}
		}
	}
	else
	{
		hr = E_POINTER;
	}

    return hr;
}

//------------------------------------------------------------------------------
// FillBuffer
// This is where we set the timestamps for the samples.
// FillBuffer is called once for every sample in the stream.
HRESULT CDecklinkToneSourcePin::FillBuffer(IMediaSample *pSample)
{
    BYTE *pData;
    long cbData;

    CheckPointer(pSample, E_POINTER);

    CAutoLock cAutoLockShared(&m_cSharedState);

	pSample->GetPointer(&pData);
	cbData = pSample->GetActualDataLength();

    // Check that we're still using audio
    ASSERT(m_mt.formattype == FORMAT_WaveFormatEx);
	WAVEFORMATEX* pwfex = (WAVEFORMATEX*)m_mt.pbFormat;

	// Set to silence.
	ZeroMemory(pData, cbData);
	unsigned long ulSampleCount = cbData * 8 / pwfex->wBitsPerSample / pwfex->nChannels;

	REFERENCE_TIME rtStart = m_rtNextValidStart;
	REFERENCE_TIME rtDuration = (UNITS * ulSampleCount / pwfex->nSamplesPerSec);
	REFERENCE_TIME rtStop  = rtStart + rtDuration;
	m_rtNextValidStart += rtDuration;	// Increment the stream time.
	m_rtMediaTime += rtDuration;	// Increment the media time.
	pSample->SetTime(&rtStart, &rtStop);
#ifdef _DEBUG
	CRefTime rtStream;
	m_pFilter->StreamTime(rtStream);
	static long lastTime = 0;
	long time = timeGetTime();
	DbgLog((LOG_TRACE, DBG_AUD, TEXT("AUD_TONE: %I64d [%I64d  %I64d] %ld"), rtStream.m_time, rtStart, rtStop, time - lastTime));
	lastTime = time;
#endif
	// CSourceSeeking - test the stop time against the current media time.
	// This source is a virtual audio file and so the media time is the current
	// position within the virtual audio file.  Test the current position 
	// against the specified stop position and stop where appropriate.
	if (m_rtMediaTime > m_rtStop)
	{
		return S_FALSE;
	}

	// set sample media times
	rtStart = m_rtMediaTime;
	rtStop = rtStart + ulSampleCount;
	pSample->SetMediaTime(&rtStart, &rtStop);
	m_rtMediaTime = rtStop;

	// Set TRUE on every sample for uncompressed frames
	pSample->SetSyncPoint(TRUE);

	// Query CBaseStreamControl to determine whether to deliver this sample.
	int iStreamState = CheckStreamState(pSample);
	if (iStreamState == STREAM_FLOWING) 
	{
		// Deliver sample.
		if (m_bLastSampleDiscarded)
		{
			pSample->SetDiscontinuity(true);
		}
		m_bLastSampleDiscarded = false;
	} 
	else 
	{
		// DO NOT deliver sample.
		m_bLastSampleDiscarded = true;
		return S_FALSE;
	}

    return S_OK;
}

// -------------------------------------------------------------------------
// OnThreadStartPlay
//
HRESULT CDecklinkToneSourcePin::OnThreadStartPlay(void)
{
	m_bLastSampleDiscarded = true;
    return DeliverNewSegment(m_rtStart, m_rtStop, m_dRateSeeking);
}

// -------------------------------------------------------------------------
// BeginFlush
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkToneSourcePin::BeginFlush(void)
{
	Flushing(TRUE);
	return CSourceStream::BeginFlush();
}

// -------------------------------------------------------------------------
// EndFlush
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkToneSourcePin::EndFlush(void)
{
	Flushing(FALSE);
	return CSourceStream::EndFlush();
}

// -------------------------------------------------------------------------
// IAMStreamConfig interface
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// GetFormat
// If connected, return the media type of the connection, otherwise return the
// 'preferred' format.  As DS states that the formats should be listed in descending
// order of quality, our preferred format is the best one which is the first item
// in the list.
//
STDMETHODIMP CDecklinkToneSourcePin::GetFormat(AM_MEDIA_TYPE** ppamt)
{
	HRESULT hr = S_OK;
	CheckPointer(ppamt, E_POINTER);

	*ppamt = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
	if (*ppamt)
	{
		AM_MEDIA_TYPE *pamt = CreateMediaType((const AM_MEDIA_TYPE*)&m_MediaType);
		*ppamt = pamt;
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

//---------------------------------------------------------------
// SetFormat
//
STDMETHODIMP CDecklinkToneSourcePin::SetFormat(AM_MEDIA_TYPE* pamt)
{
	HRESULT hr = S_OK;
	CheckPointer(pamt, E_POINTER);

	if (IsStopped())
	{
		// TODO: Check that this is an acceptable media type
		m_MediaType.Set(*pamt);
	}
	else
	{
		hr = VFW_E_NOT_STOPPED;
	}

	return hr;
}

//---------------------------------------------------------------
// GetNumberOfCapabilities
// Returns the number of video formats supported by the underlying
// HW object.
//
STDMETHODIMP CDecklinkToneSourcePin::GetNumberOfCapabilities(int* piCount, int* piSize)
{
	CheckPointer(piCount, E_POINTER);
	CheckPointer(piSize, E_POINTER);

	*piCount = 1;
	*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);

	return S_OK;
}

//---------------------------------------------------------------
// GetStreamCaps
// Returns the media format at the specified index.
//
STDMETHODIMP CDecklinkToneSourcePin::GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppamt, BYTE* pSCC)
{
	HRESULT hr = S_OK;
	CheckPointer(ppamt, E_POINTER);
	CheckPointer(pSCC, E_POINTER);
	
    if (iIndex < 1)
	{
		// Allocate a block of memory for the media type including a separate allocation
		// for the format block at the end of the structure
		*ppamt = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
		if (*ppamt == NULL)
		{
			return E_OUTOFMEMORY;
		}
		
		ZeroMemory(*ppamt, sizeof(AM_MEDIA_TYPE));

		AM_MEDIA_TYPE *pamt = CreateMediaType((const AM_MEDIA_TYPE*)&m_MediaType);
		*ppamt = pamt;

		if (pamt)
		{
			AUDIO_STREAM_CONFIG_CAPS* scc = (AUDIO_STREAM_CONFIG_CAPS*)pSCC;
			const WAVEFORMATEX* pwfex = (const WAVEFORMATEX*)pamt->pbFormat;

			ZeroMemory(pSCC, sizeof(AUDIO_STREAM_CONFIG_CAPS));
			scc->guid = (*ppamt)->formattype;
			scc->MinimumChannels = scc->MaximumChannels = pwfex->nChannels;
			scc->MinimumBitsPerSample = scc->MaximumBitsPerSample = pwfex->wBitsPerSample;
			scc->MinimumSampleFrequency = scc->MaximumSampleFrequency = pwfex->nSamplesPerSec;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		hr = S_FALSE;
	}

	if (FAILED(hr) && (*ppamt))
	{
		CoTaskMemFree(*ppamt);
		*ppamt = NULL;
	}

	return hr;
}

// -------------------------------------------------------------------------
// --- IAMBufferNegotiation ---
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// SuggestAllocatorProperties
//
STDMETHODIMP CDecklinkToneSourcePin::SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES* pprop)
{
	HRESULT hr = S_OK;
	if (pprop)
	{
		if (IsConnected())
		{
			hr = VFW_E_ALREADY_CONNECTED;
		}
		else
		{
			m_allocProp = *pprop;
		}
	}
	else
	{
		hr = E_POINTER;
	}
	return hr;
}

// -------------------------------------------------------------------------
// GetAllocatorProperties
//
STDMETHODIMP CDecklinkToneSourcePin::GetAllocatorProperties(ALLOCATOR_PROPERTIES* pprop)
{
	HRESULT hr = S_OK;
	if (pprop)
	{
		if (IsConnected())
		{
			*pprop = m_allocProp;
		}
		else
		{
			hr = VFW_E_NOT_CONNECTED;
		}
	}
	else
	{
		hr = E_POINTER;
	}
	return hr;
}

// -------------------------------------------------------------------------
// CSourceSeeking implementation
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// ChangeStart
//
HRESULT CDecklinkToneSourcePin::ChangeStart(void)
{
    m_rtNextValidStart = 0;		// Reset the time stamps.
    m_rtMediaTime = m_rtStart;	// Reset the media times.

    UpdateFromSeek();
    return S_OK;
}

// -------------------------------------------------------------------------
// ChangeStop
//
HRESULT CDecklinkToneSourcePin::ChangeStop(void)
{
    UpdateFromSeek();
    return S_OK;
}

// -------------------------------------------------------------------------
// ChangeRate
//
HRESULT CDecklinkToneSourcePin::ChangeRate(void)
{
    {   // Scope for critical section lock.
        CAutoLock cAutoLockSeeking(CSourceSeeking::m_pLock);
        if( m_dRateSeeking <= 0 ) {
            m_dRateSeeking = 1.0;  // Reset to a reasonable value.
            return E_FAIL;
        }
    }
    UpdateFromSeek();
    return S_OK;
}

// -------------------------------------------------------------------------
// UpdateFromSeek
//
void CDecklinkToneSourcePin::UpdateFromSeek(void)
{
    if (ThreadExists()) 
    {
        DeliverBeginFlush();
        Stop();
        DeliverEndFlush();
        Run();
    }
}

//------------------------------------------------------------------------------
// CDecklinkToneSource Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CreateInstance
//
CUnknown* WINAPI CDecklinkToneSource::CreateInstance(IUnknown* pUnk, HRESULT* phr)
{
	CDecklinkToneSource *pNewFilter = new CDecklinkToneSource(pUnk, phr);

	if (phr)
	{
		if (pNewFilter == NULL)
		{
			*phr = E_OUTOFMEMORY;
		}
		else
		{
			*phr = S_OK;
		}
	}

	return pNewFilter;
}

//------------------------------------------------------------------------------
// Constructor
//
CDecklinkToneSource::CDecklinkToneSource(IUnknown* pUnk, HRESULT* phr)
           : CSource(NAME("Decklink Audio Tone Source"), pUnk, CLSID_DecklinkToneSource)
{
	// The pin magically adds itself to our pin array.
	m_pPin = new CDecklinkToneSourcePin(NAME("Decklink Audio Tone Source Pin"), phr, this, L"Out");

	if (phr)
	{
		if (m_pPin == NULL)
		{
			*phr = E_OUTOFMEMORY;
		}
		else
		{
			*phr = S_OK;
		}
	}  
}

//------------------------------------------------------------------------------
// Destructor
//
CDecklinkToneSource::~CDecklinkToneSource()
{
    delete m_pPin;
}

//------------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
STDMETHODIMP CDecklinkToneSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IAMFilterMiscFlags)
	{
		return GetInterface((IAMFilterMiscFlags*)this, ppv);
	}
	return CSource::NonDelegatingQueryInterface(riid, ppv);
}

// -------------------------------------------------------------------------
// Base class overrides for IStreamControl implementation
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// SetSyncSource
//
STDMETHODIMP CDecklinkToneSource::SetSyncSource(IReferenceClock* pClock)
{
	// Notify CBaseStreamControl of the base reference clock.
	CDecklinkToneSourcePin* pPin = (CDecklinkToneSourcePin*)m_paStreams[0];
	pPin->SetSyncSource(pClock);

	return CSource::SetSyncSource(pClock);
}

// -------------------------------------------------------------------------
// JoinFilterGraph
// tell CBaseStreamControl what sink to use
STDMETHODIMP CDecklinkToneSource::JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName)
{
	HRESULT hr = CSource::JoinFilterGraph(pGraph, pName);
	if (SUCCEEDED(hr))
	{
		CDecklinkToneSourcePin* pPin = (CDecklinkToneSourcePin*)m_paStreams[0];
		pPin->SetFilterGraph(m_pSink);
	}
	return hr;
}

// -------------------------------------------------------------------------
// Stop
//
STDMETHODIMP CDecklinkToneSource::Stop()
{
	HRESULT hr = CSource::Stop();
	if (SUCCEEDED(hr))
	{
		CDecklinkToneSourcePin* pPin = (CDecklinkToneSourcePin*)m_paStreams[0];
		pPin->NotifyFilterState(State_Stopped, 0);
	}

    return hr;
}

// -------------------------------------------------------------------------
// Pause
//
STDMETHODIMP CDecklinkToneSource::Pause()
{
	HRESULT hr = CSource::Pause();
	if (SUCCEEDED(hr))
	{
		CDecklinkToneSourcePin* pPin = (CDecklinkToneSourcePin*)m_paStreams[0];
		pPin->NotifyFilterState(State_Paused, 0);
	}

    return hr;
}

// -------------------------------------------------------------------------
// Run
//
STDMETHODIMP CDecklinkToneSource::Run(REFERENCE_TIME tStart)
{
	HRESULT hr = CSource::Run(tStart);
	if (SUCCEEDED(hr))
	{
		CDecklinkToneSourcePin* pPin = (CDecklinkToneSourcePin*)m_paStreams[0];
		pPin->NotifyFilterState(State_Running, tStart);
	}

	return hr;
}

// -------------------------------------------------------------------------
// IMediaFilter interface overrides
// -------------------------------------------------------------------------
// GetState
// Override the base class implementation as this filter is a live source and
// is unable to deliver data in a paused state
STDMETHODIMP CDecklinkToneSource::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE* pState)
{
    HRESULT hr = S_OK;
    
    if (pState)
    {
		CSource::GetState(dwMilliSecsTimeout, pState);
		if (State_Paused == *pState)
		{
			hr = VFW_S_CANT_CUE;
		}
	}
	else
	{
	    hr = E_POINTER;
	}
	
	return hr;
}
