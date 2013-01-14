//-----------------------------------------------------------------------------
// DecklinkPlaybackDlg.cpp
//
// Desc: DirectShow playback sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DecklinkPlayback.h"
#include "DecklinkPlaybackDlg.h"
#include "StaticURL.h"

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

// CDecklinkPlaybackDlg dialog



CDecklinkPlaybackDlg::CDecklinkPlaybackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecklinkPlaybackDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDecklinkPlaybackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RENDERFILE, m_renderFileCtrl);
	DDX_Control(pDX, IDC_SLIDER_SHUTTLE, m_shuttleCtrl);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_deviceCtrl);
	DDX_Control(pDX, IDC_EDIT_BLENDFILE, m_blendFileCtrl);
}

BEGIN_MESSAGE_MAP(CDecklinkPlaybackDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_LOOPPLAYBACK, OnBnClickedCheckLoopPlayback)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnCbnSelchangeComboDevice)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE2, OnBnClickedButtonBrowse2)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// OnInitDialog
//
BOOL CDecklinkPlaybackDlg::OnInitDialog()
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

	m_renderFile = "<Select File>";
	m_blendFile = "<Select File>";
	m_shuttleCtrl.SetRange(0, 100);
	REFERENCE_TIME t = 0;
	UpdateDisplay(IDC_STATIC_DURATION, &t);
	UpdateDisplay(IDC_STATIC_POSITION, &t);

	// create the sample grabber callback handler
	m_pSGCallback = new CSGCallbackHandler;

	m_hThread = NULL;
	m_hExitEvent = NULL;

	m_bLoopPlayback = FALSE;
	m_bCanSeek = FALSE;

	EnableControls();

	QueryRegistry();
	m_renderFileCtrl.SetWindowText(m_renderFile);
	m_blendFileCtrl.SetWindowText(m_blendFile);
	CButton* pLoopCtrl = (CButton*)GetDlgItem(IDC_CHECK_LOOPPLAYBACK);
	pLoopCtrl->SetCheck(m_bLoopPlayback ? BST_CHECKED : BST_UNCHECKED);

	EnumerateDevices();

	// create a skeleton playback graph
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&m_pGraph));
	if (SUCCEEDED(hr))
	{
#ifdef _DEBUG
		hr = CDSUtils::AddGraphToRot(m_pGraph, &m_ROTRegister);
#endif

		m_pIMediaFilter = m_pGraph;
		ASSERT(m_pIMediaFilter);

		// create a media detector object to determine the media types of the playback files
		hr = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, IID_IMediaDet, reinterpret_cast<void**>(&m_pIMediaDetector));
		if (SUCCEEDED(hr))
		{
			// Make graph send WM_GRAPHNOTIFY when it wants our attention see "Learning 
			// When an Event Occurs" in the DX9 documentation.
			m_pMediaEvent = m_pGraph;
			if (m_pMediaEvent)
			{
				hr = m_pMediaEvent->SetNotifyWindow(reinterpret_cast<OAHWND>(m_hWnd), WM_GRAPHNOTIFY, 0);
				if (SUCCEEDED(hr))
				{
					m_pControl = m_pGraph;
					if (m_pControl)
					{
						m_pMediaSeeking = m_pGraph;
						if (m_pMediaSeeking)
						{
							hr = CDSUtils::AddFilter(m_pGraph, CLSID_VideoRendererDefault, L"Video Renderer", &m_pVideoRenderer);
							if (SUCCEEDED(hr))
							{
								// configure the renderer to be in windowless mode
								CComQIPtr<IVMRFilterConfig, &IID_IVMRFilterConfig> pIVMRFilterConfig = m_pVideoRenderer;
								if (pIVMRFilterConfig)
								{
									hr = pIVMRFilterConfig->SetRenderingMode(VMRMode_Windowless);
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

								hr = CDSUtils::AddFilter(m_pGraph, CLSID_SampleGrabber, L"Sample Grabber", &m_pSampleGrabber);
								if (SUCCEEDED(hr))
								{
									// query for the sample grabber interface
									m_pISampleGrabber = m_pSampleGrabber;
									if (m_pISampleGrabber)
									{
										hr = CreateGraph();
										hr = CreateThreads();
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------
// DestroyWindow
//
BOOL CDecklinkPlaybackDlg::DestroyWindow()
{
	DestroyThreads();
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

	SAFE_DELETE(m_pSGCallback);

	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
// OnSysCommand
//
void CDecklinkPlaybackDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CDecklinkPlaybackDlg::OnPaint() 
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
/* HandleGraphEvent
 *	At the moment we just read the event, discard it and release
 *	memory used to store it.
 */
void CDecklinkPlaybackDlg::HandleGraphEvent(void) 
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
		if ((EC_COMPLETE == lEventCode) && m_pMediaSeeking)
		{
			if (m_bLoopPlayback)
			{
				m_pMediaSeeking->SetPositions(&startOfStream, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
			}
			else
			{
				OnBnClickedButtonStop();
			}
		}

		// just free memory associated with event
		m_pMediaEvent->FreeEventParams(lEventCode, lEventParam1, lEventParam2);
	}
}

//-----------------------------------------------------------------------------
/* WindowProc
 *	Have to add our own message handling loop to handle events from the 
 *	video window and to pass Window events onto it - this is so it
 *	redraws itself correctly etc.
 */
LRESULT CDecklinkPlaybackDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
// OnQueryDragIcon
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDecklinkPlaybackDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-----------------------------------------------------------------------------
// CreateGraph
// Build a playback graph.  This is a fairly crude implementation, it could be improved
// by searching for audio and video pins and connecting appropriately rather than relying
// too heavily on intelligent connect of the graph.
// Also as the infinite T is used for preview it is invariably connected BEFORE any decoder
// filters.  Rendering to Decklink and preview requires two decoder instances which is not
// very efficient.
HRESULT CDecklinkPlaybackDlg::CreateGraph(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph && m_pControl)
	{
		// store render filename
		USES_CONVERSION;
		WCHAR filename[MAX_PATH];
		wcsncpy(filename, A2W(m_renderFile), MAX_PATH);
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetString(TEXT("RenderFile"), reinterpret_cast<const BYTE*>(filename), sizeof(filename)));

		// store blend filename
		wcsncpy(filename, A2W(m_blendFile), MAX_PATH);
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetString(TEXT("LogoFile"), reinterpret_cast<const BYTE*>(filename), sizeof(filename)));
		m_pSGCallback->LoadBitmap(m_blendFile);

		// TODO: Check file extension and build AVI, MOV or WMV playback graph as appropriate
		if (-1 < m_renderFile.Find(".wmv"))
		{
			// build Windows Media playback graph
			hr = CreateGraphWM();
		}
		else
		{
			// build generic playback graph
			hr = CreateGraphGeneric();
		}

		// check the seeking capabilities of the source
		DWORD caps = 0;
		if (SUCCEEDED(m_pMediaSeeking->GetCapabilities(&caps)))
		{
			REFERENCE_TIME duration = 0;
			// if we can get the clip duration we can scrub
			m_bCanSeek = (caps & AM_SEEKING_CanGetDuration) ? TRUE : FALSE;
			if (m_bCanSeek)
			{
				m_pMediaSeeking->GetDuration(&duration);
			}
			duration /= UNITS;	// reduce the resolution to seconds
			UpdateDisplay(IDC_STATIC_DURATION, &duration);
			EnableControls();	// update the UI
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CreateGraphGeneric
// Build a generic playback graph.
HRESULT CDecklinkPlaybackDlg::CreateGraphGeneric(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph && m_pControl)
	{
		// The graph:
		//	   --------------- 	   -----------------         ----------------         ------------------     -----------------------
		//    | Source Filter |-->|                 |------>| Sample Grabber |------>|                  |-->| Decklink video render |
		//     ---------------    | Splitter/Parser |        ----------------        | Infinite pin tee |    -----------------------
		//                        |                 |    -------------------------   |                  |    ----------------
		//                        |                 |-->| Decklink audio renderer |  |                  |-->| Video renderer |
		//                         -----------------     -------------------------    ------------------     ----------------
		//

		CComPtr<IBaseFilter> pSource = NULL;
		USES_CONVERSION;	// for T2W macro
		hr = m_pGraph->AddSourceFilter(T2W(m_renderFile), L"Video Source Filter", &pSource);
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
				
				// add the audio renderer, create the audio renderer name from the video renderer name
				size_t len = wcslen(pNameVideo) + 1;
				PWSTR pNameAudio = new WCHAR [len];
				wcsncpy(pNameAudio, pNameVideo, len);
				PWSTR pTemp = wcsstr(pNameAudio, L"Video");
				if (pTemp)
				{
					wcsncpy(pTemp, L"Audio", 5);
				}
				CComPtr<IBaseFilter> pDecklinkAudioRenderer = NULL;
				hr = CDSUtils::AddFilter2(m_pGraph, CLSID_AudioRendererCategory, pNameAudio, &pDecklinkAudioRenderer);
				delete [] pNameAudio;
				if (SUCCEEDED(hr))
				{
					// determine the media type of the source file and use this to configure the sample grabber
					AM_MEDIA_TYPE amt = {0};
					hr = m_pIMediaDetector->put_Filename(T2W(m_renderFile));
					hr = m_pIMediaDetector->put_CurrentStream(0);
					hr = m_pIMediaDetector->get_StreamMediaType(&amt);
					if (SUCCEEDED(hr))
					{
						BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(&amt);
						if (pbmih)
						{
							// set the desired media type to receive in the sample grabber
							// and recalculate the critical parameters
							amt.subtype = MEDIASUBTYPE_RGB32;
							pbmih->biBitCount = 32;
							pbmih->biCompression = BI_RGB;
							pbmih->biSizeImage = (pbmih->biWidth * pbmih->biHeight * pbmih->biBitCount) >> 3;	// NOTE: This calculation is incorrect for v210 or r210
							amt.lSampleSize = pbmih->biSizeImage;

							hr = m_pSGCallback->SetMediaType(&amt);	// inform the sample grabber call back handler of the media type of the video frames
							hr = m_pISampleGrabber->SetMediaType(&amt);	// set the media type of the sample grabber filter
							if (SUCCEEDED(hr))
							{
								m_pISampleGrabber->SetCallback(m_pSGCallback, 0);	// callback with an IMediaSample interface
							}
						}
					}

					if (SUCCEEDED(hr))
					{
						// render the video path
						hr = CDSUtils::ConnectFilters(m_pGraph, pSource, NULL, m_pSampleGrabber, NULL);
						if (SUCCEEDED(hr))
						{
							IBaseFilter* pInfiniteT = NULL;
							hr = CDSUtils::AddFilter(m_pGraph, CLSID_InfTee, L"Infinite Pin Tee Filter", &pInfiniteT);
							if (SUCCEEDED(hr))
							{
								hr = CDSUtils::ConnectFilters(m_pGraph, m_pSampleGrabber, NULL, pInfiniteT, NULL);
								if (SUCCEEDED(hr))
								{
									hr = CDSUtils::ConnectFilters(m_pGraph, pInfiniteT, NULL, pDecklinkVideoRenderer, NULL);
									if (SUCCEEDED(hr))
									{
										// render the audio path, locate parser and connect audio pin
										CComPtr<IEnumFilters> pEnum = NULL;
										hr = m_pGraph->EnumFilters(&pEnum);
										if (SUCCEEDED(hr))
										{
											BOOL bFoundParser = FALSE;
											IBaseFilter* pFilter = NULL;
											while ((S_OK == pEnum->Next(1, &pFilter, NULL)) && !bFoundParser)
											{
												FILTER_INFO info = {0};
												hr = pFilter->QueryFilterInfo(&info);
												if (SUCCEEDED(hr))
												{
													SAFE_RELEASE(info.pGraph);

													CString name(info.achName);
													if ((-1 < name.Find("Splitter")) || (-1 < name.Find("Parser")))
													{
														// NOTE: This may fail if the clip does not have audio
														CDSUtils::ConnectFilters(m_pGraph, pFilter, NULL, pDecklinkAudioRenderer, NULL);
														bFoundParser = TRUE;
													}
												}
												
												SAFE_RELEASE(pFilter);
											}
										}
									}
								}
							}

							if (SUCCEEDED(hr))
							{
								// render the video preview, locate the other pin on the infinite T
								// and connect to the screen renderer
								hr = CDSUtils::ConnectFilters(m_pGraph, pInfiniteT, NULL, m_pVideoRenderer, NULL);
								if (SUCCEEDED(hr))
								{
									// push the first frame through the graph to have it visible on the desktop and the video output
									m_pControl->Pause();
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

	return hr;
}

//-----------------------------------------------------------------------------
// CreateGraphWM
// Build a Windows Media playback graph.
HRESULT CDecklinkPlaybackDlg::CreateGraphWM(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph && m_pControl)
	{
		CComPtr<IBaseFilter> pSource = NULL;
		USES_CONVERSION;	// for T2W macro
		hr = m_pGraph->AddSourceFilter(T2W(m_renderFile), L"Video Source Filter", &pSource);
		if (SUCCEEDED(hr))
		{
			// add the video renderer
			PWSTR pNameVideo = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());
			CComPtr<IBaseFilter> pDecklinkVideoRenderer = NULL;
			hr = CDSUtils::AddFilter2(m_pGraph, CLSID_TransmitCategory, pNameVideo, &pDecklinkVideoRenderer);
			if (SUCCEEDED(hr))
			{
				// add the audio renderer, create the audio renderer name from the video renderer name
				PWSTR pNameAudio = new WCHAR [wcslen(pNameVideo) + 1];
				wcsncpy(pNameAudio, pNameVideo, wcslen(pNameVideo) + 1);
				PWSTR pTemp = wcsstr(pNameAudio, L"Video");
				wcsncpy(pTemp, L"Audio", 5);
				CComPtr<IBaseFilter> pDecklinkAudioRenderer = NULL;
				hr = CDSUtils::AddFilter2(m_pGraph, CLSID_AudioRendererCategory, pNameAudio, &pDecklinkAudioRenderer);
				delete [] pNameAudio;
				if (SUCCEEDED(hr))
				{
					// render the video path
					CComPtr<IPin> pIPinOutput = NULL;
					CComPtr<IPin> pIPinInput = NULL;
					hr = CDSUtils::GetPin(pSource, &MEDIATYPE_Video, PINDIR_OUTPUT, &pIPinOutput);
					if (SUCCEEDED(hr))
					{
						hr = CDSUtils::GetUnconnectedPin(pDecklinkVideoRenderer, PINDIR_INPUT, &pIPinInput);
						if (SUCCEEDED(hr))
						{
							hr = m_pGraph->Connect(pIPinOutput, pIPinInput);
						}
					}

					if (SUCCEEDED(hr))
					{
						pIPinOutput = NULL;
						pIPinInput = NULL;
						// render the audio path
						hr = CDSUtils::GetPin(pSource, &MEDIATYPE_Audio, PINDIR_OUTPUT, &pIPinOutput);
						if (SUCCEEDED(hr))
						{
							hr = CDSUtils::GetUnconnectedPin(pDecklinkAudioRenderer, PINDIR_INPUT, &pIPinInput);
							if (SUCCEEDED(hr))
							{
								hr = m_pGraph->Connect(pIPinOutput, pIPinInput);
							}
						}
					}

					// render the video preview, locate the other pin on the infinite T and connect to the screen renderer
					if (SUCCEEDED(hr))
					{
//						hr = CDSUtils::ConnectFilters(m_pGraph, pInfiniteT, NULL, m_pVideoRenderer, NULL);
						if (SUCCEEDED(hr))
						{
							// push the first frame through the graph to have it visible on the desktop and the video output
							m_pControl->Pause();
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

	return hr;
}

//-----------------------------------------------------------------------------
// DestroyGraph
// Remove all filters from the graph apart from the renderers and infinite T
HRESULT CDecklinkPlaybackDlg::DestroyGraph(void)
{
	HRESULT hr = S_OK;

	if (m_pControl)
	{
		// stop playback
		m_pControl->Stop();
	}

	// cancel the callback
	if (m_pISampleGrabber)
	{
		m_pISampleGrabber->SetCallback(NULL, 0);
	}

	if (m_pIMediaFilter)
	{
		m_pIMediaFilter->SetSyncSource(NULL);
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
				FILTER_INFO filterInfo = {0};
				hr = pFilter->QueryFilterInfo(&filterInfo);
				if (SUCCEEDED(hr))
				{
					SAFE_RELEASE(filterInfo.pGraph);

//					if ((NULL == wcsstr(filterInfo.achName, L"Video Renderer")) && (NULL == wcsstr(filterInfo.achName, L"Infinite")) && (NULL == wcsstr(filterInfo.achName, L"Sample Grabber")))
					if ((NULL == wcsstr(filterInfo.achName, L"Video Renderer")) && (NULL == wcsstr(filterInfo.achName, L"Sample Grabber")))
					{
						// not a renderer or infinite tee filter so remove from graph
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
// OnBnClickedButtonBrowse
//
void CDecklinkPlaybackDlg::OnBnClickedButtonBrowse()
{
	char BASED_CODE szFilters[] = "Windows Media Files|*.avi;*.mov;*.mpg;*.wmv|All Files (*.*)|*.*||";

	CFileDialog FileDlg(TRUE, "Windows Media Files", "*.avi;*.mov;*.mpg;*.wmv", 0, szFilters, this);

	if (FileDlg.DoModal() == IDOK)
	{
		m_renderFile = FileDlg.GetPathName();
		m_renderFileCtrl.SetWindowText(m_renderFile);
		
		DestroyGraph();
		CreateGraph();
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonBrowse2
//
void CDecklinkPlaybackDlg::OnBnClickedButtonBrowse2()
{
	char BASED_CODE szFilters[] = "Windows Bitmap Files|*.bmp|All Files (*.*)|*.*||";

	CFileDialog FileDlg(TRUE, "Windows Bitmap Files", "*.bmp", 0, szFilters, this);

	if (FileDlg.DoModal() == IDOK)
	{
		m_blendFile = FileDlg.GetPathName();
		m_blendFileCtrl.SetWindowText(m_blendFile);
		
		if (m_pSGCallback)
		{
			HRESULT hr = m_pSGCallback->LoadBitmap(m_blendFile);
			if (SUCCEEDED(hr))
			{
				// store filename
				USES_CONVERSION;
				WCHAR filename[MAX_PATH];
				wcsncpy(filename, A2W(m_blendFile), MAX_PATH);
				EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetString(TEXT("LogoFile"), reinterpret_cast<const BYTE*>(filename), sizeof(filename)));
			}
		}
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonPlay
//
void CDecklinkPlaybackDlg::OnBnClickedButtonPlay()
{
	// get the shuttle control slider position
	if (m_pMediaSeeking && m_bCanSeek)
	{
		LONGLONG duration = 0;
		HRESULT hr = m_pMediaSeeking->GetDuration(&duration);
		if (SUCCEEDED(hr))
		{
			LONGLONG current = 0;
			if (0 == duration)
			{
				// avoid div zero error
				duration = 1;
			}

			// set the media start position in sympathy with the shuttle slider position
			current = (LONGLONG)((float)m_shuttleCtrl.GetPos() / 100 * duration);
			m_pMediaSeeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, NULL, 0);
		}
	}

	if (m_pControl)
	{
		m_pControl->Run();
		DisableControls();
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonStop
//
void CDecklinkPlaybackDlg::OnBnClickedButtonStop()
{
	// stop graph
	if (m_pControl)
	{
		m_pControl->Stop();
		EnableControls();
	}

	// reset media position
	if (m_pMediaSeeking)
	{
		LONGLONG current = 0;
		m_pMediaSeeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, NULL, 0);
	}

	// reset the shuttle slider position
	m_shuttleCtrl.SetPos(0);
	m_shuttleCtrl.RedrawWindow();
}

//-----------------------------------------------------------------------------
// EnableControls
//
void CDecklinkPlaybackDlg::EnableControls(void)
{
	CWnd* pWnd = GetDlgItem(IDC_EDIT_RENDERFILE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_BROWSE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_PLAY);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_STOP);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_SLIDER_SHUTTLE);
	pWnd->EnableWindow(m_bCanSeek ? TRUE : FALSE);
}

//-----------------------------------------------------------------------------
// DisableControls
//
void CDecklinkPlaybackDlg::DisableControls(void)
{
	CWnd* pWnd = GetDlgItem(IDC_EDIT_RENDERFILE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_BROWSE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_PLAY);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_STOP);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_SLIDER_SHUTTLE);
	pWnd->EnableWindow(m_bCanSeek ? TRUE : FALSE);
}

//-----------------------------------------------------------------------------
// CreateThread
//
HRESULT CDecklinkPlaybackDlg::CreateThreads(void)
{
	HRESULT hr = S_OK;

	m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, "Decklink Playback Sample Event");
	if (m_hExitEvent)
	{
		m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
		if (NULL == m_hThread)
		{
			hr = AmGetLastErrorToHResult();
		}
	}
	else
	{
		hr = AmGetLastErrorToHResult();
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// DestroyThread
//
HRESULT CDecklinkPlaybackDlg::DestroyThreads(void)
{
	HRESULT hr = S_OK;

	// signal the thread to exit
	SetEvent(m_hExitEvent);
	// wait for thread to exit
	EXECUTE_ASSERT(WAIT_OBJECT_0 == WaitForSingleObject(m_hThread, 10000));
	
	m_hExitEvent = NULL;
	m_hThread = NULL;
	
	return hr;
}

//-----------------------------------------------------------------------------
// ThreadProc
// Static function that wraps class thread implementation
DWORD WINAPI CDecklinkPlaybackDlg::ThreadProc(LPVOID lpParameter)
{
	ASSERT(lpParameter);
	CDecklinkPlaybackDlg* pDlg = reinterpret_cast<CDecklinkPlaybackDlg*>(lpParameter);
	return pDlg->Thread();
}

//-----------------------------------------------------------------------------
// Thread
// Actual class thread
DWORD CDecklinkPlaybackDlg::Thread(void)
{
	HRESULT hr = S_OK;
	LONGLONG LastPos = 0;
	BOOL bRunning = TRUE;
	while (bRunning)
	{
		switch (WaitForSingleObject(m_hExitEvent, 10))
		{
			default:
			case WAIT_ABANDONED:
			case WAIT_OBJECT_0:
				bRunning = FALSE;
				break;

			case WAIT_TIMEOUT:
				if (m_pMediaSeeking && m_bCanSeek)
				{
					OAFilterState filterState;
					hr = m_pControl->GetState(0, &filterState);
					if (SUCCEEDED(hr) && (State_Running == filterState))
					{
						// update the slider playback position
						LONGLONG duration = 0;
						hr = m_pMediaSeeking->GetDuration(&duration);
						if (SUCCEEDED(hr))
						{
							LONGLONG current = 0;
							hr = m_pMediaSeeking->GetCurrentPosition(&current);
							if (SUCCEEDED(hr))
							{
								duration /= UNITS;	// reduce the frequency of the update to once per second
								current /= UNITS;
								if (LastPos != current)
								{
									if (0 == duration)
									{
										// avoid div zero error
										duration = 1;
									}
									
									// update shuttle control to show current playback position
									int pos = (int)(((float)current / duration * 100) + 0.5);
									m_shuttleCtrl.SetPos(pos);
									m_shuttleCtrl.RedrawWindow();

									// update display
									UpdateDisplay(IDC_STATIC_POSITION, &current);
									
									LastPos = current;
								}
							}
						}
					}
				}
				break;		
		}	
	}

	return 0;
}

//-----------------------------------------------------------------------------
// OnNMReleasedcaptureSliderShuttle
// Updates the current media position
void CDecklinkPlaybackDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_pMediaSeeking)
	{
		LONGLONG duration = 0;
		HRESULT hr = m_pMediaSeeking->GetDuration(&duration);
		if (SUCCEEDED(hr))
		{
			LONGLONG current = 0;
			if (0 == duration)
			{
				// avoid div zero error
				duration = 1;
			}

			// set the media start position in sympathy with the shuttle slider position
			current = (LONGLONG)((float)m_shuttleCtrl.GetPos() / 100 * duration);
			m_pMediaSeeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, NULL, 0);
			
			// if the graph is stopped temporarily pause to get a video frame through to the renderers
			FILTER_STATE filterState;
			hr = m_pControl->GetState(100, (OAFilterState*)&filterState);
			if (State_Stopped == filterState)
			{
				m_pControl->Pause();
				hr = m_pControl->GetState(INFINITE, (OAFilterState*)&filterState);
				m_pControl->Stop();
			}
		}
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

//-----------------------------------------------------------------------------
// OnBnClickedCheckLoopPlayback
//
void CDecklinkPlaybackDlg::OnBnClickedCheckLoopPlayback()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_LOOPPLAYBACK);
	if (pCheck)
	{
		m_bLoopPlayback = (BST_CHECKED == pCheck->GetCheck())? TRUE : FALSE;
		
		// save the new state
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("Loop"), reinterpret_cast<const BYTE*>(&m_bLoopPlayback), sizeof(m_bLoopPlayback)));
	}
}

//-----------------------------------------------------------------------------
// UpdateDisplay
// Format an hour, minute and second display string
void CDecklinkPlaybackDlg::UpdateDisplay(int ctrlID, REFERENCE_TIME* pTime)
{
	CStatic* pDisp = (CStatic*)GetDlgItem(ctrlID);
	if (pTime && pDisp)
	{
		int s = (int)((*pTime % 3600) % 60);
		int m = (int)((*pTime % 3600) / 60);
		int h = (int)(*pTime / 3600);

		TCHAR szText[32];
		StringCbPrintf(szText, sizeof(szText), "%02d:%02d:%02d", h, m, s);
		pDisp->SetWindowText(szText);
	}
}

//-----------------------------------------------------------------------------
// QueryRegistry
// retrieve previous settings from registry
void CDecklinkPlaybackDlg::QueryRegistry(void)
{
	if (ERROR_SUCCESS == m_regUtils.Open(TEXT("DecklinkPlaybackSample")))
	{
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetBinary(TEXT("Loop"), reinterpret_cast<LPBYTE>(&m_bLoopPlayback), sizeof(m_bLoopPlayback)));

		WCHAR filename[MAX_PATH];
		ZeroMemory(filename, sizeof(filename));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetString(TEXT("RenderFile"), reinterpret_cast<LPBYTE>(filename), sizeof(filename)));
		m_renderFile = filename;

		ZeroMemory(filename, sizeof(filename));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetString(TEXT("LogoFile"), reinterpret_cast<LPBYTE>(filename), sizeof(filename)));
		m_blendFile = filename;
	}
	else
	{
		// create the key and registry values
		if (ERROR_SUCCESS == m_regUtils.Create(TEXT("DecklinkPlaybackSample")))
		{
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("Loop"), reinterpret_cast<const BYTE*>(&m_bLoopPlayback), sizeof(m_bLoopPlayback)));

			USES_CONVERSION;
			WCHAR filename[MAX_PATH];
			wcsncpy(filename, A2W(m_renderFile), MAX_PATH);
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetString(TEXT("RenderFile"), reinterpret_cast<const BYTE*>(filename), sizeof(filename)));

			wcsncpy(filename, A2W(m_blendFile), MAX_PATH);
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetString(TEXT("LogoFile"), reinterpret_cast<const BYTE*>(filename), sizeof(filename)));
		}
	}
}

//-----------------------------------------------------------------------------
// EnumerateDevices
// Find all the capture devices in the system
HRESULT CDecklinkPlaybackDlg::EnumerateDevices(void)
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
// OnCbnSelchangeComboDevice
// Rebuild the playback graph with the new device
void CDecklinkPlaybackDlg::OnCbnSelchangeComboDevice()
{
	OnBnClickedButtonStop();	// stop the graph if running
	DestroyGraph();	// destroy the old graph
	CreateGraph();	// create a graph with the new device
}
