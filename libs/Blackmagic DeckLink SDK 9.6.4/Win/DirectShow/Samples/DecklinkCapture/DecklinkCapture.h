//-----------------------------------------------------------------------------
// DecklinkCapture.h
//
// Desc: DirectShow capture sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CDecklinkCaptureApp:
// See DecklinkCapture.cpp for the implementation of this class
//

class CDecklinkCaptureApp : public CWinApp
{
public:
	CDecklinkCaptureApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDecklinkCaptureApp theApp;