//-----------------------------------------------------------------------------
// DecklinkHistogram.h
//
// Desc: DirectShow histogram sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CDecklinkHistogramApp:
// See DecklinkHistogram.cpp for the implementation of this class
//

class CDecklinkHistogramApp : public CWinApp
{
public:
	CDecklinkHistogramApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDecklinkHistogramApp theApp;