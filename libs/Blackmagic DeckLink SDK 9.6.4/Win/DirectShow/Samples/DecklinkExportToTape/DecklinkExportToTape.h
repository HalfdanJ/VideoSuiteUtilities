// DecklinkExportToTape.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CDecklinkExportToTapeApp:
// See DecklinkExportToTape.cpp for the implementation of this class
//

class CDecklinkExportToTapeApp : public CWinApp
{
public:
	CDecklinkExportToTapeApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

private:
	HACCEL m_hAccel;
};

extern CDecklinkExportToTapeApp theApp;