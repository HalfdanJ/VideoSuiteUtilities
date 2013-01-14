// DecklinkIngest.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// CDecklinkIngestApp:
// See DecklinkIngest.cpp for the implementation of this class
//

class CDecklinkIngestApp : public CWinApp
{
public:
	CDecklinkIngestApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

private:
	HACCEL m_hAccel;
};

extern CDecklinkIngestApp theApp;