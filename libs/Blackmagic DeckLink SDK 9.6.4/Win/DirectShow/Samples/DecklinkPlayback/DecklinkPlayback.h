//-----------------------------------------------------------------------------
// DecklinkPlayback.h
//
// Desc: DirectShow playback sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CDecklinkPlaybackApp:
// See DecklinkPlayback.cpp for the implementation of this class
//

class CDecklinkPlaybackApp : public CWinApp
{
public:
	CDecklinkPlaybackApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDecklinkPlaybackApp theApp;