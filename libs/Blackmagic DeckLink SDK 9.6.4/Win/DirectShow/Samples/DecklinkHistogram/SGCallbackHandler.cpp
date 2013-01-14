//-----------------------------------------------------------------------------
// SGCallbackHandler.cpp
//
// Desc: DirectShow histogram sample callback handler
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "SGCallbackHandler.h"

//-----------------------------------------------------------------------------
// Constuctor
//
CSGCallbackHandler::CSGCallbackHandler(CHistogramCtrl* pCtrl)
	: m_pCtrl(pCtrl)
{
}

//-----------------------------------------------------------------------------
// Destructor
//
CSGCallbackHandler::~CSGCallbackHandler(void)
{
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
// ISampleGrabberCB interface
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// SampleCB
// Callback for sample grabber interface which provides a media sample
STDMETHODIMP CSGCallbackHandler::SampleCB(double SampleTime, IMediaSample* pSample)
{
	HRESULT hr = S_OK;

	if (pSample && m_pCtrl)
	{
		long cbData = pSample->GetActualDataLength();
		BYTE* pData = NULL;
		hr = pSample->GetPointer(&pData);
		if (SUCCEEDED(hr))
		{
			hr = m_pCtrl->SetBuffer(reinterpret_cast<const unsigned char*>(pData), cbData);
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
STDMETHODIMP CSGCallbackHandler::BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen)
{
	HRESULT hr = S_OK;

	if (pBuffer && m_pCtrl)
	{
		hr = m_pCtrl->SetBuffer(reinterpret_cast<const unsigned char*>(pBuffer), BufferLen);
	}
	else
	{
		hr = E_POINTER;
	}
	
	return hr;
}
