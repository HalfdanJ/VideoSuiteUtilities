#pragma once

#include "Timecode.h"

//-----------------------------------------------------------------------------
// CClip
//-----------------------------------------------------------------------------
class CClip
{
public:
	CClip(void);
	CClip(LPCTSTR lpszTapename, LPCTSTR lpszClipname, LPCTSTR lpszInpoint, LPCTSTR lpszOutpoint, WORD FrameRate);
	CClip(LPCTSTR lpszTapename, LPCTSTR lpszClipname, TIMECODE& Inpoint, TIMECODE& Outpoint);
	CClip(LPCTSTR lpszTapename, LPCTSTR lpszClipname, CTimecode& Inpoint, CTimecode& Outpoint);
	~CClip(void);

	void SetTapename(basic_string<TCHAR> Tapename) { m_Tapename = Tapename; }
	basic_string<TCHAR> Tapename(void) const { return m_Tapename; }

	void SetClipname(basic_string<TCHAR> Clipname) { m_Clipname = Clipname; }
	basic_string<TCHAR> Clipname(void) const { return m_Clipname; }
	
	void SetInpoint(TIMECODE& Inpoint) { m_Inpoint = Inpoint; }
	void SetInpoint(CTimecode& Inpoint) { m_Inpoint = Inpoint; }
	CTimecode Inpoint(void) const { return m_Inpoint; }

	void SetOutpoint(TIMECODE& Outpoint) { m_Outpoint = Outpoint; }
	void SetOutpoint(CTimecode& Outpoint) { m_Outpoint = Outpoint; }
	CTimecode Outpoint(void) const { return m_Outpoint; }

	void SetDuration(TIMECODE& Duration) { m_Duration = Duration; }
	void SetDuration(CTimecode& Duration) { m_Duration = Duration; }
	CTimecode Duration(void) const { return m_Duration; }

private:
	basic_string<TCHAR> m_Tapename;
	basic_string<TCHAR> m_Clipname;
	CTimecode m_Inpoint;
	CTimecode m_Outpoint;
	CTimecode m_Duration;
};
