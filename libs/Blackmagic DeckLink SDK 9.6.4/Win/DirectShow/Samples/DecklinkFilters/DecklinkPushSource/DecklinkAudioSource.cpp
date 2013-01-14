//------------------------------------------------------------------------------
// DecklinkAudioSource.cpp
//
// Desc: DirectShow sample code - External push mode source filter
//       External applications, through a custom interface provide
//		 frames to the output audio stream.
//		 Largely based upon DirectShow SDK push source sample.
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//
// Read readme.txt!
//
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "DecklinkFilters_h.h"

#include "DecklinkAudioSource.h"
#include "CustomAllocator.h"

//------------------------------------------------------------------------------
static const int DBG_ERR = 1;
static const int DBG_AUD = 2;
static const int DBG_WRN = 2;

//------------------------------------------------------------------------------
// CDecklinkAudioSourcePin Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Constructor
//
CDecklinkAudioSourcePin::CDecklinkAudioSourcePin(HRESULT* phr, CSource* pFilter)
	: CDecklinkPushPin(NAME("Decklink Audio Source"), phr, pFilter, L"Audio")
	, m_rtMediaTime(0)
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
}

//------------------------------------------------------------------------------
// DecideBufferSize
// Put in a request to the allocator for number of buffers to allocate, their size,
// alignment, etc.
HRESULT CDecklinkAudioSourcePin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
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
HRESULT CDecklinkAudioSourcePin::FillBuffer(IMediaSample *pSample)
{
    CheckPointer(pSample, E_POINTER);

    CAutoLock cAutoLockShared(&m_cSharedState);

    // Check that we're still using audio
    ASSERT(m_mt.formattype == FORMAT_WaveFormatEx);

	WAVEFORMATEX* pwfex = (WAVEFORMATEX*)m_mt.pbFormat;
    if (WAVE_FORMAT_PCM == pwfex->wFormatTag)
    {
		unsigned long ulSampleCount = pSample->GetActualDataLength() * 8 / pwfex->wBitsPerSample / pwfex->nChannels;

		// Set the sample timestamp.  Pretty bloody important.
		// DShow is governed by sample timestamps.  As this push source
		// will deliver at a variable rate, determined by the rate at
		// which the external app can deliver frames, use the current stream time
		// for the sample timestamp of the FIRST sample.  Every subsequent sample
		// is timestamped at intervals of the number of audio samples written.

		CRefTime rtStream;
		m_pFilter->StreamTime(rtStream);

		// timestamp the first sample with the current stream time.
		if (0 == m_iFrameNumber)
		{
			m_rtNextValidStart = rtStream;
		}
	    REFERENCE_TIME rtStart = m_rtNextValidStart;
	    REFERENCE_TIME rtStop  = rtStart + (UNITS * ulSampleCount / pwfex->nSamplesPerSec);
	    m_rtNextValidStart = rtStop;

		pSample->SetTime(&rtStart, &rtStop);
		static long lastTime = 0;
		long time = timeGetTime();
		DbgLog((LOG_TRACE, DBG_AUD, TEXT("AUD_PUSH: %I64d [%I64d  %I64d] %ld  %I1064d"), rtStream.m_time, rtStart, rtStop, time - lastTime, m_rtMediaTime));
		lastTime = time;

		// set sample media times
		rtStart = m_rtMediaTime;
		rtStop = rtStart + ulSampleCount;
		pSample->SetMediaTime(&rtStart, &rtStop);
		m_rtMediaTime = rtStop;

	    m_iFrameNumber++;

		// Set TRUE on every sample for uncompressed frames
		pSample->SetSyncPoint(TRUE);
	}

    return S_OK;
}

// -------------------------------------------------------------------------
// OnThreadStartPlay
//
HRESULT CDecklinkAudioSourcePin::OnThreadStartPlay(void)
{
	m_rtMediaTime = 0;
	return CDecklinkPushPin::OnThreadStartPlay();
}

// -------------------------------------------------------------------------
// IAMStreamConfig interface
// -------------------------------------------------------------------------
// GetFormat
// If connected, return the media type of the connection, otherwise return the
// 'preferred' format.  As DS states that the formats should be listed in descending
// order of quality, our preferred format is the best one which is the first item
// in the list.
//
STDMETHODIMP CDecklinkAudioSourcePin::GetFormat(AM_MEDIA_TYPE** ppamt)
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
STDMETHODIMP CDecklinkAudioSourcePin::SetFormat(AM_MEDIA_TYPE* pamt)
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

//------------------------------------------------------------------------------
// CDecklinkAudioSource Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CreateInstance
//
CUnknown* WINAPI CDecklinkAudioSource::CreateInstance(IUnknown* pUnk, HRESULT* phr)
{
	CDecklinkAudioSource *pNewFilter = new CDecklinkAudioSource(pUnk, phr);

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
CDecklinkAudioSource::CDecklinkAudioSource(IUnknown* pUnk, HRESULT* phr)
           : CSource(NAME("Decklink Audio Push Source"), pUnk, CLSID_DecklinkAudioSource)
{
	// The pin magically adds itself to our pin array.
	m_pPin = new CDecklinkAudioSourcePin(phr, this);

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
CDecklinkAudioSource::~CDecklinkAudioSource()
{
    delete m_pPin;
}

//------------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
STDMETHODIMP CDecklinkAudioSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IAMFilterMiscFlags)
	{
		return GetInterface((IAMFilterMiscFlags*)this, ppv);
	}
	return CSource::NonDelegatingQueryInterface(riid, ppv);
}

// -------------------------------------------------------------------------
// IMediaFilter interface overrides
// -------------------------------------------------------------------------
// GetState
// Override the base class implementation as this filter is a live source and
// is unable to deliver data in a paused state
STDMETHODIMP CDecklinkAudioSource::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE* pState)
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
