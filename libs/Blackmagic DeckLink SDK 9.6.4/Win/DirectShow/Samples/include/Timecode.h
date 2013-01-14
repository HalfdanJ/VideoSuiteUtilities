#pragma once

class CTimecode
{
public:
	CTimecode(void);
	CTimecode(TIMECODE& Timecode);
	CTimecode(WORD FrameRate, bool bDropFrame, DWORD Frames);
	CTimecode(WORD FrameRate, bool bDropFrame, LPCTSTR lpszTimecode);
	virtual ~CTimecode(void) {}

	TIMECODE GetTimecode(void) { return m_Timecode; }
	WORD GetFrameRate(void) { return m_FrameRate; }
	DWORD GetFrameCount(void) { return m_FrameCount; }

	bool IsValid(void) { return (m_InvalidTimecodeBCD == m_Timecode.dwFrames) ? false : true; }

	CString TimecodeToString(void);

	CTimecode& operator=(const int rhs);
	CTimecode& operator=(const TIMECODE& rhs);
	CTimecode& operator=(const CString& rhs);

	CTimecode operator+(const int rhs) const;
	CTimecode operator+(const TIMECODE& rhs) const;

	CTimecode operator-(const int rhs) const;
	CTimecode operator-(const TIMECODE& rhs) const;

	bool operator==(const int rhs) const;
	bool operator==(const TIMECODE& rhs) const;

	bool operator!=(const int rhs) const;
	bool operator!=(const TIMECODE& rhs) const;

	bool operator<(const int rhs) const;
	bool operator<(const TIMECODE& rhs) const;

	bool operator<=(const int rhs) const;
	bool operator<=(const TIMECODE& rhs) const;

	bool operator>(const int rhs) const;
	bool operator>(const TIMECODE& rhs) const;

	bool operator>=(const int rhs) const;
	bool operator>=(const TIMECODE& rhs) const;

	operator const TIMECODE&() const { return m_Timecode; }

private:
	TIMECODE m_Timecode;	// BCD representation of timecode.
	WORD m_FrameRate;	// Internal field for simplifying timecode math.
	DWORD m_FrameCount;	// Internal field for simplifying timecode math.
	static const DWORD m_InvalidTimecodeBCD;

	void GetFields(int Timecode, int* pHours, int* pMinutes, int* pSeconds, int* pFrames) const;

	DWORD GetFrameCount(int Hours, int Minutes, int Seconds, int Frames) const;

	DWORD BCDToFrameCount(DWORD Timecode) const;
	DWORD FrameCountToBCD(DWORD FrameCount) const;

	int BCDToDecimal(int BCD) const { return (10 * ((BCD & 0xF0) >> 4) + (BCD & 0x0F)); }
	int DecimalToBCD(int Decimal) const { return (Decimal % 10) | ((Decimal / 10 % 10) << 4); }
};
