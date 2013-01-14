//-----------------------------------------------------------------------------
// SGCallbackHandler.h
//
// Desc: DirectShow histogram sample callback handler
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include "HistogramCtrl.h"

class CSGCallbackHandler : public ISampleGrabberCB
{
public:
	CSGCallbackHandler(CHistogramCtrl* pCtrl);
	~CSGCallbackHandler(void);

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef() { return 2; }		// fake the reference counting
	STDMETHODIMP_(ULONG) Release() { return 1; }	// fake the reference counting

	// ISampleGrabberCB interface
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample);
	STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen);

private:
	CHistogramCtrl* m_pCtrl;
};
