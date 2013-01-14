// SGCallbackHandler.h
//

#pragma once

class CSGCallbackHandler : public ISampleGrabberCB
{
public:
	CSGCallbackHandler(void);
	~CSGCallbackHandler(void);

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef() { return 2; }		// fake the reference counting
	STDMETHODIMP_(ULONG) Release() { return 1; }	// fake the reference counting

	HRESULT SetMediaType(const AM_MEDIA_TYPE* pmt);
	HRESULT LoadBitmap(LPCTSTR lpFilename);

	// ISampleGrabberCB interface
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample);
	STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen);

private:
	CCritSec m_critSec;	// critical section for buffer swap control
	CMediaType m_mediaTypeVideo;
	BITMAPINFOHEADER* m_pbmihVideo;
	BITMAPINFOHEADER m_bmihLogo;
	unsigned char* m_pLogoBuffer[2];	// two frame buffer pointers
	BOOL m_bActiveBuffer;	// active buffer flag
	BOOL m_bSwapBuffer;	// swap buffer flag

	HRESULT Read(HANDLE hFile, LPVOID lpBuffer, DWORD cbBuffer);
};
