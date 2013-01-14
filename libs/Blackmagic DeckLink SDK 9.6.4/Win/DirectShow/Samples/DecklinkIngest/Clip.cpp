#include "stdafx.h"
#include "Clip.h"

//-----------------------------------------------------------------------------
// Construction
//
CClip::CClip(void)
{
	m_Tapename = TEXT("Untitled Tape");
	m_Clipname = TEXT("Untitled Clip");
}


CClip::CClip(LPCTSTR lpszTapename, LPCTSTR lpszClipname, LPCTSTR lpszInpoint, LPCTSTR lpszOutpoint, WORD FrameRate)
	: m_Tapename(lpszTapename)
	, m_Clipname(lpszClipname)
{
	m_Inpoint =  CTimecode(FrameRate, true, lpszInpoint);
	m_Outpoint = CTimecode(FrameRate, true, lpszOutpoint);

	if (m_Outpoint < m_Inpoint)
	{
		m_Outpoint = m_Inpoint;
	}

	m_Duration = m_Outpoint - m_Inpoint + 1;
}

CClip::CClip(LPCTSTR lpszTapename, LPCTSTR lpszClipname, TIMECODE& Inpoint, TIMECODE& Outpoint)
	: m_Tapename(lpszTapename)
	, m_Clipname(lpszClipname)
{
	m_Inpoint = Inpoint;
	m_Outpoint = Outpoint;
	
	if (m_Outpoint < m_Inpoint)
	{
		m_Outpoint = m_Inpoint;
	}

	m_Duration = m_Outpoint - m_Inpoint + 1;
}

CClip::CClip(LPCTSTR lpszTapename, LPCTSTR lpszClipname, CTimecode& Inpoint, CTimecode& Outpoint)
	: m_Tapename(lpszTapename)
	, m_Clipname(lpszClipname)
{
	m_Inpoint = Inpoint;
	m_Outpoint = Outpoint;
	
	if (m_Outpoint < m_Inpoint)
	{
		m_Outpoint = m_Inpoint;
	}

	m_Duration = m_Outpoint - m_Inpoint + 1;
}

//-----------------------------------------------------------------------------
// Destruction
//
CClip::~CClip(void)
{
}
