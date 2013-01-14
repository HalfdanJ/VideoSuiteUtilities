//-----------------------------------------------------------------------------
// DecklinkKeyDlg.cpp
//
// Desc: DirectShow keying sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DecklinkKey.h"
#include "DecklinkKeyDlg.h"
#include "StaticURL.h"

#include <set>
using namespace std;

#include "DecklinkFilters_h.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_GRAPHNOTIFY		WM_APP+1		// for Filter Graph event notification

//-----------------------------------------------------------------------------
// CAboutDlg dialog used for App About
//-----------------------------------------------------------------------------
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	CStaticURL m_LinkBMD;

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};

//-----------------------------------------------------------------------------
// Constructor
//
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

//-----------------------------------------------------------------------------
// DoDataExchange
//
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------
// Message map
//
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// OnInitDialog
// Subclass the static controls that display URLs.
BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_LinkBMD.SubclassDlgItem(IDC_STATIC_URLBMD, this);
	return FALSE;
}

//-----------------------------------------------------------------------------
// CDecklinkKeyDlg dialog
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Construction
//
CDecklinkKeyDlg::CDecklinkKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecklinkKeyDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-----------------------------------------------------------------------------
// DoDataExchange
//
void CDecklinkKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILENAME, m_filenameCtrl);
	DDX_Control(pDX, IDC_SLIDER_ALPHA, m_alphaCtrl);
	DDX_Control(pDX, IDC_STATIC_ALPHA, m_alphaDisplayCtrl);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_preview);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_deviceCtrl);
	DDX_Control(pDX, IDC_COMBO_FRAMERATE, m_frameRateCtrl);
}

//-----------------------------------------------------------------------------
// Message Map
//
BEGIN_MESSAGE_MAP(CDecklinkKeyDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnCbnSelchangeComboDevice)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_RADIO_KEYOFF, OnBnClickedRadioKey)
	ON_BN_CLICKED(IDC_RADIO_KEYINTERNAL, OnBnClickedRadioKey)
	ON_BN_CLICKED(IDC_RADIO_KEYEXTERNAL, OnBnClickedRadioKey)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBnClickedButtonBrowse)
	ON_CBN_SELCHANGE(IDC_COMBO_FRAMERATE, OnCbnSelchangeComboFramerate)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// CDecklinkKeyDlg message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// OnInitDialog
//
BOOL CDecklinkKeyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_ROTRegister = 0;
	
	// filename control
	m_filename = "<Select File>";

	// key level control
	m_keyMode = KEY_OFF;
	m_keyLevel = 50;
	m_alphaCtrl.SetRange(0, 100);

	EnumerateDevices();

	QueryRegistry();

	m_alphaCtrl.SetPos(100 - m_keyLevel);	// the sense of the scroll bar is reversed
	CString alphaDisp;
	alphaDisp.Format("%d%%", m_keyLevel);
	m_alphaDisplayCtrl.SetWindowText(alphaDisp);
	m_filenameCtrl.SetWindowText(m_filename);

	// add filters used for every keying graph
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&m_pGraph));
	if (SUCCEEDED(hr))
	{
#ifdef _DEBUG
		hr = CDSUtils::AddGraphToRot(m_pGraph, &m_ROTRegister);
#endif
		m_pControl = m_pGraph;
		if (m_pControl)
		{
			// Make graph send WM_GRAPHNOTIFY when it wants our attention see "Learning 
			// When an Event Occurs" in the DX9 documentation.
			m_pMediaEvent = m_pGraph;
			if (m_pMediaEvent)
			{
				hr = m_pMediaEvent->SetNotifyWindow(reinterpret_cast<OAHWND>(m_hWnd), WM_GRAPHNOTIFY, 0);
				if (SUCCEEDED(hr))
				{
					// locate the media seeking interface to determine the length of the avi movies
					m_pIMediaSeeking = m_pGraph;
					if (m_pIMediaSeeking)
					{
						m_pIMediaFilter = m_pGraph;
						if (m_pIMediaFilter)
						{
							// checker source for background of keying preview
							hr = CDSUtils::AddFilter(m_pGraph, CLSID_DecklinkStillSource, L"Decklink Checker Source Filter", &m_pCheckerSource);
							if (SUCCEEDED(hr))
							{
								// stream control interface to turn off source so that avi movies can loop
								hr = CDSUtils::FindPinInterface(m_pCheckerSource, L"Video", IID_IAMStreamControl, reinterpret_cast<void**>(&m_pIStreamControl));
								if (SUCCEEDED(hr))
								{
									hr = CDSUtils::AddFilter(m_pGraph, CLSID_InfTee, L"Infinite Pin Tee Filter", &m_pInfiniteTee);
									if (SUCCEEDED(hr))
									{
										// add and configure the VMR
										hr = CDSUtils::AddFilter(m_pGraph, CLSID_VideoRendererDefault, L"Video Renderer", &m_pVideoRenderer);
										if (SUCCEEDED(hr))
										{
											// configure the renderer to be in windowless mode and load the mixer component of the VMR to
											// blend checker with the source which must be done before the VMR is connected
											CComQIPtr<IVMRFilterConfig, &IID_IVMRFilterConfig> pIVMRFilterConfig = m_pVideoRenderer;
											if (pIVMRFilterConfig)
											{
												hr = pIVMRFilterConfig->SetRenderingMode(VMRMode_Windowless);

												hr = pIVMRFilterConfig->SetNumberOfStreams(2);	// loads the mixer component
												if (SUCCEEDED(hr))
												{
													// query for the VMR mixer control interface
													m_pIVMRMixerControl = m_pVideoRenderer;
												}
											}

											// configure the renderer to be in windowless mode
											CComQIPtr<IVMRWindowlessControl, &IID_IVMRWindowlessControl> pIVMRWindowlessCtrl = m_pVideoRenderer;
											if (pIVMRWindowlessCtrl)
											{
												CWnd* pWnd = GetDlgItem(IDC_STATIC_PREVIEW);
												hr = pIVMRWindowlessCtrl->SetVideoClippingWindow(pWnd->GetSafeHwnd());	// set the bounds of the video to the preview window
												RECT rcDst = {0};
												pWnd->GetClientRect(&rcDst);
												SetRect(&rcDst, 0, 0, rcDst.right, rcDst.bottom);
												hr = pIVMRWindowlessCtrl->SetVideoPosition(NULL, &rcDst);	// show the whole of the source frame in the whole of the client area of the control
												hr = pIVMRWindowlessCtrl->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);	// maintain the aspect ratio of the video
												hr = pIVMRWindowlessCtrl->SetBorderColor(GetSysColor(COLOR_BTNFACE));	// set the colour of the letter or pillar boxed area
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	BuildGraph();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------
// DestroyWindow
// Called when the window is being destroyed, clean up and free all resources.
BOOL CDecklinkKeyDlg::DestroyWindow()
{
	m_regUtils.Close();
#ifdef _DEBUG
	CDSUtils::RemoveGraphFromRot(m_ROTRegister);
#endif
	DestroyGraph();

	// release the device names attached to the item's data
	while (m_deviceCtrl.GetCount())
	{
		PWSTR pName = (PWSTR)m_deviceCtrl.GetItemData(0);
		delete [] pName;
		m_deviceCtrl.DeleteString(0);
	}

	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
// OnSysCommand
//
void CDecklinkKeyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

//-----------------------------------------------------------------------------
// OnPaint
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CDecklinkKeyDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//-----------------------------------------------------------------------------
// HandleGraphEvent
// At the moment we just read the event, discard it and release memory used to store it.
void CDecklinkKeyDlg::HandleGraphEvent(void) 
{
	LONG lEventCode, lEventParam1, lEventParam2;
	
	if (!m_pMediaEvent)
	{
		return;
	}
	
	while (SUCCEEDED(m_pMediaEvent->GetEvent(&lEventCode, reinterpret_cast<LONG_PTR *>(&lEventParam1), reinterpret_cast<LONG_PTR *>(&lEventParam2), 0)))
	{
		// handle EC_COMPLETE event to loop avis
		LONGLONG startOfStream = 0;
		if ((EC_COMPLETE == lEventCode) && m_pIMediaSeeking)
		{
			// TODO: investigate further to see if looping can be improved
			m_pControl->Stop();
			m_pIMediaSeeking->SetPositions(&startOfStream, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);

			LONGLONG duration;
			m_pIMediaSeeking->GetDuration(&duration);
			m_pIStreamControl->StopAt(&duration, FALSE, 0);
			m_pControl->Run();
		}

		// just free memory associated with event
		m_pMediaEvent->FreeEventParams(lEventCode, lEventParam1, lEventParam2);
	}
}

//-----------------------------------------------------------------------------
// WindowProc
// Have to add our own message handling loop to handle events from the preview video
// window and to pass Window events onto it - this is so it redraws itself correctly etc.
LRESULT CDecklinkKeyDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_GRAPHNOTIFY:
			HandleGraphEvent();
			break;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

//-----------------------------------------------------------------------------
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDecklinkKeyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboDevice
// Rebuild graph with new device
void CDecklinkKeyDlg::OnCbnSelchangeComboDevice()
{
	BuildGraph();
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboFramerate
// Rebuild graph with new frame rate
void CDecklinkKeyDlg::OnCbnSelchangeComboFramerate()
{
	BuildGraph();
}

//-----------------------------------------------------------------------------
// OnVScroll
// Update the alpha
void CDecklinkKeyDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int min, max;
	m_alphaCtrl.GetRange(min, max);
	m_keyLevel = max - m_alphaCtrl.GetPos();	// invert the sense of the control 100% is at the top
	CString alphaText;
	alphaText.Format("%d%%", m_keyLevel);
	m_alphaDisplayCtrl.SetWindowText(alphaText);

	unsigned int alpha = (unsigned int)((float)m_keyLevel * 255.0f / 100.0f);
	if (m_pIDecklinkKeyer)
	{
		m_pIDecklinkKeyer->set_AlphaLevel(alpha);
	}

	if (m_pIVMRMixerControl)
	{
		m_pIVMRMixerControl->SetAlpha(1, (float)m_keyLevel / 100.0f);
	}

	EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("KeyLevel"), reinterpret_cast<const BYTE*>(&m_keyLevel), sizeof(m_keyLevel)));

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

//-----------------------------------------------------------------------------
// OnBnClickedRadioKey
// Respond to the key control notification
void CDecklinkKeyDlg::OnBnClickedRadioKey()
{
	CButton* pButton = NULL;
	for (int button=IDC_RADIO_KEYOFF; button<=IDC_RADIO_KEYEXTERNAL; ++button)
	{
		pButton = (CButton*)GetDlgItem(button);
		if (BST_CHECKED == pButton->GetCheck())
		{
			m_keyMode = (KEYMODE)(button - IDC_RADIO_KEYOFF);
			if (m_pIDecklinkKeyer)
			{
				if (KEY_INT == m_keyMode)
				{
					m_pIDecklinkKeyer->set_AlphaBlendModeOn(FALSE);
				}
				else if (KEY_EXT == m_keyMode)
				{
					m_pIDecklinkKeyer->set_AlphaBlendModeOn(TRUE);
				}
				else
				{
					m_pIDecklinkKeyer->set_AlphaBlendModeOff();
				}
			}
			// save new state
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("KeyMode"), reinterpret_cast<const BYTE*>(&m_keyMode), sizeof(m_keyMode)));
		}
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonBrowse
// Update the filename control with the selection
void CDecklinkKeyDlg::OnBnClickedButtonBrowse()
{
	char BASED_CODE szFilters[] = "Image Stills|*.bmp;*.avi|All Files (*.*)|*.*||";

	CFileDialog FileDlg(TRUE, "Windows Media Files", "*.bmp;*.avi", 0, szFilters, this);

	if (FileDlg.DoModal() == IDOK)
	{
		m_filenameCtrl.SetWindowText(FileDlg.GetPathName());
	}
	
	// update the graph with the new file
	BuildGraph();
}

//-----------------------------------------------------------------------------
// BuildGraph
// Build or rebuild the graph depending upon the key file selected.  Due to the custom
// bitmap source filter it is possible to just update the bitmap without disconnecting
// the Decklink video render filter.
HRESULT CDecklinkKeyDlg::BuildGraph(void)
{
	HRESULT hr = DestroyGraph();	// tear down the old graph and rebuild with new source
	if (SUCCEEDED(hr))
	{
		TCHAR buf[512] = {0};

	    m_filenameCtrl.GetWindowText(m_filename);		

		// store filename
		USES_CONVERSION;
		WCHAR keyFile[MAX_PATH];
		wcsncpy(keyFile, A2W(m_filename), MAX_PATH);
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetString(TEXT("KeyFile"), reinterpret_cast<const BYTE*>(keyFile), sizeof(keyFile)));

		CComPtr<IBaseFilter> pSource = NULL;
		if (-1 == m_filename.Find(".avi"))
		{
			// check for an existing still source filter
			hr = m_pGraph->FindFilterByName(L"Decklink Still Source Filter", &pSource);
			if (FAILED(hr))
			{
				// filter does not exist so add a new bitmap still source
				hr = CDSUtils::AddFilter(m_pGraph, CLSID_DecklinkStillSource, L"Decklink Still Source Filter", &pSource);
			}

			if (SUCCEEDED(hr))
			{
				// load the new file into the still source
				CComPtr<IFileSourceFilter> pIFSF = NULL;
				hr = CDSUtils::FindFilterInterface(pSource, IID_IFileSourceFilter, reinterpret_cast<void**>(&pIFSF));
				if (SUCCEEDED(hr))
				{
					hr = pIFSF->Load(A2W(m_filename), NULL);
					if (SUCCEEDED(hr))
					{
						// retrieve the frame dimensions from fill still source in order to
						// validate and subsequently set the frame rate
						LPOLESTR pszFilename = NULL;
						CMediaType mediaType;
						hr = pIFSF->GetCurFile(&pszFilename, &mediaType);
						if (SUCCEEDED(hr))
						{
							// populate the frame rate control with formats supported by the card
							// that match the frame dimensions of the fill
							hr = PopulateFrameRateCtrl(mediaType);
							if (SUCCEEDED(hr))
							{
								// as we are only interested in the AvgTimePerFrame field this is a safe cast
								// for both the VIDEOINFOHEADER and VIDEOINFOHEADER2 structures
								VIDEOINFOHEADER* pvih = reinterpret_cast<VIDEOINFOHEADER*>(mediaType.Format());
								if (pvih)
								{
									// set the desired frame rate of the fill
									pvih->AvgTimePerFrame = m_frameRateCtrl.GetItemData(m_frameRateCtrl.GetCurSel());
								}
								
								hr = pIFSF->Load(pszFilename, &mediaType);
							}
							else
							{
								StringCbPrintf(buf, sizeof(buf), TEXT("The device is unable to support the frame dimensions of the fill:\r\n\r\n%s\r\n"), COLE2T(pszFilename));
								MessageBox(buf, TEXT("Unsupported frame dimensions"), MB_OK | MB_ICONERROR);
							}
							CoTaskMemFree(pszFilename);
						}
					}
					else
					{
						StringCbPrintf(buf, sizeof(buf), TEXT("FAILED to load fill:\r\n%s"), m_filename);
						MessageBox(buf, TEXT("File open error"), MB_OK | MB_ICONERROR);
					}
				}
			}
		}
		else
		{
			// use an avi file source
			hr = m_pGraph->AddSourceFilter(A2W(m_filename), L"Source File", &pSource);
		}

		if (SUCCEEDED(hr))
		{
			// add the video renderer
			PWSTR pNameVideo = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());
			CComPtr<IBaseFilter> pDecklinkVideoRenderer = NULL;
			hr = CDSUtils::AddFilter2(m_pGraph, CLSID_TransmitCategory, pNameVideo, &pDecklinkVideoRenderer);
			if (SUCCEEDED(hr))
			{
				// set the graph reference clock to the hardware derived clock on the DeckLink card
				if (m_pIMediaFilter)
				{
					m_pIReferenceClock = pDecklinkVideoRenderer;
					if (m_pIReferenceClock)
					{
						m_pIMediaFilter->SetSyncSource(m_pIReferenceClock);
					}
				}
				
				// locate the Decklink keying interface
				hr = CDSUtils::FindFilterInterface(pDecklinkVideoRenderer, IID_IDecklinkKeyer, reinterpret_cast<void**>(&m_pIDecklinkKeyer));
				if (SUCCEEDED(hr))
				{
					// connect the source to the infinite tee filter
					hr = CDSUtils::ConnectFilters(m_pGraph, pSource, NULL, m_pInfiniteTee, NULL);
					if (SUCCEEDED(hr))
					{
						// connect the infinite tee to the decklink renderer
						hr = CDSUtils::ConnectFilters(m_pGraph, m_pInfiniteTee, NULL, pDecklinkVideoRenderer, NULL);
						if (SUCCEEDED(hr))
						{
							// configure the checker source, determine the source media type and load the appropriate checker
							CComQIPtr<IFileSourceFilter, &IID_IFileSourceFilter> pIFSF = pSource;
							if (pIFSF)
							{
								// retrieve the media type from the fill still source so a checker 
								// can be created with the same frame dimensions
								LPWSTR lpszFilename = NULL;
								CMediaType mediaType;
								hr = pIFSF->GetCurFile(&lpszFilename, &mediaType);
								if (SUCCEEDED(hr))
								{
									CoTaskMemFree(lpszFilename);	// release the filename buffer
									pIFSF = m_pCheckerSource;
									if (pIFSF)
									{
										hr = pIFSF->Load(L"BMD_Checker", &mediaType);
										if (SUCCEEDED(hr))
										{
											// Get the duration of the avi source and set the checker bitmap to stream for the same duration.
											// This way both streams will complete at the same time and the graph will issue an EC_COMPLETE
											// message.  We intercept this message using an event handler and reset the movie to the start
											// so that it appears to continuously loop.
											LONGLONG duration;
											if (SUCCEEDED(m_pIMediaSeeking->GetDuration(&duration)))
											{
												m_pIStreamControl->StopAt(&duration, TRUE, 0);
											}
										}
										else
										{
											MessageBox(TEXT("FAILED to load checker bitmap"), TEXT("File open error"), MB_OK | MB_ICONERROR);
										}
									}
								}
							}

							// connect the checker source to the first pin of the VMR
							hr = CDSUtils::ConnectFilters(m_pGraph, m_pCheckerSource, NULL, m_pVideoRenderer, NULL);
							if (SUCCEEDED(hr))
							{
								// connect the other infinite pin tee pin to the second pin of the VMR
								hr = CDSUtils::ConnectFilters(m_pGraph, m_pInfiniteTee, NULL, m_pVideoRenderer, NULL);
								if (SUCCEEDED(hr))
								{
									hr = m_pIDecklinkKeyer->set_AlphaLevel((unsigned int)((float)m_keyLevel * 255.0f / 100.0f));
									hr = m_pIVMRMixerControl->SetAlpha(1, (float)m_keyLevel / 100);

									if (SUCCEEDED(hr))
									{
										m_pControl->Run();
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		hr = E_POINTER;
	}

	// Update the UI after acquiring the IDecklinkKeyer interface
	UpdateUI();

	// set the keying mode
	OnBnClickedRadioKey();

	return hr;
}

//-----------------------------------------------------------------------------
// DestroyGraph
// Remove all intermediate filters, keep any Decklink and video render filters as
// these are used by all the graphs.
HRESULT CDecklinkKeyDlg::DestroyGraph()
{
	HRESULT hr = S_OK;

	if (m_pControl)
	{
		// stop the graph as we are about to modify it
		m_pControl->Stop();
	}

	if (m_pIDecklinkKeyer)
	{
		m_pIDecklinkKeyer = reinterpret_cast<IDecklinkKeyer*>(NULL);	// release the outstanding reference
	}

	if (m_pIReferenceClock)
	{
		m_pIReferenceClock = reinterpret_cast<IReferenceClock*>(NULL);	// release the outstanding reference
	}

	if (m_pGraph)
	{
		CComPtr<IEnumFilters> pEnum = NULL;
		hr = m_pGraph->EnumFilters(&pEnum);
		if (SUCCEEDED(hr))
		{
			IBaseFilter* pFilter = NULL;
			while (S_OK == pEnum->Next(1, &pFilter, NULL))
			{
				CDSUtils::DisconnectPins(m_pGraph, pFilter);

				FILTER_INFO filterInfo = {0};
				hr = pFilter->QueryFilterInfo(&filterInfo);
				if (SUCCEEDED(hr))
				{
					SAFE_RELEASE(filterInfo.pGraph);
					
					// leave the renderers, infinite T and checker source in the graph
					if ((NULL == wcsstr(filterInfo.achName, L"Video Renderer")) && (NULL == wcsstr(filterInfo.achName, L"Infinite")) && (NULL == wcsstr(filterInfo.achName, L"Checker")))
					{
						hr = m_pGraph->RemoveFilter(pFilter);
						if (SUCCEEDED(hr))
						{
							hr = pEnum->Reset();
						}
					}
				}

				SAFE_RELEASE(pFilter);
			}
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// QueryRegistry
// retrieve previous settings from registry
void CDecklinkKeyDlg::QueryRegistry(void)
{
	if (ERROR_SUCCESS == m_regUtils.Open(TEXT("DecklinkKeySample")))
	{
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetBinary(TEXT("KeyMode"), reinterpret_cast<LPBYTE>(&m_keyMode), sizeof(m_keyMode)));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetBinary(TEXT("KeyLevel"), reinterpret_cast<LPBYTE>(&m_keyLevel), sizeof(m_keyLevel)));

		WCHAR keyFile[MAX_PATH];
		ZeroMemory(keyFile, sizeof(keyFile));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetString(TEXT("KeyFile"), reinterpret_cast<LPBYTE>(keyFile), sizeof(keyFile)));
		m_filename = keyFile;
	}
	else
	{
		// create the key and registry values
		if (ERROR_SUCCESS == m_regUtils.Create(TEXT("DecklinkKeySample")))
		{
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("KeyMode"), reinterpret_cast<const BYTE*>(&m_keyMode), sizeof(m_keyMode)));
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("KeyLevel"), reinterpret_cast<const BYTE*>(&m_keyLevel), sizeof(m_keyLevel)));
			USES_CONVERSION;
			WCHAR keyFile[MAX_PATH];
			wcsncpy(keyFile, A2W(m_filename), MAX_PATH);
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetString(TEXT("KeyFile"), reinterpret_cast<const BYTE*>(keyFile), sizeof(keyFile)));
		}
	}
}

//-----------------------------------------------------------------------------
// UpdateUI
//
void CDecklinkKeyDlg::UpdateUI(void)
{
	// key mode control, first determine the keying supported by the hardware
	BOOL bIntKey = FALSE, bExtKey = FALSE;
	if (m_pIDecklinkKeyer)
	{
		// we have the keying interface
		if (SUCCEEDED(m_pIDecklinkKeyer->get_DeviceSupportsKeying()))
		{
			bIntKey = TRUE;
		}

		if (SUCCEEDED(m_pIDecklinkKeyer->get_DeviceSupportsExternalKeying()))
		{
			bExtKey = TRUE;
		}
	}

	// Check our keying mode settings.
	if ((KEY_INT == m_keyMode) && (FALSE == bIntKey))
	{
		m_keyMode = KEY_OFF;
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("KeyMode"), reinterpret_cast<const BYTE*>(&m_keyMode), sizeof(m_keyMode)));
	}
	else if ((KEY_EXT == m_keyMode) && (FALSE == bExtKey))
	{
		m_keyMode = KEY_OFF;
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("KeyMode"), reinterpret_cast<const BYTE*>(&m_keyMode), sizeof(m_keyMode)));
	}

	// update the key mode control state
	CButton* pButton = (CButton*)GetDlgItem(IDC_RADIO_KEYOFF);
	pButton->EnableWindow(bIntKey);
	pButton->SetCheck((KEY_OFF == m_keyMode) ? BST_CHECKED : BST_UNCHECKED);

	pButton = (CButton*)GetDlgItem(IDC_RADIO_KEYINTERNAL);
	pButton->EnableWindow(bIntKey);
	pButton->SetCheck((KEY_INT == m_keyMode) ? BST_CHECKED : BST_UNCHECKED);

	pButton = (CButton*)GetDlgItem(IDC_RADIO_KEYEXTERNAL);
	pButton->EnableWindow(bExtKey);
	pButton->SetCheck((KEY_EXT == m_keyMode) ? BST_CHECKED : BST_UNCHECKED);

	// If no keying is available, disable all the keying controls.
	BOOL bSliderEnable = FALSE;
	if (bIntKey || bExtKey)
	{
		bSliderEnable = TRUE;
	}

	CWnd* pWnd = GetDlgItem(IDC_SLIDER_ALPHA);
	pWnd->EnableWindow(bSliderEnable);
	pWnd = GetDlgItem(IDC_STATIC_ALPHA);
	pWnd->EnableWindow(bSliderEnable);
}

//-----------------------------------------------------------------------------
// EnumerateDevices
// Find all the capture devices in the system
HRESULT CDecklinkKeyDlg::EnumerateDevices(void)
{
	// first enumerate the system devices for the specifed class and filter name
	CComPtr<ICreateDevEnum> pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pSysDevEnum));

	if (SUCCEEDED(hr))
	{
		CComPtr<IEnumMoniker> pEnumCat = NULL;
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_TransmitCategory, &pEnumCat, 0);

		if (S_OK == hr)
		{
			IMoniker* pMoniker = NULL;
			while (S_OK == pEnumCat->Next(1, &pMoniker, NULL))
			{
				IPropertyBag* pPropBag = NULL;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&pPropBag));

				if (SUCCEEDED(hr))
				{
					VARIANT varName;
					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr))
					{
						size_t len = wcslen(varName.bstrVal) + 1;
						PWSTR pName = new WCHAR [len];
						StringCchCopyW(pName, len, varName.bstrVal);
						CW2TEX<> buf(varName.bstrVal);
						m_deviceCtrl.SetItemData(m_deviceCtrl.AddString(buf), (DWORD)pName);
					}

					VariantClear(&varName);
					
					// contained within a loop, decrement the reference count
					SAFE_RELEASE(pPropBag);
				}
				SAFE_RELEASE(pMoniker);
			}
		}
	}

	m_deviceCtrl.SetCurSel(0);

	return hr;
}

//-----------------------------------------------------------------------------
// PopulateFrameRateCtrl
// From the currently selected device populate the frame rate control with
// frame rates that are supported by the given frame dimensions.
HRESULT CDecklinkKeyDlg::PopulateFrameRateCtrl(CMediaType& mediaType)
{
	HRESULT hr = S_OK;
	set<REFERENCE_TIME> FrameRates;

	// Use the video capture filter of the selected device to retrieve the
	// supported video formats.  Create the video capture filter name from
	// the name of the selected video render device.
	basic_string<WCHAR> NameVideoCapture = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());
	basic_string<WCHAR>::size_type Index = NameVideoCapture.find(L"Render");
	if (basic_string<WCHAR>::npos != Index)
	{
		NameVideoCapture.erase(Index, 6);
		NameVideoCapture.insert(Index, L"Capture");
	}
	
	// Temporarily add the filter to the graph.
	CComPtr<IBaseFilter> pVideoCapture;
	hr = CDSUtils::AddFilter2(m_pGraph, CLSID_VideoInputDeviceCategory, NameVideoCapture.c_str(), &pVideoCapture);
	if (SUCCEEDED(hr))
	{
		// QI for the IAMStreamConfig interface.
		CComPtr<IAMStreamConfig> pIAMStreamConfig;
		hr = CDSUtils::FindPinInterface(pVideoCapture, L"Capture", IID_IAMStreamConfig, reinterpret_cast<void**>(&pIAMStreamConfig));
		if (SUCCEEDED(hr))
		{
			BITMAPINFOHEADER* pbmihFrameDimensions = CUtils::GetBMIHeader(mediaType);
			if (pbmihFrameDimensions)
			{
				// Loop through all the supported formats for those that match the frame dimensions
				// of the given media type.
				int count, size;
				hr = pIAMStreamConfig->GetNumberOfCapabilities(&count, &size);
				if (SUCCEEDED(hr))
				{
					if (sizeof(VIDEO_STREAM_CONFIG_CAPS) == size)
					{
						AM_MEDIA_TYPE* pmt = NULL;
						VIDEO_STREAM_CONFIG_CAPS vscc;

						for (int index=0; index<count; ++index)
						{
							hr = pIAMStreamConfig->GetStreamCaps(index, &pmt, reinterpret_cast<BYTE*>(&vscc));
							if (SUCCEEDED(hr))
							{
								BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(pmt);
								if (pbmih)
								{
									if ((pbmih->biWidth == pbmihFrameDimensions->biWidth) && (pbmih->biHeight == pbmihFrameDimensions->biHeight))
									{
										// Found a video format the matches the given frame dimensions,
										// add the frame rate to the list.
										FrameRates.insert(CUtils::GetAvgTimePerFrame(pmt));
									}
								}
								else
								{
									hr = E_POINTER;
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				hr = E_POINTER;
			}
		}

		// Remove the filter from the graph.
		m_pGraph->RemoveFilter(pVideoCapture);
	}

	if (SUCCEEDED(hr))
	{
		TCHAR* pszUnavailableString = TEXT("<Unavailable>");
		// Update the frame rate control with the new frame rates.
		// Attempt to keep the current selection, this way if the fill
		// has changed but has the same dimensions as the previous one
		// the frame rate selection will not change.
		if (!FrameRates.empty())
		{
			REFERENCE_TIME rtAvgTimePerFrameCur = 0;
			if (CB_ERR == m_frameRateCtrl.FindString(-1, pszUnavailableString))
			{
				// The combobox has a list of frame rates so cache the current selection.
				rtAvgTimePerFrameCur = m_frameRateCtrl.GetItemData(m_frameRateCtrl.GetCurSel());
			}

			m_frameRateCtrl.ResetContent();

			TCHAR buf[32];
			TCHAR* pszFormatString = TEXT("%.2lf frames per second");
			for (set<REFERENCE_TIME>::iterator it = FrameRates.begin(); it != FrameRates.end(); ++it)
			{
				ZeroMemory(buf, sizeof(buf));
				StringCbPrintf(buf, sizeof(buf), pszFormatString, (double)UNITS / (*it));
				m_frameRateCtrl.SetItemData(m_frameRateCtrl.AddString(buf), (DWORD)(*it));
			}

			int CurSel = 0;
			if (rtAvgTimePerFrameCur)
			{
				// One of the new frame rates matches the previous selection so
				// use this frame rate as the current selection for the new list.
				ZeroMemory(buf, sizeof(buf));
				StringCbPrintf(buf, sizeof(buf), pszFormatString, (double)UNITS / rtAvgTimePerFrameCur);
				int Index = m_frameRateCtrl.FindStringExact(-1, buf);
				if (CB_ERR != Index)
				{
					CurSel = Index;
				}
			}
			m_frameRateCtrl.SetCurSel(CurSel);
			m_frameRateCtrl.EnableWindow(TRUE);
		}
		else
		{
			m_frameRateCtrl.ResetContent();
			m_frameRateCtrl.AddString(pszUnavailableString);
			m_frameRateCtrl.SetCurSel(0);
			m_frameRateCtrl.EnableWindow(FALSE);
			hr = E_FAIL;
		}
	}

	return hr;
}
