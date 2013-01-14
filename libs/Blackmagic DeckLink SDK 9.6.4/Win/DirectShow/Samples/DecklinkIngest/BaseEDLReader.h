//-----------------------------------------------------------------------------
// BaseEDLReader.h
//
// Desc: Abstract base class for reading EDLs.
//
// Copyright (c) Blackmagic Design 2007. All rights reserved.
//-----------------------------------------------------------------------------
#pragma once

#include "BaseClip.h"

class CBaseEDLReader
{
public:
	CBaseEDLReader(const CString Filename, HRESULT* phr);
	virtual ~CBaseEDLReader(void);

	virtual HRESULT Parse(void) = 0;	// Force derived classes to implement this method.

	const basic_string<TCHAR>& Name(void) const { return m_Name; }	// Name of the EDL.
	const list<CBaseClip*>& ClipList(void) const { return m_lClips; }	// A list of clips extracted from the EDL.

private:
	HANDLE m_hFile;	// File handle to the EDL.
	char* m_pFile;	// Pointer to the EDL data.
	unsigned long m_cbFile;	// Size of the EDL data.

	basic_string<TCHAR> m_Name;	// Name of the EDL.
	list<CBaseClip*> m_lClips;	// A list of clips extracted from the EDL.
};
