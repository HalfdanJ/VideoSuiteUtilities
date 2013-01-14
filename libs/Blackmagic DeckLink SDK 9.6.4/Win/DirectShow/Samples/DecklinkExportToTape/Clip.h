#pragma once

#include "Timecode.h"

//-----------------------------------------------------------------------------
// CClip
//-----------------------------------------------------------------------------
class CClip
{
public:
	CClip(void);
	CClip(basic_string<TCHAR> Filename);
	~CClip(void);

	void SetFilePath(basic_string<TCHAR> Filename) { m_Filename = Filename; }
	basic_string<TCHAR> FilePath(void) { return m_Filename; }
	basic_string<TCHAR> Filename(void);
	basic_string<TCHAR> Path(void);
	basic_string<TCHAR> Extension(void);
	
	void SetStart(TIMECODE& Start) { m_Start = Start; }
	void SetStart(CTimecode& Start) { m_Start = Start; }
	CTimecode Start(void) { return m_Start; }

	void SetEnd(TIMECODE& End) { m_End = End; }
	void SetEnd(CTimecode& End) { m_End = End; }
	CTimecode End(void) { return m_End; }

	void SetDuration(TIMECODE& Duration) { m_Duration = Duration; }
	void SetDuration(CTimecode& Duration) { m_Duration = Duration; }
	CTimecode Duration(void) { return m_Duration; }

	void SetInpoint(TIMECODE& Inpoint) { m_Inpoint = Inpoint; }
	void SetInpoint(CTimecode& Inpoint) { m_Inpoint = Inpoint; }
	CTimecode Inpoint(void) { return m_Inpoint; }

	void SetOutpoint(TIMECODE& Outpoint) { m_Outpoint = Outpoint; }
	void SetOutpoint(CTimecode& Outpoint) { m_Outpoint = Outpoint; }
	CTimecode Outpoint(void) { return m_Outpoint; }

	void SetFrameRate(double FrameRate);
	double FrameRate(void) { return (double)m_Rate / m_Scale; }

private:
	basic_string<TCHAR> m_Filename;
	CTimecode m_Start;
	CTimecode m_End;
	CTimecode m_Duration;
	CTimecode m_Inpoint;
	CTimecode m_Outpoint;
	unsigned long m_Rate;
	unsigned long m_Scale;
};
