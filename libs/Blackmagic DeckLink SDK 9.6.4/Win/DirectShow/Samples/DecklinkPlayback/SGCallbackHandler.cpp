//-----------------------------------------------------------------------------
// SGCallbackHandler.cpp
//
// Desc: DirectShow sample grabber callback handler class
//
// Copyright (c) Blackmagic Design 2006. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "SGCallbackHandler.h"
#include "Utils.h"

//-----------------------------------------------------------------------------
// Constuctor
//
CSGCallbackHandler::CSGCallbackHandler(void)
	: m_pbmihVideo(NULL)
	, m_bActiveBuffer(FALSE)
	, m_bSwapBuffer(FALSE)
{
	m_mediaTypeVideo.InitMediaType();
	ZeroMemory(&m_bmihLogo, sizeof(m_bmihLogo));
	ZeroMemory(m_pLogoBuffer, sizeof(m_pLogoBuffer));
}

//-----------------------------------------------------------------------------
// Destructor
//
CSGCallbackHandler::~CSGCallbackHandler(void)
{
	int count = sizeof(m_pLogoBuffer) / sizeof(m_pLogoBuffer[0]);
	for (int buffer=0; buffer<count; ++buffer)
	{
		SAFE_DELETE(m_pLogoBuffer[buffer]);
	}
}

//-----------------------------------------------------------------------------
// QueryInterface
//
STDMETHODIMP CSGCallbackHandler::QueryInterface(REFIID riid, void **ppv)
{
	CheckPointer(ppv, E_POINTER);
	HRESULT hr = S_OK;

	if ((riid == IID_ISampleGrabberCB) || (riid == IID_IUnknown))
	{
		*ppv = (void*)static_cast<ISampleGrabberCB*>(this);
	}
	else
	{
		hr = E_NOINTERFACE;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// SetMediaType
//
HRESULT CSGCallbackHandler::SetMediaType(const AM_MEDIA_TYPE* pmt)
{
	HRESULT hr = S_OK;

	if (pmt)
	{
		hr = m_mediaTypeVideo.Set(*pmt);
		if (SUCCEEDED(hr))
		{
			m_pbmihVideo = CUtils::GetBMIHeader(pmt);
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// LoadBitmap
// Attempt to load the bitmap from the specified file
HRESULT CSGCallbackHandler::LoadBitmap(LPCTSTR lpFilename)
{
	HRESULT hr = S_OK;
	
	if (lpFilename)
	{
		// attempt to open the file
		HANDLE hFile = CreateFile(lpFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile)
		{
			// attempt to process the bitmap image
			// TODO: Parser other still files, e.g. TGA, JPG, etc.
			BITMAPFILEHEADER bmfh = {0};
			hr = Read(hFile, &bmfh, sizeof(bmfh));
			if (SUCCEEDED(hr))
			{
				// check that this is a bitmap file
				if ('MB' == bmfh.bfType)
				{
					// NOTE: Always assume a BITMAPINFOHEADER, this could be wrong!
					hr = Read(hFile, &m_bmihLogo, sizeof(m_bmihLogo));
					if (SUCCEEDED(hr))
					{
						if ((BI_RGB == m_bmihLogo.biCompression) && (32 == m_bmihLogo.biBitCount))
						{
							CAutoLock lock(&m_critSec);
							
							if (0 == m_bmihLogo.biSizeImage)
							{
								m_bmihLogo.biSizeImage = (m_bmihLogo.biWidth * m_bmihLogo.biHeight * m_bmihLogo.biBitCount) >> 3;
							}

							// set up the back buffer, if the pin is already streaming this
							// will eventually appear
							SAFE_DELETE(m_pLogoBuffer[!m_bActiveBuffer]);
							
							// copy bitmap data into inactive buffer
							m_pLogoBuffer[!m_bActiveBuffer] = new unsigned char [m_bmihLogo.biSizeImage];
							if (m_pLogoBuffer[!m_bActiveBuffer])
							{
								hr = Read(hFile, m_pLogoBuffer[!m_bActiveBuffer], m_bmihLogo.biSizeImage);
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
							// compression is not RGB or no alpha channel present
							hr = E_FAIL;
						}
					}
				}
				else
				{
					hr = E_FAIL;
				}
			}
			
			CloseHandle(hFile);
		}
		else
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
// Private methods
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Read
// Read from the file handle
HRESULT CSGCallbackHandler::Read(HANDLE hFile, LPVOID lpBuffer, DWORD cbBuffer)
{
	HRESULT hr = S_OK;
	DWORD dwBytesRead = 0;
	if (hFile && lpBuffer)
	{
		if ((0 == ReadFile(hFile, lpBuffer, cbBuffer, &dwBytesRead, NULL)) || (0 == dwBytesRead))
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

//-----------------------------------------------------------------------------
// ISampleGrabberCB interface
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// SampleCB
// Callback for sample grabber interface which provides a media sample
// blend the bitmap with the video frame
STDMETHODIMP CSGCallbackHandler::SampleCB(double SampleTime, IMediaSample* pSample)
{
	HRESULT hr = S_OK;

	if (pSample && m_pbmihVideo && m_pbmihVideo->biHeight && m_bmihLogo.biHeight)
	{
		CAutoLock lock(&m_critSec);	// lock the buffer control flags
		if (m_bSwapBuffer)
		{
			// change the active buffer
			m_bActiveBuffer = !m_bActiveBuffer;
			m_bSwapBuffer = FALSE;
		}

		long cbData = pSample->GetActualDataLength();
		BYTE* pBuffer = NULL;
		hr = pSample->GetPointer(&pBuffer);
		if (SUCCEEDED(hr))
		{
			unsigned long videoRowBytes = cbData / m_pbmihVideo->biHeight;
			unsigned long logoRowBytes = CUtils::GetImageSize(&m_bmihLogo) / m_bmihLogo.biHeight;

			unsigned char* pBuffer2 = m_pLogoBuffer[m_bActiveBuffer];

			for (LONG line=0; line<m_bmihLogo.biHeight; ++line)
			{
				unsigned char* pData1 = pBuffer;
				pBuffer += videoRowBytes;

				unsigned char* pData2 = pBuffer2;
				pBuffer2 += logoRowBytes;

				if (logoRowBytes > videoRowBytes)
				{
					logoRowBytes = videoRowBytes;
				}

				unsigned long bytes = logoRowBytes >> 2;
				while (bytes--)
				{
					long blue1 = pData1[0];
					long green1 = pData1[1];
					long red1 = pData1[2];
					long alpha1 = pData1[3];

					long blue2 = pData2[0];
					long green2 = pData2[1];
					long red2 = pData2[2];
					long alpha2 = pData2[3];

					// calculate Channel * alpha from:
					//  y = x.A + (1-x).B
					//  y = x.A + B - x.B
					//  y = x.(A-B) + B
					//
					// scale arithmetic by 65536 to avoid using floating point
					// NOTE: This can be greatly accelerated using MMX/SSE intrinsics
					blue1 = ((alpha2 * 257 * (blue2 - blue1) + 32768) >> 16) + blue1;
					green1 = ((alpha2 * 257 * (green2 - green1) + 32768) >> 16) + green1;
					red1 = ((alpha2 * 257 * (red2 - red1) + 32768) >> 16) + red1;

					pData1[0] = (unsigned char)(blue1 & 0xFF);
					pData1[1] = (unsigned char)(green1 & 0xFF);
					pData1[2] = (unsigned char)(red1 & 0xFF);
					pData1[3] = 0xFF;	// alpha has been applied to pixel, make pixel opaque

					pData1 += 4;
					pData2 += 4;
				}
			}
		}
	}
	else
	{
		hr = E_POINTER;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// BufferCB
// Callback for sample grabber interface which provides a buffer
// blend the bitmap with the video frame
STDMETHODIMP CSGCallbackHandler::BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen)
{
	HRESULT hr = S_OK;

	if (pBuffer && m_bmihLogo.biHeight)
	{
		CAutoLock lock(&m_critSec);	// lock the buffer control flags
		if (m_bSwapBuffer)
		{
			// change the active buffer
			m_bActiveBuffer = !m_bActiveBuffer;
			m_bSwapBuffer = FALSE;
		}

		unsigned long videoRowBytes = BufferLen / 576;
		unsigned long logoRowBytes = CUtils::GetImageSize(&m_bmihLogo) / m_bmihLogo.biHeight;

		unsigned char* pBuffer2 = m_pLogoBuffer[m_bActiveBuffer];

		for (LONG line=0; line<m_bmihLogo.biHeight; ++line)
		{
			unsigned char* pData1 = pBuffer;
			pBuffer += videoRowBytes;

			unsigned char* pData2 = pBuffer2;
			pBuffer2 += logoRowBytes;

			unsigned long bytes = logoRowBytes >> 2;
			while (bytes--)
			{
				long blue1 = pData1[0];
				long green1 = pData1[1];
				long red1 = pData1[2];
				long alpha1 = pData1[3];

				long blue2 = pData2[0];
				long green2 = pData2[1];
				long red2 = pData2[2];
				long alpha2 = pData2[3];

				// calculate Channel * alpha from:
				//  y = x.A + (1-x).B
				//  y = x.A + B - x.B
				//  y = x.(A-B) + B
				//
				// scale arithmetic by 65536 to avoid using floating point
				// NOTE: This can be accelerated using MMX/SSE intrinsics
				blue1 = ((alpha2 * 257 * (blue2 - blue1) + 32768) >> 16) + blue1;
				green1 = ((alpha2 * 257 * (green2 - green1) + 32768) >> 16) + green1;
				red1 = ((alpha2 * 257 * (red2 - red1) + 32768) >> 16) + red1;

				pData1[0] = (unsigned char)(blue1 & 0xFF);
				pData1[1] = (unsigned char)(green1 & 0xFF);
				pData1[2] = (unsigned char)(red1 & 0xFF);
				pData1[3] = 0xFF;	// alpha has been applied to pixel, make pixel opaque

				pData1 += 4;
				pData2 += 4;
			}
		}
	}
	else
	{
		hr = E_POINTER;
	}
	
	return hr;
}
