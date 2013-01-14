//-----------------------------------------------------------------------------
// TimecodeGrabber.h
//
// Desc: DirectShow sample grabber callback handler class
//
// Copyright (c) Blackmagic Design 2007. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include "Timecode.h"

//-----------------------------------------------------------------------------
// CTimecodeGrabber class
//-----------------------------------------------------------------------------
class CTimecodeGrabber : public ISampleGrabberCB
{
public:
	CTimecodeGrabber(void);
	~CTimecodeGrabber(void);

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef() { return 2; }		// fake the reference counting
	STDMETHODIMP_(ULONG) Release() { return 1; }	// fake the reference counting

	// ISampleGrabberCB interface
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample);
	STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen);

	HRESULT SetTimecodeFilter(IBaseFilter* pFilter);
	HRESULT GetTimecode(CTimecode& Timecode, REFERENCE_TIME* prtStart = NULL, REFERENCE_TIME* prtEnd = NULL, basic_string<TCHAR>* pTimecodeSource = NULL);

private:
	enum { TC_SRC_HANC, TC_SRC_VITC };

	CCritSec m_critSec;	// critical section for buffer swap control
	
	CTimecode m_Timecode;	// Timecode read from a stream, either VITC or RP188.
	REFERENCE_TIME m_rtStart;	// Timecode sample start time.
	REFERENCE_TIME m_rtEnd;	// Timecode sample stop time.
	bool m_bTimecodeStreamValid;	// Stream timecode has been detected.
	int m_Counter;	// The sample grabber callback will continually test the stream for a valid timecode, this is the number of time to test before restarting.
	int m_TimecodeSource;

	CComQIPtr<IAMTimecodeReader, &__uuidof(IAMTimecodeReader)> m_pITimecodeReader;	// This interface is used to read timecode from the RS422 serial port.
	CComQIPtr<IDecklinkIOControl, &__uuidof(IDecklinkIOControl)> m_pIDecklinkIOControl;	// This interface is used to select VITC or RP188 timecode for the stream.
};
