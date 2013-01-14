//-----------------------------------------------------------------------------
// DecklinkFrameSource.cpp
//
// Desc: DirectShow frame source sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DecklinkFrameSource.h"
#include "DecklinkFrameSourceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDecklinkFrameSourceApp

BEGIN_MESSAGE_MAP(CDecklinkFrameSourceApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDecklinkFrameSourceApp construction

CDecklinkFrameSourceApp::CDecklinkFrameSourceApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDecklinkFrameSourceApp object

CDecklinkFrameSourceApp theApp;


// CDecklinkFrameSourceApp initialization

BOOL CDecklinkFrameSourceApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	CoInitialize(NULL);

	CDecklinkFrameSourceDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	CoUninitialize();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
