#include "stdafx.h"
#include "Timecode.h"

//-----------------------------------------------------------------------------
const DWORD CTimecode::m_InvalidTimecodeBCD = 0xFFFFFFFF;
//-----------------------------------------------------------------------------
// Construction
//
CTimecode::CTimecode(void)
{
	m_Timecode.wFrameRate = m_FrameRate = 0;
	m_Timecode.wFrameFract = 0x1000;
	m_Timecode.dwFrames = m_InvalidTimecodeBCD;
	m_FrameCount = 0;
}

CTimecode::CTimecode(TIMECODE& Timecode)
{
	m_Timecode = Timecode;
	switch (m_Timecode.wFrameRate)
	{
		case ED_FORMAT_SMPTE_24:
			m_FrameRate = 24;
			m_FrameCount = BCDToFrameCount(m_Timecode.dwFrames);
			break;

		case ED_FORMAT_SMPTE_25:
			m_FrameRate = 25;
			m_FrameCount = BCDToFrameCount(m_Timecode.dwFrames);
			break;

		case ED_FORMAT_SMPTE_30DROP:
		case ED_FORMAT_SMPTE_30:
			m_FrameRate = 30;
			m_FrameCount = BCDToFrameCount(m_Timecode.dwFrames);
			break;

		default:
			m_Timecode.wFrameRate = m_FrameRate = 0;
			m_Timecode.wFrameFract = 0x1000;
			m_Timecode.dwFrames = m_InvalidTimecodeBCD;
			m_FrameCount = 0;
			break;
	}
}

CTimecode::CTimecode(WORD FrameRate, bool bDropFrame, DWORD FrameCount)
{
	m_FrameRate = FrameRate;
	m_FrameCount = FrameCount;
	switch (m_FrameRate)
	{
		case 23:
			++m_FrameRate;

		case 24:
			m_Timecode.wFrameRate = ED_FORMAT_SMPTE_24;
			m_Timecode.dwFrames = FrameCountToBCD(m_FrameCount);
			break;

		case 25:
			m_Timecode.wFrameRate = ED_FORMAT_SMPTE_25;
			m_Timecode.dwFrames = FrameCountToBCD(m_FrameCount);
			break;

		case 29:
		case 59:
			m_Timecode.wFrameRate = ED_FORMAT_SMPTE_30DROP;
			++m_FrameRate;
			m_Timecode.dwFrames = FrameCountToBCD(m_FrameCount);
			break;

		case 30:
		case 60:
			m_Timecode.wFrameRate = bDropFrame ? ED_FORMAT_SMPTE_30DROP : ED_FORMAT_SMPTE_30;
			m_Timecode.dwFrames = FrameCountToBCD(m_FrameCount);
			break;

		default:
			m_Timecode.wFrameRate = m_FrameRate = 0;
			m_Timecode.dwFrames = m_InvalidTimecodeBCD;
			m_FrameCount = 0;
			break;
	}

	m_Timecode.wFrameFract = 0x1000;
}

CTimecode::CTimecode(WORD FrameRate, bool bDropFrame, LPCTSTR lpszTimecode)
{
	switch (FrameRate)
	{
		case 23:
			++FrameRate;

		case 24:
			m_Timecode.wFrameRate = ED_FORMAT_SMPTE_24;
			break;

		case 25:
			m_Timecode.wFrameRate = ED_FORMAT_SMPTE_25;
			break;

		case 29:
		case 59:
			m_Timecode.wFrameRate = ED_FORMAT_SMPTE_30DROP;
			++FrameRate;
			break;

		case 30:
		case 60:
			m_Timecode.wFrameRate = bDropFrame ? ED_FORMAT_SMPTE_30DROP : ED_FORMAT_SMPTE_30;
			break;

		default:
			m_Timecode.wFrameRate = m_FrameRate = 0;
			m_Timecode.dwFrames = m_InvalidTimecodeBCD;
			FrameRate = 0;
			break;
	}

	if (FrameRate)
	{
		m_FrameRate = FrameRate;
		m_Timecode.dwFrames = m_InvalidTimecodeBCD;
		m_FrameCount = 0;

		basic_string<TCHAR> Timecode = lpszTimecode;
		if (!Timecode.empty())
		{
			int Frames = 0, Seconds = 0, Minutes = 0, Hours = 0;
			char Dummy;	// Ignore the drop/non-drop notation.
			if (5 == _stscanf(Timecode.c_str(), TEXT("%d:%d:%d%c%d"), &Hours, &Minutes, &Seconds, &Dummy, &Frames))
			{
				m_FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);
				m_Timecode.dwFrames = FrameCountToBCD(m_FrameCount);
			}
		}
	}

	m_Timecode.wFrameFract = 0x1000;
}

//-----------------------------------------------------------------------------
// TimecodeToString
//
CString CTimecode::TimecodeToString(void)
{
	CString Timecode = TEXT("FF:FF:FF:FF");

	if (m_Timecode.wFrameRate && (m_InvalidTimecodeBCD != m_Timecode.dwFrames))
	{
		int Hours = BCDToDecimal((m_Timecode.dwFrames & 0xFF000000) >> 24);
		int Minutes = BCDToDecimal((m_Timecode.dwFrames & 0x00FF0000) >> 16);
		int Seconds = BCDToDecimal((m_Timecode.dwFrames & 0x0000FF00) >> 8);
		int Frames = BCDToDecimal(m_Timecode.dwFrames & 0x000000FF);

		Timecode.Empty();

		TCHAR buf[8] = {0};
		if (10 > Hours)
		{
			Timecode += "0";
		}
		Timecode += _itot(Hours, buf, 10);
		Timecode += ":";
		if (10 > Minutes)
		{
			Timecode += "0";
		}
		Timecode += _itot(Minutes, buf, 10);
		Timecode += ":";
		if (10 > Seconds)
		{
			Timecode += "0";
		}
		Timecode += _itot(Seconds, buf, 10);
		Timecode += (ED_FORMAT_SMPTE_30DROP == m_Timecode.wFrameRate) ? ";" : ":";
		if (10 > Frames)
		{
			Timecode += "0";
		}
		Timecode += _itot(Frames, buf, 10);
	}

	return Timecode;
}

//-----------------------------------------------------------------------------
// Overloaded operators
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Assignment
//
CTimecode& CTimecode::operator=(const int rhs)
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	unsigned long FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);

	m_FrameCount = FrameCount;
	m_Timecode.dwFrames = FrameCountToBCD(m_FrameCount);
	return *this;
}

//-----------------------------------------------------------------------------
// Assignment
//
CTimecode& CTimecode::operator=(const TIMECODE& rhs)
{
	m_Timecode = rhs;
	switch (m_Timecode.wFrameRate)
	{
		case ED_FORMAT_SMPTE_24:
			m_FrameRate = 24;
			m_FrameCount = BCDToFrameCount(m_Timecode.dwFrames);
			break;

		case ED_FORMAT_SMPTE_25:
			m_FrameRate = 25;
			m_FrameCount = BCDToFrameCount(m_Timecode.dwFrames);
			break;

		case ED_FORMAT_SMPTE_30DROP:
		case ED_FORMAT_SMPTE_30:
			m_FrameRate = 30;
			m_FrameCount = BCDToFrameCount(m_Timecode.dwFrames);
			break;

		default:
			m_Timecode.wFrameRate = m_FrameRate = 0;
			m_Timecode.wFrameFract = 0x1000;
			m_Timecode.dwFrames = m_InvalidTimecodeBCD;
			m_FrameCount = 0;
			break;
	}

	return *this;
}

//-----------------------------------------------------------------------------
// Assignment
//
CTimecode& CTimecode::operator=(const CString& rhs)
{
	unsigned long FrameCount = 0;

	if (!rhs.IsEmpty())
	{
		int Frames = 0, Seconds = 0, Minutes = 0, Hours = 0;
		char DropFrame;	// Ignore the drop/non-drop notation.
		if (5 == _stscanf(rhs, TEXT("%d:%d:%d%c%d"), &Hours, &Minutes, &Seconds, &DropFrame, &Frames))
		{
			FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);
		}
	}

	m_FrameCount = FrameCount;
	m_Timecode.dwFrames = FrameCountToBCD(m_FrameCount);
	return *this;
}

//-----------------------------------------------------------------------------
// Addition
//
CTimecode CTimecode::operator+(const int rhs) const
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	int FrameCount = (int)GetFrameCount(Hours, Minutes, Seconds, Frames);
	if (-1 < rhs)
	{
		// addition
		FrameCount = (int)m_FrameCount + FrameCount;
	}
	else
	{
		// subtraction
		if ((int)m_FrameCount > FrameCount)
		{
			FrameCount = (int)m_FrameCount - FrameCount;
		}
		else
		{
			FrameCount = 0;
		}
	}

	return CTimecode(m_FrameRate, (ED_FORMAT_SMPTE_30DROP == m_Timecode.wFrameRate), FrameCount);
}

CTimecode CTimecode::operator+(const TIMECODE& rhs) const
{
	int FrameCount = 0;
	if ((m_Timecode.wFrameRate == rhs.wFrameRate) && (m_Timecode.wFrameFract == rhs.wFrameFract))
	{
		// TODO: Test for overflow?
		FrameCount = (int)m_FrameCount + BCDToFrameCount(rhs.dwFrames);
	}

	return CTimecode(m_FrameRate, (ED_FORMAT_SMPTE_30DROP == m_Timecode.wFrameRate), FrameCount);
}

//-----------------------------------------------------------------------------
// Subtraction
//
CTimecode CTimecode::operator-(const int rhs) const
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	int FrameCount = (int)GetFrameCount(Hours, Minutes, Seconds, Frames);
	if (-1 < rhs)
	{
		// subtraction
		if ((int)m_FrameCount > FrameCount)
		{
			FrameCount = (int)m_FrameCount - FrameCount;
		}
		else
		{
			FrameCount = 0;
		}
	}
	else
	{
		// addition
		FrameCount = (int)m_FrameCount + FrameCount;
	}

	return CTimecode(m_FrameRate, (ED_FORMAT_SMPTE_30DROP == m_Timecode.wFrameRate), FrameCount);
}

CTimecode CTimecode::operator-(const TIMECODE& rhs) const
{
	int FrameCount = 0;
	if ((m_Timecode.wFrameRate == rhs.wFrameRate) && (m_Timecode.wFrameFract == rhs.wFrameFract))
	{
		FrameCount = BCDToFrameCount(rhs.dwFrames);
		if ((int)m_FrameCount > FrameCount)
		{
			FrameCount = (int)m_FrameCount - FrameCount;
		}
		else
		{
			FrameCount = 0;
		}
	}

	return CTimecode(m_FrameRate, (ED_FORMAT_SMPTE_30DROP == m_Timecode.wFrameRate), FrameCount);
}

//-----------------------------------------------------------------------------
// Comparison ==
//
bool CTimecode::operator==(const int rhs) const
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	DWORD FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);
	return (m_FrameCount == FrameCount) ? true : false;
}

bool CTimecode::operator==(const TIMECODE& rhs) const
{
	return (0 == memcmp(&m_Timecode, &rhs, sizeof(m_Timecode))) ? true : false;
}

//-----------------------------------------------------------------------------
// Comparison !=
//
bool CTimecode::operator!=(const int rhs) const
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	DWORD FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);
	return (m_FrameCount != FrameCount) ? true : false;
}

bool CTimecode::operator!=(const TIMECODE& rhs) const
{
	return (0 != memcmp(&m_Timecode, &rhs, sizeof(m_Timecode))) ? true : false;
}

//-----------------------------------------------------------------------------
// Comparison <
//
bool CTimecode::operator<(const int rhs) const
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	DWORD FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);
	return (m_FrameCount < FrameCount) ? true : false;
}

bool CTimecode::operator<(const TIMECODE& rhs) const
{
	return (m_FrameCount < BCDToFrameCount(rhs.dwFrames)) ? true : false;
}

//-----------------------------------------------------------------------------
// Comparison <=
//
bool CTimecode::operator<=(const int rhs) const
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	DWORD FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);
	return (m_FrameCount <= FrameCount) ? true : false;
}

bool CTimecode::operator<=(const TIMECODE& rhs) const
{
	return (m_FrameCount <= BCDToFrameCount(rhs.dwFrames)) ? true : false;
}

//-----------------------------------------------------------------------------
// Comparison >
//
bool CTimecode::operator>(const int rhs) const
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	DWORD FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);
	return (m_FrameCount > FrameCount) ? true : false;
}

bool CTimecode::operator>(const TIMECODE& rhs) const
{
	return (m_FrameCount > BCDToFrameCount(rhs.dwFrames)) ? true : false;
}

//-----------------------------------------------------------------------------
// Comparison >=
//
bool CTimecode::operator>=(const int rhs) const
{
	int Hours, Minutes, Seconds, Frames;
	GetFields(abs(rhs), &Hours, &Minutes, &Seconds, &Frames);

	DWORD FrameCount = GetFrameCount(Hours, Minutes, Seconds, Frames);
	return (m_FrameCount >= FrameCount) ? true : false;
}

bool CTimecode::operator>=(const TIMECODE& rhs) const
{
	return (m_FrameCount >= BCDToFrameCount(rhs.dwFrames)) ? true : false;
}

//-----------------------------------------------------------------------------
// private methods
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// GetFields
// Convert the timecode to individual timecode fields, e.g. 100 is 1 second,
// 1111 is 11 seconds and 11 frames, 87 could be 3 seconds and 12 frames @ 25fps, etc.
void CTimecode::GetFields(int Timecode, int* pHours, int* pMinutes, int* pSeconds, int* pFrames) const
{
	if (pHours && pMinutes && pSeconds && pFrames)
	{
		if (m_FrameRate)
		{
			*pFrames = Timecode % 100;
			Timecode /= 100;
			*pSeconds = Timecode % 100;
			Timecode /= 100;
			*pMinutes = Timecode % 100;
			Timecode /= 100;
			*pHours = Timecode % 100;

			if (*pFrames >= m_FrameRate)
			{
				*pSeconds += (*pFrames / m_FrameRate);
				*pFrames %= m_FrameRate;
			}

			if (*pSeconds >= 60)
			{
				*pMinutes += (*pSeconds / 60);
				*pSeconds %= 60;
			}

			if (*pMinutes >= 60)
			{
				*pHours += (*pMinutes / 60);
				*pMinutes %= 60;
			}

			if (*pHours >= 24)
			{
				*pHours %= 24;
			}

			if (ED_FORMAT_SMPTE_30DROP == m_Timecode.wFrameRate)
			{
				if ((0 != (*pMinutes % 10)) && (2 > *pFrames))
				{
					*pFrames = 2;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// GetFrameCount
// Convert the BCD timecode to individual timecode fields.
DWORD CTimecode::GetFrameCount(int Hours, int Minutes, int Seconds, int Frames) const
{
	unsigned long FrameCount = 0;

	if (m_Timecode.wFrameRate)
	{
		FrameCount = Hours * 60;
		FrameCount += Minutes;
		FrameCount *= 60;
		FrameCount += Seconds;
		FrameCount *= m_FrameRate;
		FrameCount += Frames;

		// convert the non-drop frame count to drop frame
		if (ED_FORMAT_SMPTE_30DROP == m_Timecode.wFrameRate)
		{
			// convert to minutes to determine the number of frames to drop
			unsigned long TotalMinutes = (FrameCount / (m_FrameRate * 60));
			FrameCount -= (2 * TotalMinutes - 2 * (TotalMinutes / 10));
		}
	}

	return FrameCount;
}

//-----------------------------------------------------------------------------
// BCDToFrameCount
// Primarily used by the operators to convert BCD timecode to a frame count for
// easy mathematical and comparison operations.
DWORD CTimecode::BCDToFrameCount(DWORD BCDTimecode) const
{
	int Value, Hours, Minutes, Seconds, Frames;
	// Validate the fields
	Value = BCDToDecimal(BCDTimecode & 0x000000FF);
	Frames = Value % m_FrameRate;
	Value = BCDToDecimal((BCDTimecode & 0x0000FF00) >> 8) + (Value / m_FrameRate);
	Seconds = Value % 60;
	Value = BCDToDecimal((BCDTimecode & 0x00FF0000) >> 16) + (Value / 60);
	Minutes = Value % 60;
	Value = BCDToDecimal((BCDTimecode & 0xFF000000) >> 24) + (Value / 60);
	Hours = Value % 24;

	return GetFrameCount(Hours, Minutes, Seconds, Frames);
}

//-----------------------------------------------------------------------------
// FrameCountToBCD
// Convert the frame count argument to a BCD timecode value using the
// frame rate of this object.
DWORD CTimecode::FrameCountToBCD(DWORD FrameCount) const
{
	DWORD BCDTimecode = m_InvalidTimecodeBCD;

	if (m_Timecode.wFrameRate)
	{
		unsigned long framecount = FrameCount;
		if (ED_FORMAT_SMPTE_30DROP == m_Timecode.wFrameRate)
		{
			// convert the non-drop frame count to drop frame
			unsigned long d, m;

			d = (FrameCount / 17982);
			m = (FrameCount % 17982);		
			
			if (m < 2)
			{
				m = 2;
			}

			framecount += ((18 * d) + (2 * ((m - 2) / 1798)));
		}

		int Hours, Minutes, Seconds, Frames;
		Frames = (int)(framecount % m_FrameRate);
		framecount /= m_FrameRate;
		Seconds = (int)(framecount % 60);
		framecount /= 60;
		Minutes = (int)(framecount % 60);
		framecount /= 60;
		Hours = (int)framecount;

		BCDTimecode = (((DecimalToBCD(Hours) << 24) & 0xFF000000) | ((DecimalToBCD(Minutes) << 16) & 0x00FF0000) | ((DecimalToBCD(Seconds) << 8) & 0x0000FF00) | (DecimalToBCD(Frames) & 0x000000FF));
	}

	return BCDTimecode;
}
