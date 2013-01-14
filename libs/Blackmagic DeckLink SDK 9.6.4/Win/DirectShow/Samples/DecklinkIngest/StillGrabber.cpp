#include "stdafx.h"
#include "StillGrabber.h"

//-----------------------------------------------------------------------------
// CStillGrabber implementation.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constuction
//
CStillGrabber::CStillGrabber(void)
	: m_FilenameIndex(1)
	, m_bCaptureStill(false)
	, m_hThread(NULL)
	, m_hStopThreadEvent(NULL)
{
	// Set the default file name and path for the still capture object.
	TCHAR szPath[MAX_PATH] = {0};
	if (S_OK == SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, szPath))
	{
		if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(szPath))
		{
			m_FilePath = szPath;
		}
	}
	else
	{
		// The 'My Pictures' directory was not located so use the application folder instead.
		if (GetModuleFileName(NULL, szPath, MAX_PATH))
		{
			m_FilePath = szPath;
			basic_string<TCHAR>::size_type Index = m_FilePath.rfind(TEXT("DecklinkIngest.exe"));
			m_FilePath.erase(Index - 1);
		}
	}
	m_FilenameBase = TEXT("Untitled %d.bmp");

	ZeroMemory(&m_bmih, sizeof(m_bmih));

	m_hStopThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hStopThreadEvent)
	{
		m_hThread = CreateThread(NULL, 0, ThreadWrapper, reinterpret_cast<LPVOID>(this), 0, NULL);
		ASSERT(m_hThread);
	}
}

//-----------------------------------------------------------------------------
// Destuction
// Release resources.
CStillGrabber::~CStillGrabber(void)
{
	while (!m_SampleList.empty())
	{
		IMediaSample* pSample = m_SampleList.front();
		m_SampleList.pop_front();
		pSample->Release();
	}

	SetEvent(m_hStopThreadEvent);
	EXECUTE_ASSERT(WAIT_OBJECT_0 == WaitForSingleObject(m_hThread, 10000));
	CloseHandle(m_hThread);
	CloseHandle(m_hStopThreadEvent);
}

//-----------------------------------------------------------------------------
// QueryInterface
//
STDMETHODIMP CStillGrabber::QueryInterface(REFIID riid, void **ppv)
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
// SetFilePath
// Validate and set the image still file path.
HRESULT CStillGrabber::SetFilePath(basic_string<TCHAR>& FilePath)
{
	HRESULT hr = S_OK;
	
	if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(FilePath.c_str()))
	{
		m_FilePath = FilePath;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// SetFilename
// Validate the new filename and create a filename template.
HRESULT CStillGrabber::SetFilename(basic_string<TCHAR>& Filename)
{
	HRESULT hr = S_OK;

	if (Filename.length())
	{
		CAutoLock lock(&m_CritSec);
		m_FilenameBase = Filename;
		m_FilenameIndex = 1;

		// Ignore the extension if it has been provided.
		basic_string<TCHAR>::size_type Index = m_FilenameBase.rfind(_T('.'));
		if (basic_string<TCHAR>::npos != Index)
		{
			m_FilenameBase.erase(Index);
		}

		m_FilenameBase.append(TEXT(" %d.bmp"));
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// ISampleGrabberCB interface
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// SampleCB
// Callback for sample grabber interface which provides a media sample.  AddRef
// the sample and add it to a queue for writing to disk in order to minimise
// the impact file writing will have on the stream.
STDMETHODIMP CStillGrabber::SampleCB(double SampleTime, IMediaSample* pSample)
{
	CheckPointer(pSample, E_POINTER);
	HRESULT hr = S_OK;

	CAutoLock lock(&m_CritSec);
	if (m_bCaptureStill)
	{
		// Push the sample onto the internal list to save to a still image file.
		pSample->AddRef();
		m_SampleList.push_back(pSample);
		m_bCaptureStill = false;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// BufferCB
// Callback for sample grabber interface which provides a buffer.
STDMETHODIMP CStillGrabber::BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
// Private functions.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ThreadWrapper
//
DWORD WINAPI CStillGrabber::ThreadWrapper(LPVOID lpParam)
{
	DWORD ret = 0;
	CStillGrabber* pStillGrabber = reinterpret_cast<CStillGrabber*>(lpParam);
	if (pStillGrabber)
	{
		ret = pStillGrabber->Thread();
	}
	return ret;
}

//-----------------------------------------------------------------------------
// Thread
//
DWORD CStillGrabber::Thread(void)
{
	DWORD ret = 1;
	bool bRunning = true;
	IMediaSample* pSample = NULL;
	
	while (bRunning)
	{
		switch (WaitForSingleObject(m_hStopThreadEvent, 10))
		{
			case WAIT_TIMEOUT:
				pSample = NULL;
				{
					// Retrieve a sample from the list.
					CAutoLock lock(&m_CritSec);
					if (!m_SampleList.empty())
					{
						pSample = m_SampleList.front();
						m_SampleList.pop_front();
					}
				}

				if (pSample)
				{
					// Save the sample as a bitmap image still to disk.
					BYTE* pBuffer = NULL;
					if (SUCCEEDED(pSample->GetPointer(&pBuffer)))
					{
						long cbBuffer = pSample->GetActualDataLength();
						if (cbBuffer > m_bmih.biSizeImage)
						{
							cbBuffer = m_bmih.biSizeImage;
						}
						
						// Build an indexed filename.
						TCHAR szFilename[MAX_PATH] = {0};
						StringCchPrintf(szFilename, MAX_PATH, TEXT("%s"), m_FilePath.c_str());
						PathAppend(szFilename, m_FilenameBase.c_str());
						StringCchPrintf(szFilename, MAX_PATH, szFilename, m_FilenameIndex);

						// Create the file, the file header and write the image data to the file.
						HANDLE hFile = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						if (INVALID_HANDLE_VALUE != hFile)
						{
							BITMAPFILEHEADER bmfh = {0};
							bmfh.bfType = 'MB';
							bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + cbBuffer;
							bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
							
							DWORD dwNumberOfBytesWritten = 0;
							if (WriteFile(hFile, &bmfh, sizeof(BITMAPFILEHEADER), &dwNumberOfBytesWritten, NULL))
							{
								if (WriteFile(hFile, &m_bmih, sizeof(BITMAPINFOHEADER), &dwNumberOfBytesWritten, NULL))
								{
									if (WriteFile(hFile, pBuffer, cbBuffer, &dwNumberOfBytesWritten, NULL))
									{
										++m_FilenameIndex;
									}
								}
							}
							CloseHandle(hFile);
						}
					}
					pSample->Release();	// Release the outstanding reference on this sample so that it can be returned to the allocator.
				}
				break;
				
			default:
				bRunning = false;
				break;
		}
	}
	
	return ret;
}
