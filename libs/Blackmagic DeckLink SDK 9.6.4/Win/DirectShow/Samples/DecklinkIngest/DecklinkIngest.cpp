// DecklinkIngest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DecklinkIngest.h"
#include "DecklinkIngestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDecklinkIngestApp

BEGIN_MESSAGE_MAP(CDecklinkIngestApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDecklinkIngestApp construction

CDecklinkIngestApp::CDecklinkIngestApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDecklinkIngestApp object

CDecklinkIngestApp theApp;


// CDecklinkIngestApp initialization

BOOL CDecklinkIngestApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	CoInitialize(NULL);

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR));

	CDecklinkIngestDlg dlg;
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

//-----------------------------------------------------------------------------
// ProcessMessageFilter
// Override in order to provide key accelerators for the dialog app.  Overriding
// this method is a bit taboo...but it does work.
BOOL CDecklinkIngestApp::ProcessMessageFilter(int code, LPMSG lpMsg) 
{
	if (m_hAccel)
	{
		if (::TranslateAccelerator(m_pMainWnd->m_hWnd, m_hAccel, lpMsg)) 
		{
			return(TRUE);
		}
	}

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}