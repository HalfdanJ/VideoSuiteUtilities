//-----------------------------------------------------------------------------
// DecklinkFrameSource.h
//
// Desc: DirectShow frame source sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CDecklinkFrameSourceApp:
// See DecklinkFrameSource.cpp for the implementation of this class
//

class CDecklinkFrameSourceApp : public CWinApp
{
public:
	CDecklinkFrameSourceApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDecklinkFrameSourceApp theApp;