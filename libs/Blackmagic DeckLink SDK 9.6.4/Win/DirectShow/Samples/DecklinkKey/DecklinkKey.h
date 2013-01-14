//-----------------------------------------------------------------------------
// DecklinkKey.h
//
// Desc: DirectShow keying sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CDecklinkKeyApp:
// See DecklinkKey.cpp for the implementation of this class
//

class CDecklinkKeyApp : public CWinApp
{
public:
	CDecklinkKeyApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDecklinkKeyApp theApp;