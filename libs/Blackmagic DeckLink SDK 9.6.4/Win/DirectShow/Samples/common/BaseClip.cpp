#include "stdafx.h"
#include "BaseClip.h"

//-----------------------------------------------------------------------------
// Construction
//
CBaseClip::CBaseClip(void)
	: m_Rate(0)
	, m_Scale(1)
{
	m_Filename = TEXT("<No Filename>");
}

CBaseClip::CBaseClip(const basic_string<TCHAR>& Filename)
	: m_Filename(Filename)
	, m_Rate(0)
	, m_Scale(1)
{
}

//-----------------------------------------------------------------------------
// GetFilename
//
basic_string<TCHAR> CBaseClip::Filename(void) const
{
	return basic_string<TCHAR>(PathFindFileName(m_Filename.c_str()));
}

//-----------------------------------------------------------------------------
// GetPath
//
basic_string<TCHAR> CBaseClip::Path(void) const
{
	basic_string<TCHAR> Path = m_Filename;
	basic_string<TCHAR>::size_type pos = m_Filename.find(Filename());
	if (basic_string<TCHAR>::npos != pos)
	{
		Path.erase(pos);
	}
	return Path;
}

//-----------------------------------------------------------------------------
// Extension
//
basic_string<TCHAR> CBaseClip::Extension(void) const
{
	return basic_string<TCHAR>(PathFindExtension(m_Filename.c_str()));
}

//-----------------------------------------------------------------------------
// SetInpoint
//
void CBaseClip::SetInpoint(const TIMECODE& Inpoint)
{
	m_Inpoint = Inpoint;

	if (!m_Outpoint.IsValid() || (m_Outpoint < m_Inpoint))
	{
		m_Outpoint = m_Inpoint;
	}
	m_Duration = m_Outpoint - m_Inpoint + 1;
}

//-----------------------------------------------------------------------------
// SetOutpoint
//
void CBaseClip::SetOutpoint(const TIMECODE& Outpoint)
{
	m_Outpoint = Outpoint;
	if (!m_Inpoint.IsValid() || (m_Inpoint > m_Outpoint))
	{
		m_Inpoint = m_Outpoint;
	}
	m_Duration = m_Outpoint - m_Inpoint + 1;
}

//-----------------------------------------------------------------------------
// SetDuration
//
void CBaseClip::SetDuration(const TIMECODE& Duration)
{
	m_Duration = Duration;
	if (!m_Duration.IsValid() || (m_Duration == 0))
	{
		m_Duration = 1;
	}

	if (!m_Inpoint.IsValid())
	{
		m_Inpoint = 0;
	}

	m_Outpoint = m_Inpoint + m_Duration - 1;
}

//-----------------------------------------------------------------------------
// SetFrameRate
//
void CBaseClip::SetFrameRate(double FrameRate)
{
	if ((FrameRate - (int)FrameRate) > 0.01)
	{
		// fractional frame rate
		m_Rate = (unsigned long)(FrameRate * 1000.0);
		m_Scale = 1000;
	}
	else
	{
		m_Rate = (unsigned long)FrameRate;
		m_Scale = 1;
	}
}
