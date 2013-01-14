//------------------------------------------------------------------------------
// DecklinkStillSource.h
//
// Desc: DirectShow sample code - External push mode source filter
//
// Copyright (c) Blackmagic Design 2003-2005.  All rights reserved.
//------------------------------------------------------------------------------

#include <string>

using namespace std;

//------------------------------------------------------------------------------
// CDecklinkStillSourcePin class
//------------------------------------------------------------------------------
class CDecklinkStillSourcePin : public CSourceStream
								, public CBaseStreamControl
{
public:
    CDecklinkStillSourcePin(HRESULT* phr, CSource* pFilter);
    ~CDecklinkStillSourcePin();

	DECLARE_IUNKNOWN;

	// Base class overrides
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // Override the version that offers exactly one media type
    virtual HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT GetMediaType(CMediaType* pMediaType);
    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest);
    HRESULT FillBuffer(IMediaSample* pSample);
	HRESULT OnThreadStartPlay(void);

	// for the IAMStreamControl interface
	STDMETHODIMP BeginFlush(void);
	STDMETHODIMP EndFlush(void);

    // Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter* pSelf, Quality q)
    {
        return E_FAIL;
    }

private:
    int m_iFrameNumber;					// number of frames delivered
    REFERENCE_TIME m_rtFrameLength;		// frame duration in 100ns units
    REFERENCE_TIME m_rtNextValidStart;	// used to guarantee that sample timestamps do not overlap

	BOOL m_fLastSampleDiscarded;	// for the IAMStreamControl implementation

    CCritSec m_cSharedState;            // Protects our internal state
};

//------------------------------------------------------------------------------
// CDecklinkStillSource class
//------------------------------------------------------------------------------
class CDecklinkStillSource : public CSource
							, public IAMFilterMiscFlags
							, public IFileSourceFilter
{
public:
    static CUnknown* WINAPI CreateInstance(IUnknown* pUnk, HRESULT* phr);  

	DECLARE_IUNKNOWN;

	// Base class overrides
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// for the IAMStreamControl interface
	STDMETHODIMP SetSyncSource(IReferenceClock* pClock);
	STDMETHODIMP JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName);
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	STDMETHODIMP Pause(void);
	STDMETHODIMP Stop(void);
	
	// IAMFilterMiscFlags interface
	ULONG STDMETHODCALLTYPE GetMiscFlags(void) {return AM_FILTER_MISC_FLAGS_IS_SOURCE;}

	// IFileSourceFilter interface
	STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
	STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);

	HRESULT GetMediaType(CMediaType* pmt);
	HRESULT GetBuffer(unsigned char** ppBuffer, unsigned long* pcbBuffer);

private:
    // Constructor is private because you have to use CreateInstance
    CDecklinkStillSource(IUnknown* pUnk, HRESULT* phr);
    ~CDecklinkStillSource();

    CDecklinkStillSourcePin* m_pPin;

	basic_string<WCHAR> m_filename;
	HANDLE m_hFile;
	CMediaType m_mediaType;	// media type of the desired connection

	CCritSec m_critSec;	// critical section for buffer swap control
	unsigned char* m_pFrameBuffer[2];	// two frame buffer pointers
	BOOL m_bActiveBuffer;	// active buffer flag
	BOOL m_bSwapBuffer;	// swap buffer flag

	HRESULT Read(LPVOID lpBuffer, DWORD cbBuffer);
	HRESULT CreateCheckerFrame(const BITMAPINFOHEADER* pbmih, unsigned char* pBuffer, unsigned long cbBuffer);
};
