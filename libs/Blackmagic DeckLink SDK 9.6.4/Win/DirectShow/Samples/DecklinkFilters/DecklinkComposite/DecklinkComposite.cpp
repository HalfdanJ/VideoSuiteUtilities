//------------------------------------------------------------------------------
// DecklinkComposite.cpp
//
// Desc: DirectShow sample code - Illustrates a basic frame composition by taking
//								  a non-SDI source frame and compositing onto an
//								  SDI frame size for rendering with the Decklink
//								  video renderer.
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "DecklinkFilters_h.h"
#include "DecklinkComposite.h"

//------------------------------------------------------------------------------
// CreateInstance
// Provide the way for COM to create a CDecklinkComposite object
CUnknown* WINAPI CDecklinkComposite::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
    HRESULT hr = S_OK;
    CDecklinkComposite* pFilter = static_cast<CDecklinkComposite*>(new CDecklinkComposite(NAME("Decklink Composite filter"), punk, &hr));

    if (NULL == pFilter)
    {
        *phr = E_OUTOFMEMORY;
    }

	if (FAILED(hr) && phr)
	{
		*phr = hr;
	}

    return pFilter;
}

//------------------------------------------------------------------------------
// Constructor
// 
CDecklinkComposite::CDecklinkComposite(TCHAR* tszName, LPUNKNOWN punk, HRESULT* phr)
    : CTransformFilter(tszName, punk, CLSID_DecklinkComposite)
{
	m_mtOutput.InitMediaType();
}

//------------------------------------------------------------------------------
// Destructor
// 
CDecklinkComposite::~CDecklinkComposite()
{
}

//------------------------------------------------------------------------------
// NonDelegatingQueryInterface
//
STDMETHODIMP CDecklinkComposite::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
	}

	if (riid == IID_IAMStreamConfig)
	{
		return GetInterface(static_cast<IAMStreamConfig*>(this), ppv);
	}

    return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}

//------------------------------------------------------------------------------
// Transform
// 
HRESULT CDecklinkComposite::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
    CheckPointer(pIn, E_POINTER);
    CheckPointer(pOut, E_POINTER);
    HRESULT hr = S_OK;

    // transform the sample data
    BYTE* pSrc, *pDst;
    long cbSrc = pIn->GetActualDataLength();
    long cbDst = pOut->GetActualDataLength();

    pIn->GetPointer(&pSrc);
    pOut->GetPointer(&pDst);

	if (pSrc && pDst)
	{
		BITMAPINFOHEADER* pbmihSrc = CUtils::GetBMIHeader(m_pInput->CurrentMediaType());
		BITMAPINFOHEADER* pbmihDst = CUtils::GetBMIHeader(m_pOutput->CurrentMediaType());

		if (pbmihSrc && pbmihDst)
		{
			// TODO: for RGB formats honour -ve heights
			LONG srcHeight = abs(pbmihSrc->biHeight);
			LONG dstHeight = abs(pbmihDst->biHeight);
			unsigned long srcRowBytes = cbSrc / srcHeight;
			unsigned long dstRowBytes = cbDst / dstHeight;
			unsigned long cbPixel = CUtils::GetImageSize(pbmihSrc) / pbmihSrc->biWidth / pbmihSrc->biHeight;
			unsigned long cLines = 0, cbLine = 0;
			
			if (srcRowBytes && dstRowBytes)
			{
				if (pbmihSrc->biWidth < pbmihDst->biWidth)
				{
					// source frame is smaller than the destination frame,
					// center the source frame in the destination frame
					pDst += (((pbmihDst->biWidth - pbmihSrc->biWidth) * cbPixel) >> 1);	// provide horizontal offset into destination buffer
					cbLine = cbPixel * pbmihSrc->biWidth;
				}
				else
				{
					// source frame is larger than than the destination frame,
					// show a small portion of the source in the destination
					pSrc += (((pbmihSrc->biWidth - pbmihDst->biWidth) * cbPixel) >> 1);	// provide horizontal offset into source buffer
					cbLine = cbPixel * pbmihDst->biWidth;
				}

				LONG offsetToPreventFieldSwap = 0;	// modifier to ensure a field swap does not take place when copying interlaced media
				if (srcHeight < dstHeight)
				{
					// source frame is smaller than the destination frame,
					// center the source frame in the destination frame
					offsetToPreventFieldSwap = (dstHeight - srcHeight) % 2;
					pDst += (((dstHeight - srcHeight - offsetToPreventFieldSwap) >> 1) * dstRowBytes);	// provide vertical offset into destination buffer
					cLines = srcHeight;
				}
				else
				{
					// source frame is larger than than the destination frame,
					// show a small portion of the source in the destination
					offsetToPreventFieldSwap = (srcHeight - dstHeight) % 2;
					pSrc += (((srcHeight - dstHeight - offsetToPreventFieldSwap) >> 1) * srcRowBytes);	// provide vertical offset into source buffer
					cLines = dstHeight;
				}

				if (BI_RGB == pbmihSrc->biCompression)
				{
					// honour -ve height RGB formats by inverting the frame, start at the top of the buffer
					// and move down through it, note that RGB formats are bottom up for +ve heights
					if (pbmihSrc->biHeight < 0)
					{
						pSrc = pSrc + (srcRowBytes * (srcHeight - 1));
						srcRowBytes = ~srcRowBytes + 1;
					}

					if (pbmihDst->biHeight < 0)
					{
						pDst = pDst + (dstRowBytes * (dstHeight - 1));
						dstRowBytes = ~dstRowBytes + 1;
					}
				}

				// copy the source lines into the destination buffer
				for (unsigned long line=0; line<cLines; ++line)
				{
					memcpy(pDst, pSrc, cbLine);
					pSrc += srcRowBytes;
					pDst += dstRowBytes;
				}
			}
		}
		else
		{
			hr = E_POINTER;
		}
		
		if (SUCCEEDED(hr))
		{
			pOut->SetActualDataLength(pbmihDst->biSizeImage);

		    // Copy the sample times
		    REFERENCE_TIME rtStart, rtEnd;
			if (SUCCEEDED(pIn->GetTime(&rtStart, &rtEnd)))
			{
				CRefTime rtStreamOut;
				StreamTime(rtStreamOut);
				TCHAR buf[128];
				StringCchPrintf(buf, 128, TEXT("VID: %10I64d [%10I64d  %10I64d]  %10I64d"), rtStreamOut, rtStart, rtEnd);
				OutputDebugString(buf);
				pOut->SetTime(&rtStart, &rtEnd);
			}

			LONGLONG rtMediaStart, rtMediaEnd;
			if (SUCCEEDED(pIn->GetMediaTime(&rtMediaStart, &rtMediaEnd)))
			{
				pOut->SetMediaTime(&rtMediaStart, &rtMediaEnd);
			}

			// copy the sync property
			if (S_OK == pIn->IsSyncPoint())
			{
				pOut->SetSyncPoint(TRUE);
			}
			else if (S_FALSE == pIn->IsSyncPoint())
			{
				pOut->SetSyncPoint(FALSE);
			}

			// copy the preroll property
			if (S_OK == pIn->IsPreroll())
			{
				pOut->SetPreroll(TRUE);
			}
			else if (S_FALSE == pIn->IsPreroll())
			{
				pOut->SetPreroll(FALSE);
			}

			// copy the discontinuity property
			if (S_OK == pIn->IsDiscontinuity())
			{
				pOut->SetDiscontinuity(TRUE);
			}
			else if (S_FALSE == pIn->IsDiscontinuity())
			{
				pOut->SetDiscontinuity(FALSE);
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
// CheckInputType
// Some basic checks on preferred formats.
HRESULT CDecklinkComposite::CheckInputType(const CMediaType* mtIn)
{
	CheckPointer(mtIn, E_POINTER);
	HRESULT hr = S_FALSE;
	// Make some very rudimentary checks on the media type
	if (MEDIATYPE_Video == mtIn->majortype)
	{
		if ((MEDIASUBTYPE_UYVY == mtIn->subtype) || (IID_MEDIASUBTYPE_HDYC == mtIn->subtype) || (MEDIASUBTYPE_YUY2 == mtIn->subtype) || 
			(MEDIASUBTYPE_RGB24 == mtIn->subtype) || (MEDIASUBTYPE_RGB32 == mtIn->subtype) || (MEDIASUBTYPE_ARGB32 == mtIn->subtype))
		{
			if ((FORMAT_VideoInfo == mtIn->formattype) || (FORMAT_VideoInfo2 == mtIn->formattype))
			{
				BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(*mtIn);
				if (pbmih)
				{
					if (('YVYU' == pbmih->biCompression) || ('2YUY' == pbmih->biCompression) ||
						(((24 == pbmih->biBitCount) || (32 == pbmih->biBitCount)) && (BI_RGB == pbmih->biCompression)))
					{
						// acceptable media type
						hr = S_OK;
					}
				}
			}
		}
	}
		
	return hr;
}

//------------------------------------------------------------------------------
// CheckTransform
//
HRESULT CDecklinkComposite::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
    CheckPointer(mtIn, E_POINTER);
    CheckPointer(mtOut, E_POINTER);

    HRESULT hr = CheckInputType(mtIn);
    if (SUCCEEDED(hr))
    {
		if (mtOut->majortype == mtIn->majortype)
		{
			if (mtOut->subtype == mtIn->subtype)
			{
				if ((FORMAT_VideoInfo == mtOut->formattype) || (FORMAT_VideoInfo2 == mtOut->formattype))
				{
					BITMAPINFOHEADER* pbmihIn = CUtils::GetBMIHeader(*mtIn);
					BITMAPINFOHEADER* pbmihOut = CUtils::GetBMIHeader(*mtOut);
					if (pbmihIn && pbmihOut)
					{
						if (pbmihOut->biCompression == pbmihIn->biCompression)
						{
							// acceptable media type
							hr = S_OK;
						}
					}
				}
			}
		}
    }
	return hr;
}

//------------------------------------------------------------------------------
// DecideBufferSize
//
HRESULT CDecklinkComposite::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pProperties, E_POINTER);
	HRESULT hr = S_OK;

    // Is the input pin connected
    if (m_pInput->IsConnected())
	{
		BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(m_pOutput->CurrentMediaType());
		if (pbmih)
		{
			pProperties->cBuffers = 1;
			pProperties->cbBuffer = CUtils::GetImageSize(pbmih);
			ASSERT(pProperties->cbBuffer);
//			pProperties->cbAlign = 16;	// SSE2 alignment requirement

			// Ask the allocator to reserve us some sample memory, NOTE the function
			// can succeed (that is return NOERROR) but still not have allocated the
			// memory that we requested, so we must check we got whatever we wanted
			ALLOCATOR_PROPERTIES Actual = {0};
			hr = pAlloc->SetProperties(pProperties, &Actual);
			if (SUCCEEDED(hr))
			{
				ASSERT(Actual.cBuffers >= 1);
				if ((pProperties->cBuffers > Actual.cBuffers) || (pProperties->cbBuffer > Actual.cbBuffer))
				{
					hr = E_FAIL;
				}
			}
		}
		else
		{
			hr = E_POINTER;
		}
	}
	else
    {
        hr = E_UNEXPECTED;
    }

	return hr;
}

//------------------------------------------------------------------------------
// GetMediaType
//
HRESULT CDecklinkComposite::GetMediaType(int iPosition, CMediaType* pMediaType)
{
	CheckPointer(pMediaType, E_POINTER);
	HRESULT hr = S_OK;
	
	// Is the input pin connected
	if (m_pInput->IsConnected())
	{
		// This should never happen
		if (iPosition >= 0)
		{
			// Do we have more items to offer
			if (iPosition == 0)
			{
				hr = pMediaType->Set(m_mtOutput);
			}
			else
			{
				hr  = VFW_S_NO_MORE_ITEMS;
			}
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}
	else
	{
		hr = E_UNEXPECTED;
	}

	return hr;
}

//------------------------------------------------------------------------------
// SetMediaType
//
HRESULT CDecklinkComposite::SetMediaType(PIN_DIRECTION direction, const CMediaType* pmt)
{
	HRESULT hr = S_OK;
	
	if (pmt)
	{
		if (PINDIR_INPUT == direction)
		{
			// once the input pin has been connected, set the default
			// output media type to match the input media type
			m_mtOutput.Set(*pmt);
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//------------------------------------------------------------------------------
// ISpecifyPropertyPages interface
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// GetPages
//
STDMETHODIMP CDecklinkComposite::GetPages(CAUUID* pPages)
{
	if (pPages == NULL)
		return E_POINTER;

	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));

	if (pPages->pElems == NULL) 
		return E_OUTOFMEMORY;

	pPages->pElems[0] = CLSID_DecklinkCompositeProperties;
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
STDMETHODIMP CDecklinkComposite::GetFormat(AM_MEDIA_TYPE** ppamt)
{
	HRESULT hr = S_OK;
	CheckPointer(ppamt, E_POINTER);

	if (m_pInput->IsConnected())
	{
		*ppamt = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
		if (*ppamt)
		{
			AM_MEDIA_TYPE *pamt = CreateMediaType((const AM_MEDIA_TYPE*)&m_mtOutput);
			*ppamt = pamt;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		hr = VFW_E_NOT_CONNECTED;
	}

	return hr;
}

//---------------------------------------------------------------
// SetFormat
//
STDMETHODIMP CDecklinkComposite::SetFormat(AM_MEDIA_TYPE* pamt)
{
	HRESULT hr = S_OK;
	CheckPointer(pamt, E_POINTER);

	if (m_pInput->IsConnected())
	{
		if (IsStopped())
		{
			// TODO: Check that this is an acceptable media type
			m_mtOutput.Set(*pamt);
		}
		else
		{
			hr = VFW_E_NOT_STOPPED;
		}
	}
	else
	{
		hr = VFW_E_NOT_CONNECTED;
	}

	return hr;
}

//---------------------------------------------------------------
// GetNumberOfCapabilities
// Returns the number of video formats supported by the underlying
// HW object.
//
STDMETHODIMP CDecklinkComposite::GetNumberOfCapabilities(int* piCount, int* piSize)
{
	CheckPointer(piCount, E_POINTER);
	CheckPointer(piSize, E_POINTER);
	HRESULT hr = S_OK;

	if (m_pInput->IsConnected())
	{
		*piCount = 1;
		*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
	}
	else
	{
		hr = VFW_E_NOT_CONNECTED;
	}

	return hr;
}

//---------------------------------------------------------------
// GetStreamCaps
// Returns the media format at the specified index.
//
STDMETHODIMP CDecklinkComposite::GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppamt, BYTE* pSCC)
{
	HRESULT hr = S_OK;
	CheckPointer(ppamt, E_POINTER);
	CheckPointer(pSCC, E_POINTER);
	
	if (m_pInput->IsConnected())
	{
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

			AM_MEDIA_TYPE *pamt = CreateMediaType((const AM_MEDIA_TYPE*)&m_mtOutput);
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
	}
	else
	{
		hr = VFW_E_NOT_CONNECTED;
	}

	if (FAILED(hr) && (*ppamt))
	{
		CoTaskMemFree(*ppamt);
		*ppamt = NULL;
	}

	return hr;
}

