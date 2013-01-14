#pragma once

#include "Timecode.h"

//-----------------------------------------------------------------------------
// CBaseClip
//-----------------------------------------------------------------------------
class CBaseClip
{
public:
	CBaseClip(void);
	CBaseClip(const basic_string<TCHAR>& Filename);
	virtual ~CBaseClip(void) {}

	void SetFilePath(const basic_string<TCHAR>& Filename) { m_Filename = Filename; }
	basic_string<TCHAR> FilePath(void) const { return m_Filename; }
	basic_string<TCHAR> Filename(void) const;
	basic_string<TCHAR> Path(void) const;
	basic_string<TCHAR> Extension(void) const;
	
	void SetStart(const TIMECODE& Start) { m_Start = Start; }
	CTimecode Start(void) const { return m_Start; }

	void SetEnd(const TIMECODE& End) { m_End = End; }
	CTimecode End(void) const { return m_End; }

	void SetDuration(const TIMECODE& Duration);
	CTimecode Duration(void) const { return m_Duration; }

	void SetInpoint(const TIMECODE& Inpoint);
	CTimecode Inpoint(void) const { return m_Inpoint; }

	void SetOutpoint(const TIMECODE& Outpoint);
	CTimecode Outpoint(void) const { return m_Outpoint; }

	void SetFrameRate(double FrameRate);
	double FrameRate(void) const { return (double)m_Rate / m_Scale; }

protected:
	basic_string<TCHAR> m_Filename;
	CTimecode m_Start;
	CTimecode m_End;
	CTimecode m_Duration;
	CTimecode m_Inpoint;
	CTimecode m_Outpoint;
	unsigned long m_Rate;
	unsigned long m_Scale;
};
