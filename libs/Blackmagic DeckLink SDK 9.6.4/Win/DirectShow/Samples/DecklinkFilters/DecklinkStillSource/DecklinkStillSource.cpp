//------------------------------------------------------------------------------
// DecklinkStillSource.cpp
//
// Desc: DirectShow sample code - External push mode source filter
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//
// Read readme.txt!
//
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "DecklinkStillSource.h"
#include "DecklinkFilters_h.h"

//------------------------------------------------------------------------------
static const int DBG_ERR = 1;
static const int DBG_VID = 2;
static const int DBG_WRN = 2;

//------------------------------------------------------------------------------
// CDecklinkStillSourcePin Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Constructor
//
CDecklinkStillSourcePin::CDecklinkStillSourcePin(HRESULT* phr, CSource* pFilter)
      : CSourceStream(NAME("Decklink Still Source"), phr, pFilter, L"Video")
        , m_iFrameNumber(0)
        , m_rtFrameLength(FPS_2997)
        , m_fLastSampleDiscarded(FALSE)
{
}


//------------------------------------------------------------------------------
// Destructor
//
CDecklinkStillSourcePin::~CDecklinkStillSourcePin()
{   
    DbgLog((LOG_TRACE, 3, TEXT("Frames written %d"),m_iFrameNumber));
}

// -------------------------------------------------------------------------
// Reveal our property page, persistance, and control interfaces
//
STDMETHODIMP CDecklinkStillSourcePin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IAMStreamControl)
    {
		return GetInterface((IAMStreamControl*)this, ppv);
    } 

	return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

//------------------------------------------------------------------------------
// CompleteConnect
// Check the connection media type for the average time per frame.
HRESULT CDecklinkStillSourcePin::CompleteConnect(IPin *pReceivePin)
{
	HRESULT hr = CSourceStream::CompleteConnect(pReceivePin);
	if (SUCCEEDED(hr))
	{
		m_rtFrameLength = CUtils::GetAvgTimePerFrame(&m_mt);
	}
	return hr;
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
HRESULT CDecklinkStillSourcePin::GetMediaType(CMediaType* pMediaType)
{
    CheckPointer(pMediaType, E_POINTER);
    CAutoLock cAutoLock(m_pFilter->pStateLock());

	CDecklinkStillSource* pFilter = (CDecklinkStillSource*)m_pFilter;
    return pFilter->GetMediaType(pMediaType);
}

//------------------------------------------------------------------------------
// DecideBufferSize
// Put in a request to the allocator for number of buffers to allocate, their size,
// alignment, etc.
HRESULT CDecklinkStillSourcePin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pRequest, E_POINTER);

    HRESULT hr = S_OK;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(&m_mt);
	if (pbmih)
	{
		if (pRequest->cBuffers == 0)
		{
			pRequest->cBuffers = 2;
		}
		pRequest->cbBuffer = pbmih->biSizeImage;

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
HRESULT CDecklinkStillSourcePin::FillBuffer(IMediaSample *pSample)
{
    CheckPointer(pSample, E_POINTER);
    CAutoLock cAutoLockShared(&m_cSharedState);
    HRESULT hr = S_OK;

	BYTE *pData;
	long cbData;

	hr = pSample->GetPointer(&pData);
	if (SUCCEEDED(hr))
	{
		cbData = pSample->GetActualDataLength();

		// Check that we're still using video
		ASSERT(m_mt.formattype == FORMAT_VideoInfo);

		CDecklinkStillSource* pFilter = (CDecklinkStillSource*)m_pFilter;
	
		unsigned char* pFrame = NULL;
		unsigned long cbFrame = 0;
		hr = pFilter->GetBuffer(&pFrame, &cbFrame);
		if (pFrame && SUCCEEDED(hr))
		{
			if (cbData > (long)cbFrame)
			{
				cbData = cbFrame;
				pSample->SetActualDataLength(cbData);
			}
			memcpy(pData, pFrame, cbData);
		}
	
		CRefTime rtStream;
		m_pFilter->StreamTime(rtStream);

		REFERENCE_TIME rtStart = m_iFrameNumber * m_rtFrameLength;
		REFERENCE_TIME rtStop  = rtStart + m_rtFrameLength;
		m_rtNextValidStart = rtStop;

		pSample->SetTime(&rtStart, &rtStop);
		DbgLog((LOG_TRACE, DBG_VID, TEXT("VID_STILL: %I64d [%I64d  %I64d]"), rtStream.m_time, rtStart, rtStop));

		// set sample media times
		rtStart = m_iFrameNumber;
		rtStop = rtStart + 1;
		pSample->SetMediaTime(&rtStart, &rtStop);

		m_iFrameNumber++;

		// Set TRUE on every sample for uncompressed frames
		pSample->SetSyncPoint(TRUE);
	}

	// check the streaming state
	int iStreamState = CheckStreamState(pSample);
	if (STREAM_FLOWING == iStreamState) 
	{
		// deliver the sample
		if (m_fLastSampleDiscarded)
		{
			pSample->SetDiscontinuity(TRUE);
		}
		m_fLastSampleDiscarded = FALSE;
	} 
	else 
	{
		// discard this sample, do not deliver it
		m_fLastSampleDiscarded = TRUE;
		hr = S_FALSE;	// indicate EOS to downstream filters
	}

	return hr;
}

// -------------------------------------------------------------------------
// OnThreadStartPlay
//
HRESULT CDecklinkStillSourcePin::OnThreadStartPlay(void)
{
	// reset the streaming counters
	m_rtNextValidStart = 0;
	m_iFrameNumber = 0;
	return S_OK;
}

// -------------------------------------------------------------------------
// BeginFlush
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkStillSourcePin::BeginFlush(void)
{
	Flushing(TRUE);
	return CSourceStream::BeginFlush();
}

// -------------------------------------------------------------------------
// EndFlush
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkStillSourcePin::EndFlush(void)
{
	Flushing(FALSE);
	return CSourceStream::EndFlush();
}

//------------------------------------------------------------------------------
// CDecklinkPushSource Class
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CreateInstance
//
CUnknown* WINAPI CDecklinkStillSource::CreateInstance(IUnknown* pUnk, HRESULT* phr)
{
	CDecklinkStillSource* pFilter = new CDecklinkStillSource(pUnk, phr);

	if (phr && (NULL == pFilter))
	{
		*phr = E_OUTOFMEMORY;
	}

	return pFilter;
}

//------------------------------------------------------------------------------
// Constructor
//
CDecklinkStillSource::CDecklinkStillSource(IUnknown* pUnk, HRESULT* phr)
           : CSource(NAME("CDecklinkStillSource"), pUnk, CLSID_DecklinkStillSource)
			, m_pPin(NULL)
			, m_filename(L"<Select File>")
			, m_hFile(NULL)
			, m_bActiveBuffer(FALSE)
			, m_bSwapBuffer(FALSE)
{
	ZeroMemory(m_pFrameBuffer, sizeof(m_pFrameBuffer));
	m_mediaType.InitMediaType();

	// create the filter pin
	m_pPin = new CDecklinkStillSourcePin(phr, this);
	if (phr && (NULL == m_pPin))
	{
		*phr = E_OUTOFMEMORY;
	}
}

//------------------------------------------------------------------------------
// Destructor
//
CDecklinkStillSource::~CDecklinkStillSource()
{
	int count = sizeof(m_pFrameBuffer) / sizeof(m_pFrameBuffer[0]);
	for (int buffer=0; buffer<count; ++buffer)
	{
		SAFE_DELETE(m_pFrameBuffer[buffer]);
	}
	SAFE_DELETE(m_pPin);
}

// -------------------------------------------------------------------------
// Reveal our property page, persistance, and control interfaces
//
STDMETHODIMP CDecklinkStillSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IAMFilterMiscFlags)
	{
		return GetInterface((IAMFilterMiscFlags*)this, ppv);
	}

    if (riid == IID_IFileSourceFilter)
    {
		return GetInterface((IFileSourceFilter*)this, ppv);
    } 

    if (riid == IID_ISpecifyPropertyPages)
    {
		return GetInterface((ISpecifyPropertyPages*)this, ppv);
    } 

	return CSource::NonDelegatingQueryInterface(riid, ppv);
}

//------------------------------------------------------------------------------
// For IAMStreamControl implementation
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SetSyncSource
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkStillSource::SetSyncSource(IReferenceClock* pClock)
{
	m_pPin->SetSyncSource(pClock);
	return CSource::SetSyncSource(pClock);
}

//------------------------------------------------------------------------------
// JoinFilterGraph
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkStillSource::JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName)
{
	HRESULT hr = CSource::JoinFilterGraph(pGraph, pName);

	if (SUCCEEDED(hr)) 
	{
		m_pPin->SetFilterGraph(m_pSink);
	}

	return hr;
}

//------------------------------------------------------------------------------
// Run
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkStillSource::Run(REFERENCE_TIME tStart)
{
	m_pPin->NotifyFilterState(State_Running, tStart);
	return CSource::Run(tStart); // Call the filter base class.
}

//------------------------------------------------------------------------------
// Pause
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkStillSource::Pause()
{
	m_pPin->NotifyFilterState(State_Paused);
	return CSource::Pause();
}

//------------------------------------------------------------------------------
// Stop
// Overridden to provide IAMStreamControl implementation
STDMETHODIMP CDecklinkStillSource::Stop()
{
	m_pPin->NotifyFilterState(State_Stopped);
	return CSource::Stop();
}

//------------------------------------------------------------------------------
// IFileSourceFilter interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Load
// Load the given filename.  This implementation differs slightly from the 'official'
// interface in that it can be called more than once in order to change source still.
// It is also possible to create synthetic frames by specifying a keyword as the filename.
STDMETHODIMP CDecklinkStillSource::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
	HRESULT hr = S_OK;

	if (pszFileName)
	{
		m_filename = pszFileName;

		BITMAPINFOHEADER bmih = {0};
		if (pmt && (basic_string<WCHAR>::npos != m_filename.rfind(L"BMD_Checker")))
		{
			BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(pmt);
			if (pbmih)
			{
				// create the synthetic file from the supplied media type
				bmih = *pbmih;
				if ((BI_RGB != bmih.biCompression) || (1 != bmih.biPlanes))
				{
					// create a bitmap format suitable for keying
					bmih.biPlanes = 1;
					bmih.biBitCount = 32;
					bmih.biCompression = BI_RGB;
					bmih.biSizeImage = CUtils::GetImageSize(&bmih);
				}

				// set up the back buffer, if the pin is already streaming this
				// will eventually appear
				SAFE_DELETE(m_pFrameBuffer[!m_bActiveBuffer]);
				
				// copy bitmap data into inactive buffer
				m_pFrameBuffer[!m_bActiveBuffer] = new unsigned char [bmih.biSizeImage];
				if (m_pFrameBuffer[!m_bActiveBuffer])
				{
					hr = CreateCheckerFrame(&bmih, m_pFrameBuffer[!m_bActiveBuffer], bmih.biSizeImage);
					if (SUCCEEDED(hr))
					{
						m_bSwapBuffer = TRUE;
					}
				}
				else
				{
					hr = E_OUTOFMEMORY;
				}
			}
		}
		else
		{
			// attempt to open the 'real' filename
			m_hFile = CreateFile(CW2CT(m_filename.c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_hFile)
			{
				// attempt to process the bitmap image
				// TODO: Parser other still files, e.g. TGA, JPG, etc.
				BITMAPFILEHEADER bmfh = {0};
				hr = Read(&bmfh, sizeof(bmfh));
				if (SUCCEEDED(hr))
				{
					// check that this is a bitmap file
					if ('MB' == bmfh.bfType)
					{
						// NOTE: Always assume a BITMAPINFOHEADER, this could be wrong!
						hr = Read(&bmih, sizeof(bmih));
						if (SUCCEEDED(hr))
						{
							if ((bmih.biBitCount == 32) && (1 == bmih.biPlanes) && (BI_RGB == bmih.biCompression))
							{
								// set up the back buffer, if the pin is already streaming this
								// will eventually appear
								SAFE_DELETE(m_pFrameBuffer[!m_bActiveBuffer]);
								
								// copy bitmap data into inactive buffer
								m_pFrameBuffer[!m_bActiveBuffer] = new unsigned char [bmih.biSizeImage];
								if (m_pFrameBuffer[!m_bActiveBuffer])
								{
									hr = Read(m_pFrameBuffer[!m_bActiveBuffer], bmih.biSizeImage);
									if (SUCCEEDED(hr))
									{
										m_bSwapBuffer = TRUE;
									}
								}
								else
								{
									hr = E_OUTOFMEMORY;
								}
							}
							else
							{
								hr = E_FAIL;
							}
						}
					}
					else
					{
						hr = E_FAIL;
					}
				}
				
				CloseHandle(m_hFile);
				m_hFile = NULL;
			}
			else
			{
				hr = AmHresultFromWin32(GetLastError());
			}
		}

		if (SUCCEEDED(hr))
		{
			CAutoLock lock(&m_critSec);

			// set the media type
			m_mediaType.SetType(&MEDIATYPE_Video);
			m_mediaType.SetSubtype(&MEDIASUBTYPE_ARGB32);
			m_mediaType.SetFormatType(&FORMAT_VideoInfo);
			m_mediaType.SetSampleSize(bmih.biSizeImage);

			VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)m_mediaType.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
			ZeroMemory(pvih, sizeof(VIDEOINFOHEADER));
			pvih->bmiHeader = bmih;

			// set the frame rate of the still source
			REFERENCE_TIME rtFrameLength;
			if (pmt)
			{
				// use the supplied mediatype to set the frame rate of the still source
				rtFrameLength = CUtils::GetAvgTimePerFrame(pmt);
			}
			else
			{
				// default
				if (1080 == pvih->bmiHeader.biHeight)
				{
					rtFrameLength = FPS_2997;
				}
				else if (720 == pvih->bmiHeader.biHeight)
				{
					rtFrameLength = FPS_5994;
				}
				else if (576 == pvih->bmiHeader.biHeight)
				{
					rtFrameLength = FPS_25;
				}
				else
				{
					rtFrameLength = FPS_2997;
				}
			}
			pvih->AvgTimePerFrame = rtFrameLength;

			if (0 == pvih->bmiHeader.biSizeImage)
			{
				pvih->bmiHeader.biSizeImage = pvih->bmiHeader.biWidth * pvih->bmiHeader.biHeight * pvih->bmiHeader.biBitCount / 8;
			}
			pvih->dwBitRate = pvih->bmiHeader.biSizeImage * (DWORD)((float)UNITS / pvih->AvgTimePerFrame) * 8;
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

//------------------------------------------------------------------------------
// GetCurFile
// Return the filename and optionally the media type to the caller.  Allocate
// filename and media type format block, caller will free this memory.
STDMETHODIMP CDecklinkStillSource::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
	HRESULT hr = S_OK;
	
	if (ppszFileName)
	{
		// allocate space for the filename
		int len = m_filename.length() + 1;
		*ppszFileName = (LPOLESTR)CoTaskMemAlloc(len * sizeof(WCHAR));
		if (ppszFileName)
		{
			ZeroMemory(*ppszFileName, len * sizeof(WCHAR));
			// safely copy filename
			StringCchCopyW(*ppszFileName, m_filename.length() + 1, m_filename.c_str());
			if (pmt)
			{
				// fill out the still media type
				CopyMediaType(pmt, (const AM_MEDIA_TYPE*)&m_mediaType);
			} 
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Read
// Read from the file handle
HRESULT CDecklinkStillSource::Read(LPVOID lpBuffer, DWORD cbBuffer)
{
	HRESULT hr = S_OK;
	DWORD dwBytesRead = 0;
	if (m_hFile && lpBuffer)
	{
		if ((0 == ReadFile(m_hFile, lpBuffer, cbBuffer, &dwBytesRead, NULL)) || (0 == dwBytesRead))
		{
			hr = AmHresultFromWin32(GetLastError());
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//------------------------------------------------------------------------------
// CreateCheckerFrame
// Fill the supplied buffer with a checker pattern of white and grey squares
// which are scaled according to the size of the frame.
HRESULT CDecklinkStillSource::CreateCheckerFrame(const BITMAPINFOHEADER* pbmih, unsigned char* pBuffer, unsigned long cbBuffer)
{
	HRESULT hr = S_OK;
	if (pBuffer && cbBuffer)
	{
		// fill with a grey checkerboard (BGRA)
		bool bCheckerFlag = FALSE;
		const int checkerSize = (int)((double)pbmih->biWidth * 0.0278 + 0.5);
		unsigned long RowBytes = CUtils::GetImageSize(pbmih) / pbmih->biHeight;
		unsigned char* pNextRow = pBuffer + (pbmih->biHeight - 1) * RowBytes;
		const unsigned long pixelData[2] = {0xFFFFFFFF, 0xFFD0D0C0};
	
		// flip the 'polarity' of the checker pattern
		for (LONG height=0; height<pbmih->biHeight; ++height)
		{
			if (height && (0 == (height % checkerSize)))
			{
				bCheckerFlag = !bCheckerFlag;	// change the starting colour for the next checker row
			}
			bool bTemp = bCheckerFlag;

			unsigned long* pPixel = (unsigned long*)pNextRow;
			pNextRow -= RowBytes;
			
			for (LONG pixel=0; pixel<pbmih->biWidth; ++pixel)
			{
				// fill a line, alternating between the checker colours
				*pPixel++ = pixelData[bTemp];
				if (pixel && (0 == (pixel % checkerSize)))
				{
					bTemp = !bTemp;
				}
			}
		}
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}
	return hr;
}

//------------------------------------------------------------------------------
// Public methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// GetMediaType
// Return the media type of the bitmap still source to the caller
HRESULT CDecklinkStillSource::GetMediaType(CMediaType* pmt)
{
	HRESULT hr = S_OK;

	if (pmt)
	{
		CAutoLock lock(&m_critSec);
		hr = pmt->Set(m_mediaType);
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}

//------------------------------------------------------------------------------
// GetBuffer
// Return the active frame buffer.  With this filter it is possible to repeatedly
// call the Load method on the IFileSourceFilter interface to change/update the
// bitmap still source.
HRESULT CDecklinkStillSource::GetBuffer(unsigned char** ppBuffer, unsigned long* pcbBuffer)
{
	HRESULT hr = S_OK;

	if (ppBuffer)
	{
		CAutoLock lock(&m_critSec);	// lock the buffer control flags
		if (m_bSwapBuffer)
		{
			// change the active buffer
			m_bActiveBuffer = !m_bActiveBuffer;
			m_bSwapBuffer = FALSE;
		}

		*ppBuffer = m_pFrameBuffer[m_bActiveBuffer];
		if (*ppBuffer)
		{
			BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(&m_mediaType);
			if (pbmih)
			{
				*pcbBuffer = pbmih->biSizeImage;
			}
		}
		else
		{
			hr = E_FAIL;
		}		
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return hr;
}
