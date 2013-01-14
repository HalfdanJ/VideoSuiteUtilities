//------------------------------------------------------------------------------
// DecklinkPushSource.cpp
//
// Desc: DirectShow sample code - External push mode source filter
//       External applications, through a custom interface provide
//		 frames to the output video stream.
//		 Largely based upon DirectShow SDK push source sample.
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//
// Read readme.txt!
//
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "DecklinkFilters_h.h"

#include "DecklinkPushSource.h"
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
CDecklinkPushPin::CDecklinkPushPin(TCHAR* pObjectName, HRESULT* phr, CSource* pFilter, LPCWSTR pName)
      : CSourceStream(pObjectName, phr, pFilter, pName)
      , m_iFrameNumber(0)
      , m_rtFrameLength(FPS_2997)
      , m_rtNextValidStart(0)
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

	// initialise to "don't care"
	m_allocProp.cBuffers = m_allocProp.cbBuffer = m_allocProp.cbAlign = m_allocProp.cbPrefix = -1;
}

//------------------------------------------------------------------------------
// Destructor
//
CDecklinkPushPin::~CDecklinkPushPin()
{   
    DbgLog((LOG_TRACE, 3, TEXT("Frames written %d"),m_iFrameNumber));
    m_MediaType.ResetFormatBuffer();
}

//------------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
STDMETHODIMP CDecklinkPushPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if ((riid == IID_IDecklinkPushSource) || (riid == IID_IDecklinkPushSource2) || (riid == IID_IDecklinkPushSource3))
	{
		if (m_pAllocator)
		{
			return m_pAllocator->QueryInterface(riid, ppv);
		}
		else
		{
			// if there is no allocator the filter has not been connected.
			return E_NOINTERFACE;
		}
	}

	if (riid == IID_IAMStreamConfig)
	{
		return GetInterface((IAMStreamConfig*)this, ppv);
	}

	if (riid == IID_IAMBufferNegotiation)
	{
		return GetInterface((IAMBufferNegotiation*)this, ppv);
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
HRESULT CDecklinkPushPin::GetMediaType(CMediaType *pMediaType)
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
HRESULT CDecklinkPushPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
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
		pRequest->cbAlign = 16;	// e.g. SSE2 memory alignment requirements

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
HRESULT CDecklinkPushPin::FillBuffer(IMediaSample *pSample)
{
    BYTE *pData;
    long cbData;

    CheckPointer(pSample, E_POINTER);

    CAutoLock cAutoLockShared(&m_cSharedState);

	pSample->GetPointer(&pData);
	cbData = pSample->GetActualDataLength();

    // Check that we're still using video
    ASSERT(m_mt.formattype == FORMAT_VideoInfo);

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
// OnThreadStartPlay
//
HRESULT CDecklinkPushPin::OnThreadStartPlay(void)
{
	// reset the streaming counters
	m_rtNextValidStart = 0;
	m_iFrameNumber = 0;
	return S_OK;
}

// -------------------------------------------------------------------------
// InitAllocator
// Overriden base output pin to allow us to create our own allocator
//
HRESULT CDecklinkPushPin::InitAllocator(IMemAllocator** ppAllocator)
{
	return CoCreateInstance(CLSID_CustomMemAllocator, 0, CLSCTX_INPROC_SERVER, IID_IMemAllocator, (void**)ppAllocator);
}

// -------------------------------------------------------------------------
// DecideAllocator
// We override the base stream negotiation to deny downstream pins request and
// supply our own allocator
//
HRESULT CDecklinkPushPin::DecideAllocator(IMemInputPin* pPin, IMemAllocator** ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;

    // get downstream prop request
    // the derived class may modify this in DecideBufferSize, but
    // we assume that he will consistently modify it the same way,
    // so we only get it once
    ALLOCATOR_PROPERTIES prop;
    ZeroMemory(&prop, sizeof(prop));

    // whatever he returns, we assume prop is either all zeros
    // or he has filled it out.
    pPin->GetAllocatorRequirements(&prop);

    // if he doesn't care about alignment, then set it to 1
    if (prop.cbAlign == 0)
    {
        prop.cbAlign = 1;
    }

	// *** We ignore input pins allocator because WE make the frame buffers ***

    // Try the custom allocator
    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr))
    {
        // note - the properties passed here are in the same
        // structure as above and may have been modified by
        // the previous call to DecideBufferSize
        hr = DecideBufferSize(*ppAlloc, &prop);
        if (SUCCEEDED(hr))
        {
            hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
            if (SUCCEEDED(hr))
            {
                return NOERROR;
            }
        }
    }

    // Likewise we may not have an interface to release
    if (*ppAlloc)
	{
        (*ppAlloc)->Release();
        *ppAlloc = NULL;
    }
    return hr;
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
STDMETHODIMP CDecklinkPushPin::GetFormat(AM_MEDIA_TYPE** ppamt)
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
STDMETHODIMP CDecklinkPushPin::SetFormat(AM_MEDIA_TYPE* pamt)
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

//---------------------------------------------------------------
// GetNumberOfCapabilities
// Returns the number of video formats supported by the underlying
// HW object.
//
STDMETHODIMP CDecklinkPushPin::GetNumberOfCapabilities(int* piCount, int* piSize)
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
STDMETHODIMP CDecklinkPushPin::GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppamt, BYTE* pSCC)
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
			VIDEO_STREAM_CONFIG_CAPS* pscc = (VIDEO_STREAM_CONFIG_CAPS *)pSCC;
			VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER *)pamt->pbFormat;

			ZeroMemory(pSCC, sizeof(VIDEO_STREAM_CONFIG_CAPS));
			pscc->guid = pamt->formattype;
			pscc->InputSize.cx = pvih->bmiHeader.biWidth;
			pscc->InputSize.cy = pvih->bmiHeader.biHeight;
			pscc->MinCroppingSize = pscc->MaxCroppingSize = pscc->InputSize;
			pscc->MinOutputSize = pscc->MaxOutputSize = pscc->InputSize;
			pscc->MinBitsPerSecond = pscc->MaxBitsPerSecond = pvih->dwBitRate;
			pscc->MinFrameInterval = pscc->MaxFrameInterval = pvih->AvgTimePerFrame;
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
STDMETHODIMP CDecklinkPushPin::SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES* pprop)
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
STDMETHODIMP CDecklinkPushPin::GetAllocatorProperties(ALLOCATOR_PROPERTIES* pprop)
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

//------------------------------------------------------------------------------
// CDecklinkPushSource Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CreateInstance
//
CUnknown* WINAPI CDecklinkPushSource::CreateInstance(IUnknown* pUnk, HRESULT* phr)
{
	CDecklinkPushSource *pNewFilter = new CDecklinkPushSource(pUnk, phr);

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
CDecklinkPushSource::CDecklinkPushSource(IUnknown* pUnk, HRESULT* phr)
           : CSource(NAME("Decklink Push Source"), pUnk, CLSID_DecklinkPushSource)
{
	// The pin magically adds itself to our pin array.
	m_pPin = new CDecklinkPushPin(NAME("Decklink Push Source Pin"), phr, this, L"Out");

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
CDecklinkPushSource::~CDecklinkPushSource()
{
    delete m_pPin;
}

//------------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
STDMETHODIMP CDecklinkPushSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
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
STDMETHODIMP CDecklinkPushSource::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE* pState)
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
