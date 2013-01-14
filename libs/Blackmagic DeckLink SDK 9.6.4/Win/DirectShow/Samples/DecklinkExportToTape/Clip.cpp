#include "stdafx.h"
#include "Clip.h"

//-----------------------------------------------------------------------------
// Construction
//
CClip::CClip(void)
	: m_Rate(0)
	, m_Scale(1)
{
	m_Filename = TEXT("<No Filename>");
}

CClip::CClip(basic_string<TCHAR> Filename)
	: m_Filename(Filename)
	, m_Rate(0)
	, m_Scale(1)
{
}

//-----------------------------------------------------------------------------
// Destruction
//
CClip::~CClip(void)
{
}

//-----------------------------------------------------------------------------
// GetFilename
//
basic_string<TCHAR> CClip::Filename(void)
{
	return basic_string<TCHAR>(PathFindFileName(m_Filename.c_str()));
}

//-----------------------------------------------------------------------------
// GetPath
//
basic_string<TCHAR> CClip::Path(void)
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
basic_string<TCHAR> CClip::Extension(void)
{
	return basic_string<TCHAR>(PathFindExtension(m_Filename.c_str()));
}

//-----------------------------------------------------------------------------
// SetFrameRate
//
void CClip::SetFrameRate(double FrameRate)
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
