//-----------------------------------------------------------------------------
// CMX3600EDLReader.h
//
// Desc: A class for reading CMX3600 EDLs.
//
// Copyright (c) Blackmagic Design 2007. All rights reserved.
//-----------------------------------------------------------------------------
#pragma once
#include "BaseEDLReader.h"

class CCMX3600EDLReader : public CBaseEDLReader
{
public:
	CCMX3600EDLReader(const CString Filename, HRESULT* phr) : CBaseEDLReader(Filename, phr) { HRESULT hr = Parse(); if (FAILED(hr) && phr) { *phr = hr; } }

	virtual HRESULT Parse(void);
};
