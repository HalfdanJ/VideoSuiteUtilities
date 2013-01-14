//------------------------------------------------------------------------------
// CustomAllocator.h
//
// Desc: DirectShow sample code - Custom Allocator
//		 Based largely upon the DirectShow SDK memory allocator base class
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//------------------------------------------------------------------------------

#pragma once

#include "DecklinkFilters_h.h"
#include "..\3rd Party\GDCL\GMFBridge\dispatch.h"	// BaseIDispatch implementation.

// -------------------------------------------------------------------------
class CCustomAllocator : public CBaseAllocator
						, public IDecklinkPushSource
						, public IDecklinkPushSource2
						, public BaseIDispatch<IDecklinkPushSource3, &__uuidof(IDecklinkPushSource3)>	// Automation interface.
{
public:
	CCustomAllocator(TCHAR *, LPUNKNOWN, HRESULT *, BOOL bEvent = TRUE, BOOL fEnableReleaseCallback = FALSE);
	~CCustomAllocator(void);

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN, HRESULT *);

	DECLARE_IUNKNOWN;

    // Base class overrides
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IMemAllocator
	STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);

	STDMETHODIMP Decommit(void);

    // get container for a sample. Blocking, synchronous call to get the
    // next free buffer (as represented by an IMediaSample interface).
    // on return, the time etc properties will be invalid, but the buffer
    // pointer and size will be correct. The two time parameters are
    // optional and either may be NULL, they may alternatively be set to
    // the start and end times the sample will have attached to it
    // bPrevFramesSkipped is not used (used only by the video renderer's
    // allocator where it affects quality management in direct draw).
    STDMETHODIMP GetBuffer(IMediaSample** ppBuffer, REFERENCE_TIME* pStartTime, REFERENCE_TIME* pEndTime, DWORD dwFlags);
    // final release of a CMediaSample will call this
    STDMETHODIMP ReleaseBuffer(IMediaSample* pBuffer);

	// IDecklinkPushSource interface
	STDMETHODIMP GetFrameBuffer(unsigned char** ppBuffer, unsigned long* pSize);
	STDMETHODIMP Deliver(unsigned char* pBuffer);

	// IDecklinkPushSource2 interface
	STDMETHODIMP GetFrameBuffer(IMediaSample** ppSample);
	STDMETHODIMP Deliver(IMediaSample* pSample);

	// IDecklinkPushSource3 interface
	STDMETHODIMP GetFrameBuffer(IUnknown** ppSample);
	STDMETHODIMP Deliver(IUnknown* pSample);

private:
	CGenericList<CMediaSample> m_lFree;
	CGenericList<CMediaSample> m_lDeliver;
	CGenericList<unsigned char> m_lBuffers;

    LPBYTE m_pBuffer;   // combined memory for all buffers

    HANDLE m_hSemFree;              // For signalling
    long m_lWaitingFree;            // Waiting for a free element

    // override to free the memory when decommit completes
    // - we actually do nothing, and save the memory until deletion.
    void Free(void);

    // called from the destructor (and from Alloc if changing size/count) to
    // actually free up the memory
    void ReallyFree(void);

    // allocate the memory when commit called
    HRESULT Alloc(void);

	STDMETHODIMP GetFreeCount(LONG* plBuffersFree);

    // Notify that a sample is available
	void NotifySampleFree(void);

    // Notify that we're waiting for a sample
    void SetWaitingFree(void) { m_lWaitingFree++; }
};
