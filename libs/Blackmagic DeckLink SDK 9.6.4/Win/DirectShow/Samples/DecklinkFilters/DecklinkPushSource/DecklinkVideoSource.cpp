//------------------------------------------------------------------------------
// DecklinkVideoSource.cpp
//
// Desc: DirectShow sample code - External push mode source filter
//       External applications, through a custom interface provide
//		 frames to the output video stream.
//		 Largely based upon DirectShow SDK push source sample.
//
// Copyright (c) Blackmagic Design 2006.  All rights reserved.
//
// Read readme.txt!
//
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "DecklinkFilters_h.h"

#include "DecklinkVideoSource.h"
#include "CustomAllocator.h"

//------------------------------------------------------------------------------
static const int DBG_ERR = 1;
static const int DBG_VID = 2;
static const int DBG_WRN = 2;

//------------------------------------------------------------------------------
// CDecklinkPushPin Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Constructor
//
CDecklinkVideoSourcePin::CDecklinkVideoSourcePin(HRESULT* phr, CSource* pFilter)
      : CDecklinkPushPin(NAME("Decklink Video Source"), phr, pFilter, L"Video")
{
	// set default/preferred media type
	// the app can use IAMStreamConfig interface to change this connection media type
	m_MediaType.InitMediaType();
	m_MediaType.SetType(&MEDIATYPE_Video);
	m_MediaType.SetSubtype(&MEDIASUBTYPE_UYVY);
	m_MediaType.SetFormatType(&FORMAT_VideoInfo);
	VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)m_MediaType.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
	ZeroMemory(pvih, sizeof(VIDEOINFOHEADER));
	pvih->AvgTimePerFrame = FPS_2997;
	pvih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pvih->bmiHeader.biWidth = 720;
	pvih->bmiHeader.biHeight = 486;
	pvih->bmiHeader.biPlanes = 1;
	pvih->bmiHeader.biBitCount = 16;
	pvih->bmiHeader.biCompression = 'YVYU';
	pvih->bmiHeader.biSizeImage = m_MediaType.lSampleSize = pvih->bmiHeader.biWidth * pvih->bmiHeader.biHeight * pvih->bmiHeader.biBitCount / 8;
	pvih->dwBitRate = pvih->bmiHeader.biSizeImage * (DWORD)((float)UNITS / pvih->AvgTimePerFrame) * 8;
}

//------------------------------------------------------------------------------
// DecideBufferSize
// Put in a request to the allocator for number of buffers to allocate, their size,
// alignment, etc.
HRESULT CDecklinkVideoSourcePin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
    HRESULT hr = S_OK;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pRequest, E_POINTER);

    BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(&m_mt);
	if (pbmih)
	{
		if (pRequest->cBuffers == 0)
		{
			pRequest->cBuffers = 2;
		}
		pRequest->cbBuffer = pbmih->biSizeImage;

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
HRESULT CDecklinkVideoSourcePin::FillBuffer(IMediaSample *pSample)
{
    CheckPointer(pSample, E_POINTER);

    CAutoLock cAutoLockShared(&m_cSharedState);

    // Check that we're still using video
    ASSERT((m_mt.formattype == FORMAT_VideoInfo) || (m_mt.formattype == FORMAT_VideoInfo2));

	// Set the sample timestamp.  Pretty bloody important.
	// DShow is governed by sample timestamps.  As this push source
	// will deliver at a variable rate, determined by the rate at
	// which the external app can deliver frames, use the current stream time
	// for the sample timestamp of the FIRST sample.  Every subsequent sample
	// is timestamped at intervals of m_rtFrameLength.

	CRefTime rtStream;
    m_pFilter->StreamTime(rtStream);

	// timestamp the first sample with the current stream time.
	if (0 == m_iFrameNumber)
	{
		m_rtNextValidStart = rtStream;
	}
    REFERENCE_TIME rtStart = m_rtNextValidStart;
    REFERENCE_TIME rtStop  = rtStart + m_rtFrameLength;
		m_rtNextValidStart = rtStop;

    pSample->SetTime(&rtStart, &rtStop);
	static long lastTime = 0;
	long time = timeGetTime();
	DbgLog((LOG_TRACE, DBG_VID, TEXT("VID_PUSH: %I64d [%I64d  %I64d] %ld"), rtStream.m_time, rtStart, rtStop, time - lastTime));
	lastTime = time;

	// set sample media times
	rtStart = m_iFrameNumber;
	rtStop = rtStart + 1;
	pSample->SetMediaTime(&rtStart, &rtStop);

    m_iFrameNumber++;

    // Set TRUE on every sample for uncompressed frames
    pSample->SetSyncPoint(TRUE);

    return S_OK;
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
STDMETHODIMP CDecklinkVideoSourcePin::GetFormat(AM_MEDIA_TYPE** ppamt)
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
STDMETHODIMP CDecklinkVideoSourcePin::SetFormat(AM_MEDIA_TYPE* pamt)
{
	HRESULT hr = S_OK;
	CheckPointer(pamt, E_POINTER);

	if (IsStopped())
	{
		// TODO: Check that this is an acceptable media type
		m_MediaType.Set(*pamt);
		m_rtFrameLength = CUtils::GetAvgTimePerFrame(pamt);
	}
	else
	{
		hr = VFW_E_NOT_STOPPED;
	}

	return hr;
}

//------------------------------------------------------------------------------
// CDecklinkVideoSource Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CreateInstance
//
CUnknown* WINAPI CDecklinkVideoSource::CreateInstance(IUnknown* pUnk, HRESULT* phr)
{
	CDecklinkVideoSource *pNewFilter = new CDecklinkVideoSource(pUnk, phr);

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
CDecklinkVideoSource::CDecklinkVideoSource(IUnknown* pUnk, HRESULT* phr)
           : CSource(NAME("Decklink Video Push Source"), pUnk, CLSID_DecklinkVideoSource)
{
	// The pin magically adds itself to our pin array.
	m_pPin = new CDecklinkVideoSourcePin(phr, this);

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
CDecklinkVideoSource::~CDecklinkVideoSource()
{
    delete m_pPin;
}

//------------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
STDMETHODIMP CDecklinkVideoSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
//	if (riid == IID_IAMFilterMiscFlags)
//	{
//		return GetInterface((IAMFilterMiscFlags*)this, ppv);
//	}
	return CSource::NonDelegatingQueryInterface(riid, ppv);
}

// -------------------------------------------------------------------------
// IMediaFilter interface overrides
// -------------------------------------------------------------------------
// GetState
// Override the base class implementation as this filter is a live source and
// is unable to deliver data in a paused state
STDMETHODIMP CDecklinkVideoSource::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE* pState)
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
