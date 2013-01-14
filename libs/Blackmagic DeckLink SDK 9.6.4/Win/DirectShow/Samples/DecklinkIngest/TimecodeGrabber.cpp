//-----------------------------------------------------------------------------
// TimecodeGrabber.cpp
//
// Desc: DirectShow sample grabber callback handler class
//
// Copyright (c) Blackmagic Design 2007. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "TimecodeGrabber.h"
#include "Utils.h"

//-----------------------------------------------------------------------------
// CSGCallbackHandlerTimecode implementation
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constuctor
//
CTimecodeGrabber::CTimecodeGrabber(void)
	: m_rtStart(MAXLONGLONG)
	, m_rtEnd(MAXLONGLONG)
	, m_bTimecodeStreamValid(false)
	, m_Counter(10)
	, m_TimecodeSource(TC_SRC_HANC)
{
}

//-----------------------------------------------------------------------------
// Destructor
//
CTimecodeGrabber::~CTimecodeGrabber(void)
{
}

//-----------------------------------------------------------------------------
// QueryInterface
//
STDMETHODIMP CTimecodeGrabber::QueryInterface(REFIID riid, void **ppv)
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
// Callback for sample grabber interface which provides a timecode media sample.
// Test for RP188 timecode and if this fails, test for VITC timecode.
STDMETHODIMP CTimecodeGrabber::SampleCB(double SampleTime, IMediaSample* pSample)
{
	CheckPointer(pSample, E_POINTER);
	HRESULT hr = S_OK;

	long cbData = pSample->GetActualDataLength();
	if (sizeof(m_Timecode) <= cbData)
	{
		BYTE* pData = NULL;
		hr = pSample->GetPointer(&pData);
		if (SUCCEEDED(hr) && pData)
		{
			CAutoLock lock(&m_critSec);
			TIMECODE_SAMPLE* pTimecodeSample = (TIMECODE_SAMPLE*)pData;

			if (0xffffffff == pTimecodeSample->timecode.dwFrames)
			{
				m_bTimecodeStreamValid = false;
				--m_Counter;
			}
			else
			{
				m_bTimecodeStreamValid = true;
			}			

			if (m_bTimecodeStreamValid)
			{
				m_Timecode = pTimecodeSample->timecode;
			}
			else
			{
				if ((5 == m_Counter) && m_pIDecklinkIOControl)
				{
					// Switch the timecode source to VITC.
					m_pIDecklinkIOControl->SetCaptureTimecodeSource(DECKLINK_TIMECODESOURCE_VITC);
					m_TimecodeSource = TC_SRC_VITC;
				}
				else if ((0 >= m_Counter) && m_pIDecklinkIOControl)
				{
					// Switch the timecode source to RP188.
					m_pIDecklinkIOControl->SetCaptureTimecodeSource(DECKLINK_TIMECODESOURCE_HANC);
					m_Counter = 10;
					m_TimecodeSource = TC_SRC_HANC;
				}
			}

			hr = pSample->GetTime(&m_rtStart, &m_rtEnd);
			
			TRACE(TEXT("CTimecodeGrabber::SampleCB() - %s [%10I64d  %10I64d]\r\n"), m_Timecode.TimecodeToString(), m_rtStart, m_rtEnd);
		}
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// BufferCB
// Callback for sample grabber interface which provides a buffer.
STDMETHODIMP CTimecodeGrabber::BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen)
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// SetTimecodeFilter
// Provide the filter that can be used for reading timecode.
HRESULT CTimecodeGrabber::SetTimecodeFilter(IBaseFilter* pFilter)
{
	HRESULT hr = S_OK;
	m_pITimecodeReader = pFilter;
	m_pIDecklinkIOControl = pFilter;
	if (m_pIDecklinkIOControl)
	{
		m_pIDecklinkIOControl->SetCaptureTimecodeSource(DECKLINK_TIMECODESOURCE_HANC);
		m_Counter = 10;
		m_TimecodeSource = TC_SRC_HANC;
	}
	else
	{
		// If there is no I/O control interface then clear this flag so timecode
		// will be read only from the serial port.
		m_rtStart = MAXLONGLONG;
		m_rtEnd = MAXLONGLONG;
		m_bTimecodeStreamValid = false;
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// GetTimecode
// Read the timecode from the device using HANC (RP188) or VITC (SMPTE 12M) in
// preference to the serial port (RS422).  As it is possible to read timecode
// from a pin on a capture filter and knowing that this timecode, since it is
// derived from the video, has the same sample timestamps as the video frame,
// these timestamps can be provided to the caller for synchronisation purposes.
HRESULT CTimecodeGrabber::GetTimecode(CTimecode& Timecode, REFERENCE_TIME* prtStart, REFERENCE_TIME* prtEnd, basic_string<TCHAR>* pTimecodeSource)
{
	CAutoLock lock(&m_critSec);
	HRESULT hr = S_OK;
	LPCTSTR pszTimecodeSource = NULL;

	if (m_bTimecodeStreamValid)
	{
		// Timecode is available from the timecode stream so provide this in preference to
		// timecod read from the RS422 serial port.  If the caller is interested in the
		// timestamp of the media sample then also provide the start and stop times.
		Timecode = m_Timecode;
		pszTimecodeSource = (TC_SRC_HANC == m_TimecodeSource) ? TEXT("HANC (SMPTE RP188)") : TEXT("VITC (SMPTE 12M)");
	}
	else if (m_pITimecodeReader)
	{
		// Read timecode from the RS422 serial port.
		TIMECODE_SAMPLE TimecodeSample = {0};
		TimecodeSample.dwFlags = ED_DEVCAP_TIMECODE_READ;

		hr = m_pITimecodeReader->GetTimecode(&TimecodeSample);
		if (S_OK == hr)
		{
			Timecode = TimecodeSample.timecode;
			pszTimecodeSource = TEXT("serial port (RS422)");
		}
	}
	else
	{
		// It was not possible to read timecode from any source.
		hr = S_FALSE;
	}

	// Provide the timecode sample timestamps.
	if (prtStart)
	{
		*prtStart = m_rtStart;
	}

	if (prtEnd)
	{
		*prtEnd = m_rtEnd;
	}

	// Provide a textual description of the source of the timecode.
	if (pTimecodeSource)
	{
		if (pszTimecodeSource)
		{
			pTimecodeSource->assign(TEXT("Reading timecode from "));
			pTimecodeSource->append(pszTimecodeSource);
		}
		else
		{
			pTimecodeSource->assign(TEXT("WARNING: Unable to read timecode from device"));
		}
	}

	return hr;
}
