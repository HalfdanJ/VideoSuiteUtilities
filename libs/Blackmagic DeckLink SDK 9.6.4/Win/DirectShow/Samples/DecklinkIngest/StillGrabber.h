//-----------------------------------------------------------------------------
// StillGrabber.h
//
// Desc: DirectShow sample grabber callback handler class
//
// Copyright (c) Blackmagic Design 2007. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

class CStillGrabber : public ISampleGrabberCB
{
public:
	CStillGrabber(void);
	~CStillGrabber(void);

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef() { return 2; }		// Fake the reference counting.
	STDMETHODIMP_(ULONG) Release() { return 1; }	// Fake the reference counting.

	const basic_string<TCHAR>& GetFilePath(void) { return m_FilePath; }
	HRESULT SetFilePath(basic_string<TCHAR>& FilePath);
	HRESULT SetFilename(basic_string<TCHAR>& Filename);
	HRESULT SetBMIH(const BITMAPINFOHEADER* pbmih) { CheckPointer(pbmih, E_POINTER); CAutoLock lock(&m_CritSec); if (pbmih) { m_bmih = *pbmih; } return S_OK; }

	void Capture(void) { CAutoLock lock(&m_CritSec); m_bCaptureStill = true; }

	// ISampleGrabberCB interface
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample);
	STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen);

private:
	CCritSec m_CritSec;	// critical section for buffer swap control

	basic_string<TCHAR> m_FilePath;
	basic_string<TCHAR> m_FilenameBase;
	int m_FilenameIndex;
	BITMAPINFOHEADER m_bmih;
	
	bool m_bCaptureStill;
	list<IMediaSample*> m_SampleList;

	HANDLE m_hThread;
	HANDLE m_hStopThreadEvent;

	static DWORD WINAPI ThreadWrapper(LPVOID lpParam);
	DWORD Thread(void);
};
