//------------------------------------------------------------------------------
// DecklinkFieldSwap.cpp
//
// Desc: DirectShow sample code - Illustrates a very basic field swap filter implementation.
//								  Based entirely upon the NullNull filter sample in the 
//								  DirectShow SDK samples.
//------------------------------------------------------------------------------


#include "stdafx.h"

#include "DecklinkFilters_h.h"
#include "DecklinkFieldSwap.h"

//------------------------------------------------------------------------------
// CreateInstance
// Provide the way for COM to create a CDecklinkFieldSwap object
CUnknown* WINAPI CDecklinkFieldSwap::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
    CheckPointer(phr, NULL);
    
    CDecklinkFieldSwap* pNewObject = new CDecklinkFieldSwap(NAME("Decklink field swap filter"), punk, phr);

    if (NULL == pNewObject)
    {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//------------------------------------------------------------------------------
// Transform
// This is where the field swap actually takes place.  Its a very simple trans-in-place operation,
// if we've flagged field swapping, move the buffer contents down by one line.  We lose one active
// line in this process and have to black out the first line.  The MoveMemory operation is fast but
// still not ideal if there is a lot of other processing in the filter graph.
HRESULT CDecklinkFieldSwap::Transform(IMediaSample* pSample)
{
	HRESULT hr = S_OK;
	
	if (pSample)
	{
		AM_MEDIA_TYPE amt = {0};
		if (SUCCEEDED(m_pInput->ConnectionMediaType(&amt)))
		{
			BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(&amt);

			if (576 == pbmih->biHeight)
			{
				// field swap the buffer contents
				long cbData = pSample->GetActualDataLength();
				BYTE* pFrame = NULL;
				hr = pSample->GetPointer(&pFrame);
				if (SUCCEEDED(hr))
				{
					long rowBytes = cbData / pbmih->biHeight;
					// move the buffer contents down by one line to achieve field swap
					MoveMemory(pFrame + rowBytes, pFrame, cbData - rowBytes);
					
					
					// black out the first line
					unsigned long Black[2];
					
					// TODO: Add more complete range of FCCs, e.g. Apple equivalents
					if ((FOURCC('UYVY') == pbmih->biCompression) || (FOURCC('HDYC') == pbmih->biCompression))
					{
						// 8-bit YUV black
						Black[0] = Black[1] = 0x10801080;
					}
					else if (FOURCC('YUY2') == pbmih->biCompression)
					{
						// 8-bit YUV black
						Black[0] = Black[1] = 0x80108010;
					}
					else if (FOURCC('v210') == pbmih->biCompression)
					{
						// 10-bit YUV
						Black[0] = 0x20010200;
						Black[1] = 0x04080040;
					}
					else if (FOURCC('r210') == pbmih->biCompression)
					{
						// 10-bit RGB
						Black[0] = Black[1] = 0x40000104;
					}
					else
					{
						// just assume BI_RGB for all others
						Black[0] = Black[1] = 0x00000000;
					}
					
					unsigned long* pData = reinterpret_cast<unsigned long*>(pFrame);
					int iCount = rowBytes / sizeof(Black);
					for (int i=0; i<iCount; ++i, ++pData)
					{
						CopyMemory(pData, &Black, sizeof(Black));
					}
				}
			}
			else
			{
				// do nothing
			}
			
			FreeMediaType(amt);
		}
		else
		{
			// not connected
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
// Some basic checks on preferred formats.  In this sample enable field swapping for PAL sized frames.
HRESULT CDecklinkFieldSwap::CheckInputType(const CMediaType* mtIn)
{
	// Make some very rudimentary checks on the media type
	// This would be improved by examining the VIDEOINFOHEADER2 interlace flags
	// and processing accordingly.
	if (MEDIATYPE_Video != mtIn->majortype)
		return S_FALSE;
		
	if (FORMAT_VideoInfo != mtIn->formattype)
		return S_FALSE;

	if ((MEDIASUBTYPE_UYVY != mtIn->subtype) && (IID_MEDIASUBTYPE_HDYC != mtIn->subtype) && (MEDIASUBTYPE_YUY2 != mtIn->subtype) &&
		(IID_MEDIASUBTYPE_v210a != mtIn->subtype) && (IID_MEDIASUBTYPE_r210 != mtIn->subtype) &&
		(MEDIASUBTYPE_RGB24 != mtIn->subtype) && (MEDIASUBTYPE_RGB32 != mtIn->subtype) && (MEDIASUBTYPE_ARGB32 != mtIn->subtype))
		return S_FALSE;
		
	return S_OK;
}
