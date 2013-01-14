//-----------------------------------------------------------------------------
// DecklinkCaptureDlg.cpp
//
// Desc: DirectShow capture sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DecklinkCapture.h"
#include "DecklinkCaptureDlg.h"
#include "StaticURL.h"

#include "DecklinkFilters_h.h"

#undef lstrlenW

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
// CDecklinkCaptureDlg dialog
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor
//
CDecklinkCaptureDlg::CDecklinkCaptureDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecklinkCaptureDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-----------------------------------------------------------------------------
// DoDataExchange
// 
void CDecklinkCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_VIDEOFORMATS, m_videoFormatCtrl);
	DDX_Control(pDX, IDC_COMBO_AUDIOFORMATS, m_audioFormatCtrl);
	DDX_Control(pDX, IDC_EDIT_CAPTUREFILE, m_captureFileCtrl);
	DDX_Control(pDX, IDC_COMBO_COMPRESSION, m_compressionCtrl);
	DDX_Control(pDX, IDC_COMBO_VIDEODEVICE, m_videoDeviceCtrl);
	DDX_Control(pDX, IDC_COMBO_AUDIODEVICE, m_audioDeviceCtrl);
}

BEGIN_MESSAGE_MAP(CDecklinkCaptureDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_COMBO_VIDEOFORMATS, OnCbnSelchangeComboVideoformats)
	ON_CBN_SELCHANGE(IDC_COMBO_AUDIOFORMATS, OnCbnSelchangeComboAudioformats)
	ON_BN_CLICKED(IDC_CHECK_AUDIOMUTE, OnBnClickedCheckAudiomute)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
	ON_CBN_SELCHANGE(IDC_COMBO_COMPRESSION, OnCbnSelchangeComboCompression)
	ON_CBN_SELCHANGE(IDC_COMBO_VIDEODEVICE, OnCbnSelchangeComboVideodevice)
	ON_CBN_SELCHANGE(IDC_COMBO_AUDIODEVICE, OnCbnSelchangeComboAudiodevice)
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// CDecklinkCaptureDlg message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// OnInitDialog
// Called before the dialog is displayed, use this message handler to initialise
// our app
BOOL CDecklinkCaptureDlg::OnInitDialog()
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

	// create a basic capture graph and preview the incoming video
	m_pAudioCapture = NULL;

	m_ROTRegister = 0;
	m_bAudioMute = FALSE;
	m_compressor = 0;
	m_bEnableCompressionCtrl = TRUE;

	m_captureFile = "<Select File>";

	// initialise default video media type
	ZeroMemory(&m_vihDefault, sizeof(m_vihDefault));
	m_vihDefault.AvgTimePerFrame = 333667;
	m_vihDefault.bmiHeader.biWidth = 720;
	m_vihDefault.bmiHeader.biHeight = 486;
	m_vihDefault.bmiHeader.biBitCount = 16;
	m_vihDefault.bmiHeader.biCompression = 'YVYU';

	// initialise default audio media type
	ZeroMemory(&m_wfexDefault, sizeof(m_wfexDefault));
	m_wfexDefault.nChannels = 2;	// the only field of interest

	// retrieve last state
	QueryRegistry();

	m_captureFileCtrl.SetWindowText(m_captureFile);
	
	// create notification events for video input state changes (N.B. InputStatusChangeEvent is manual reset)
	m_hInputStatusChangeEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	m_hStopInputStatusThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	EnableControls();

	// create a preview graph
	// add the filters that will be used by all the graphs; preview, uncompressed capture, dv capture, 
	// mpeg capture and windows media capture
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&m_pGraph));
	if (SUCCEEDED(hr))
	{
#ifdef _DEBUG
		hr = CDSUtils::AddGraphToRot(m_pGraph, &m_ROTRegister);
#endif
		m_pControl = m_pGraph;
		if (m_pControl)
		{
			// locate video screen renderer for the preview window
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

				// locate the video capture devices
				hr = PopulateDeviceControl(&CLSID_VideoInputDeviceCategory, &m_videoDeviceCtrl);
				if (SUCCEEDED(hr))
				{
					hr = PopulateDeviceControl(&CLSID_AudioInputDeviceCategory, &m_audioDeviceCtrl);
					if (SUCCEEDED(hr))
					{
						PWSTR pVideoName = (PWSTR)m_videoDeviceCtrl.GetItemData(m_videoDeviceCtrl.SetCurSel(0));
						if (pVideoName)
						{
							hr = CDSUtils::AddFilter2(m_pGraph, CLSID_VideoInputDeviceCategory, pVideoName, &m_pVideoCapture);
							if (SUCCEEDED(hr))
							{
								PopulateVideoControl();	// populate the video format control with the video formats of the currently selected device
							}
						}

						PWSTR pAudioName = (PWSTR)m_audioDeviceCtrl.GetItemData(m_audioDeviceCtrl.SetCurSel(0));
						if (pAudioName)
						{
							hr = CDSUtils::AddFilter2(m_pGraph, CLSID_AudioInputDeviceCategory, pAudioName, &m_pAudioCapture);
							if (SUCCEEDED(hr))
							{
								PopulateAudioControl();	// populate the audio format control with the audio formats of the currently selected device
							}
						}

						PopulateCompressionControl();

						hr = CreatePreviewGraph();
					}
				}
			}
		}
	}

	m_hInputStatusThread = NULL;
	if (m_hStopInputStatusThreadEvent)
	{
		m_hInputStatusThread = CreateThread(NULL, 0, CDecklinkCaptureDlg::InputStatusThreadWrapper, reinterpret_cast<LPVOID>(this), 0, NULL);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------
// DestroyWindow
// Called when the window is being destroyed, clean up and free all resources.
BOOL CDecklinkCaptureDlg::DestroyWindow()
{
	// terminate the thread
	SetEvent(m_hStopInputStatusThreadEvent);
	WaitForSingleObject(m_hInputStatusThread, 10000);
	CloseHandle(m_hInputStatusThread);
	CloseHandle(m_hStopInputStatusThreadEvent);
	CloseHandle(m_hInputStatusChangeEvent);
	
	m_regUtils.Close();
#ifdef _DEBUG
	CDSUtils::RemoveGraphFromRot(m_ROTRegister);
#endif
	DestroyGraph();

	// free mediatypes attached to format controls
	int count = m_videoFormatCtrl.GetCount();
	int item;
	for (item=0; item<count; ++item)
	{
		DeleteMediaType((AM_MEDIA_TYPE*)m_videoFormatCtrl.GetItemData(item));
	}

	count = m_audioFormatCtrl.GetCount();
	for (item=0; item<count; ++item)
	{
		DeleteMediaType((AM_MEDIA_TYPE*)m_audioFormatCtrl.GetItemData(item));
	}

	// release the device names attached to the item's data
	count = m_videoDeviceCtrl.GetCount();
	for (item=0; item<count; ++item)
	{
		PWSTR pName = (PWSTR)m_videoDeviceCtrl.GetItemData(item);
		delete [] pName;
	}

	count = m_audioDeviceCtrl.GetCount();
	for (item=0; item<count; ++item)
	{
		PWSTR pName = (PWSTR)m_audioDeviceCtrl.GetItemData(item);
		delete [] pName;
	}

	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
// OnSysCommand
//
void CDecklinkCaptureDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CDecklinkCaptureDlg::OnPaint() 
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
// OnQueryDragIcon
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDecklinkCaptureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-----------------------------------------------------------------------------
// CreatePreviewGraph
// Create a graph to preview the input
// NOTE: There are many ways of building graphs, you could opt for the ICaptureGraphBuilder interface which would
// make things are lot simpler, however it doesn't always build the most efficient graphs.
HRESULT CDecklinkCaptureDlg::CreatePreviewGraph()
{
	HRESULT hr = S_OK;

	if (m_pGraph)
	{
		// locate smart-T
		// NOTE: The smart-T appears to hold references to its upstream connections even when its input pin
		// is diconnected.  The smart-T has to be removed from the graph in order to clear these references which
		// is why the filter is enumerated and added every time the preview graph is built and removed whenever
		// it is destroyed.
		ASSERT(!m_pSmartT);
		hr = CDSUtils::AddFilter(m_pGraph, CLSID_SmartTee, L"Smart Tee", &m_pSmartT);
		if (SUCCEEDED(hr))
		{
			// DV preview is slightly different to all other previews
			if (ENC_DV != m_compressionCtrl.GetItemData(m_compressionCtrl.GetCurSel()))
			{
				// uncompressed, mpeg and wm preview
				// create the following:
				//
				//  Decklink Video Capture -> Smart-T -> AVI Decompressor -> Video Renderer
				//  Decklink Audio Capture -> Default Audio Renderer
				//

				// render the preview pin on the smart-T filter
				// first connect the Decklink video capture pin to the smart-T
				hr = CDSUtils::ConnectFilters(m_pGraph, m_pVideoCapture, NULL, m_pSmartT, NULL);
				if (SUCCEEDED(hr))
				{
					// now connect the preview pin of the smart-T to the video renderer
					hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, L"Preview", m_pVideoRenderer, NULL);
				}
			}
			else
			{
				// DV Preview
				// create the following:
				//
				//  Decklink Video Capture -> AVI Decompressor -> Smart-T -> Colour Space Converter -> Video Renderer
				//  Decklink Audio Capture -> Default Audio Renderer
				//
				// this is a more efficient graph than created by the ICaptureGraphBuilder2 interface

				// add the AVI decompressor and colour space converter filters
				CComPtr<IBaseFilter> pAVIDecompressor = NULL;
				hr = CDSUtils::AddFilter(m_pGraph, CLSID_AVIDec, L"AVI Decompressor", &pAVIDecompressor);
				if (SUCCEEDED(hr))
				{
					CComPtr<IBaseFilter> pColourSpaceConverter = NULL;
					hr = CDSUtils::AddFilter(m_pGraph, CLSID_Colour, L"Color Space Converter", &pColourSpaceConverter);
					if (SUCCEEDED(hr))
					{
						// conect the Decklink video capture pin to the AVI decompressor
						hr = CDSUtils::ConnectFilters(m_pGraph, m_pVideoCapture, NULL, pAVIDecompressor, NULL);
						if (SUCCEEDED(hr))
						{
							// connect AVI decompressor to the smart-T
							hr = CDSUtils::ConnectFilters(m_pGraph, pAVIDecompressor, NULL, m_pSmartT, NULL);
							if (SUCCEEDED(hr))
							{
								// connect the preview pin of the smart-T to the colour space converter
								hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, L"Preview", pColourSpaceConverter, NULL);
								if (SUCCEEDED(hr))
								{
									// connect the colour space converter to the video renderer
									hr = CDSUtils::ConnectFilters(m_pGraph, pColourSpaceConverter, NULL, m_pVideoRenderer, NULL);
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

	if (SUCCEEDED(hr))
	{
		// optionally connect the audio path
		if (FALSE == m_bAudioMute)
		{
			// connect the Decklink audio capture pin to the mux
			hr = CDSUtils::RenderFilter(m_pGraph, m_pAudioCapture, L"Capture");
		}

		// run the graph so that we can preview the input video
		if (m_pControl)
		{
			hr = m_pControl->Run();
		}
		else
		{
			hr = E_POINTER;
		}
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CreateCaptureGraph
// Create a graph to capture the input
HRESULT CDecklinkCaptureDlg::CreateCaptureGraph()
{
	HRESULT hr = S_OK;

	// tack the file writer onto the preview graph
	if (m_pGraph && m_pControl)
	{
		// stop the graph as we are about to modify it
		m_pControl->Stop();

		// remove the default audio renderer so the Decklink audio capture filter
		// can be connected to the AVI mux, we will not preview audio whilst capturing
		CComPtr<IPin> pIPinOutput = NULL;
		hr = CDSUtils::GetPin(m_pAudioCapture, L"Capture", &pIPinOutput);
		if (SUCCEEDED(hr))
		{
			// to disconnect both pins must be disconnected
			// find the pin connected to the Decklink audio capture pin
			CComPtr<IPin> pIPinConnection = NULL;
			hr = pIPinOutput->ConnectedTo(&pIPinConnection);
			if (SUCCEEDED(hr))
			{
				// disconnect the pins
				hr = m_pGraph->Disconnect(pIPinOutput);
				hr = m_pGraph->Disconnect(pIPinConnection);
				
				// get the owning filter of the downstream pin and remove it from the graph
				PIN_INFO pinInfo = {0};
				hr = pIPinConnection->QueryPinInfo(&pinInfo);
				if (SUCCEEDED(hr))
				{
					if (pinInfo.pFilter)
					{
						hr = m_pGraph->RemoveFilter(pinInfo.pFilter);
						pinInfo.pFilter->Release();
					}
				}
			}
		}

		// retrieve the capture filename
		m_captureFileCtrl.GetWindowText(m_captureFile);
		// store filename
		USES_CONVERSION;
		WCHAR captureFile[MAX_PATH];
		wcsncpy(captureFile, A2W(m_captureFile), MAX_PATH);
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetString(TEXT("CaptureFile"), reinterpret_cast<const BYTE*>(captureFile), sizeof(captureFile)));

		// decide the type of capture graph to build
		switch (m_compressionCtrl.GetItemData(m_compressionCtrl.GetCurSel()))
		{
			default:
			case ENC_NONE:
				hr = CreateUncompressedCaptureGraph();
				break;

			case ENC_DV:
				hr = CreateDVCaptureGraph();
				break;
				
			case ENC_WM:
				hr = CreateWMCaptureGraph();
				break;

			case ENC_MJ:
				hr = CreateMJCaptureGraph();
				break;
		}

		if (FAILED(hr))
		{
			// there was a problem building the capture graph, issue a message
			// and rebuild preview graph
			TCHAR buffer[128];
			StringCbPrintf(buffer, sizeof(buffer), TEXT("The error 0x%08lx was detected when creating the capture graph with the following file name:\r\n\r\n'%s'"), hr, m_captureFile);
			MessageBox(buffer, _T("Error"), MB_ICONERROR);
			OnBnClickedButtonStop();	// destroy broken capture graph, build preview graph and enable controls
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CreateUncompressedCaptureGraph
// Create an optimum uncompressed capture graph
HRESULT CDecklinkCaptureDlg::CreateUncompressedCaptureGraph()
{
	HRESULT hr = S_OK;

	// uncompressed capture
	// locate the AVI mux and file writer filters and add them to the graph
	CComPtr<IBaseFilter> pAVIMux = NULL;
	hr = CDSUtils::AddFilter(m_pGraph, CLSID_AviDest , L"AVI Mux", &pAVIMux);
	if (SUCCEEDED(hr))
	{
		CComPtr<IBaseFilter> pFileWriter = NULL;
		hr = CDSUtils::AddFilter(m_pGraph, CLSID_FileWriter, L"File writer", &pFileWriter);
		if (SUCCEEDED(hr))
		{
			// set the output filename
			CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pIFS = pFileWriter;
			if (pIFS)
			{
				USES_CONVERSION;	// for T2W macro
				hr = pIFS->SetFileName(T2W(m_captureFile), NULL);
				if (SUCCEEDED(hr))
				{
					// connect the smart-T capture pin to the mux
					hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, L"Capture", pAVIMux, NULL);
					if (SUCCEEDED(hr))
					{
						// connect the mux to the file writer
						hr = CDSUtils::ConnectFilters(m_pGraph, pAVIMux, NULL, pFileWriter, NULL);
						if (SUCCEEDED(hr))
						{
							// video path connected now optionally connect the audio path
							if (FALSE == m_bAudioMute)
							{
								// connect the Decklink audio capture pin to the mux
								hr = CDSUtils::ConnectFilters(m_pGraph, m_pAudioCapture, L"Capture", pAVIMux, NULL);
							}

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

	return hr;
}

//-----------------------------------------------------------------------------
// CreateDVCaptureGraph
// Create an optimum DV capture graph
// NOTE: that this will only work for SD
HRESULT CDecklinkCaptureDlg::CreateDVCaptureGraph()
{
	HRESULT hr = S_OK;

	// locate the DV encoder, AVI mux and file writer filters and add them to the graph
	CComPtr<IBaseFilter> pEncoder = NULL;
	hr = CDSUtils::AddFilter(m_pGraph, CLSID_DVVideoEnc, L"DV Video Encoder", &pEncoder);
	if (SUCCEEDED(hr))
	{
		CComPtr<IBaseFilter> pAVIMux = NULL;
		hr = CDSUtils::AddFilter(m_pGraph, CLSID_AviDest , L"AVI Mux", &pAVIMux);
		if (SUCCEEDED(hr))
		{
			CComPtr<IBaseFilter> pFileWriter = NULL;
			hr = CDSUtils::AddFilter(m_pGraph, CLSID_FileWriter, L"File writer", &pFileWriter);
			if (SUCCEEDED(hr))
			{
				// set the output filename
				CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pIFS = pFileWriter;
				if (pIFS)
				{
					USES_CONVERSION;	// for T2W macro
					hr = pIFS->SetFileName(T2W(m_captureFile), NULL);
					if (SUCCEEDED(hr))
					{
						// configure the DV encoder
						CComQIPtr<IDVEnc, &IID_IDVEnc> pIDV = pEncoder;
						if (pIDV)
						{
							// located a DV compression filter, set the format
							int videoFormat, dvFormat, resolution;
							hr = pIDV->get_IFormatResolution(&videoFormat, &dvFormat, &resolution, FALSE, NULL);
							if (SUCCEEDED(hr))
							{
								ASSERT(DVENCODERFORMAT_DVSD == dvFormat);
								ASSERT(DVENCODERRESOLUTION_720x480 == resolution);

								if ((DVENCODERVIDEOFORMAT_NTSC == videoFormat) && (576 == m_vihDefault.bmiHeader.biHeight))
								{
									// set the encoder to PAL if its NTSC
									videoFormat = DVENCODERVIDEOFORMAT_PAL;
									hr = pIDV->put_IFormatResolution(videoFormat, dvFormat, resolution, FALSE, NULL);
								}
								else if ((DVENCODERVIDEOFORMAT_PAL == videoFormat) && (486 == m_vihDefault.bmiHeader.biHeight))
								{
									// set the encoder to NTSC if its PAL
									videoFormat = DVENCODERVIDEOFORMAT_NTSC;
									hr = pIDV->put_IFormatResolution(videoFormat, dvFormat, resolution, FALSE, NULL);
								}
							}
						}

						if (SUCCEEDED(hr))
						{
							// if the format is PAL, insert the Decklink field swap filter, PAL DV is the opposite
							// field order to PAL SD
							if (576 == m_vihDefault.bmiHeader.biHeight)
							{
								CComPtr<IBaseFilter> pPALFieldSwap = NULL;
								hr = CDSUtils::AddFilter(m_pGraph, CLSID_DecklinkFieldSwap, L"Decklink PAL Field Swap", &pPALFieldSwap);
								if (SUCCEEDED(hr))
								{
									// connect the smart-T capture pin to the PAL field swap filter
									hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, L"Capture", pPALFieldSwap, NULL);
									if (SUCCEEDED(hr))
									{
										// connect the field swap filter to the DV encoder
										hr = CDSUtils::ConnectFilters(m_pGraph, pPALFieldSwap, NULL, pEncoder, NULL);
									}
								}
							}
							else
							{
								// connect the smart-T capture pin to the DV Encoder
								hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, L"Capture", pEncoder, NULL);
							}

							if (SUCCEEDED(hr))
							{
								// connect the DV encoder output to the AVI mux
								hr = CDSUtils::ConnectFilters(m_pGraph, pEncoder, NULL, pAVIMux, NULL);
								if (SUCCEEDED(hr))
								{
									// connect the mux to the file writer
									hr = CDSUtils::ConnectFilters(m_pGraph, pAVIMux, NULL, pFileWriter, NULL);
									if (SUCCEEDED(hr))
									{
										// video path connected now optionally connect the audio path
										if (FALSE == m_bAudioMute)
										{
											// connect the Decklink audio capture pin to the mux
											hr = CDSUtils::ConnectFilters(m_pGraph, m_pAudioCapture, L"Capture", pAVIMux, NULL);
										}

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
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CreateWMCaptureGraph
// Create an optimum Windows Media capture graph
HRESULT CDecklinkCaptureDlg::CreateWMCaptureGraph()
{
	HRESULT hr = S_OK;

	// locate the asf writer filter and add it to the graph
	CComPtr<IBaseFilter> pASFWriter = NULL;
	hr = CDSUtils::AddFilter(m_pGraph, CLSID_WMAsfWriter, L"WM ASF Writer", &pASFWriter);
	if (SUCCEEDED(hr))
	{
		// set the output filename
		CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pIFS = pASFWriter;
		if (pIFS)
		{
			USES_CONVERSION;	// for T2W macro
			hr = pIFS->SetFileName(T2W(m_captureFile), NULL);
			if (SUCCEEDED(hr))
			{
				hr = ConfigureWMEncoder(pASFWriter);
			}
		}

		if (SUCCEEDED(hr))
		{
			if (FALSE == m_bAudioMute)
			{
				// connect the Decklink audio capture pin to the ASF writer
				hr = CDSUtils::ConnectFilters(m_pGraph, m_pAudioCapture, pASFWriter, &MEDIATYPE_Audio);
			}
			
			if (SUCCEEDED(hr))
			{
				// connect the smart-T capture pin to the ASF writer
				hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, pASFWriter, &MEDIATYPE_Video);

				if (SUCCEEDED(hr))
				{
					m_pControl->Run();
				}
			}
		}
	}

	return hr;
}

//-----------------------------------------------------------------------------
// ConfigureWMEncoder
// Configure the Windows Media encoder
HRESULT CDecklinkCaptureDlg::ConfigureWMEncoder(IBaseFilter* pASFWriter)
{
	HRESULT hr = S_OK;
	
	// modify the video output resolution of a system profile
	if (pASFWriter)
	{
		// simple system profile encoding
		CComQIPtr<IConfigAsfWriter, &IID_IConfigAsfWriter> pICW = pASFWriter;
		if (pICW)
		{
//			NOTE: You could just use the following for a default system profile
//			hr = pICW->ConfigureFilterUsingProfileGuid(WMProfile_XXX);	// RE: wmsysprf.h

//			NOTE: If you want video only capture you must modify the profile to remove the audio
//				  otherwise encoding will fail

			// Load a system profile and modify the resolution of the video output
			// NOTE: The scope of the encoding is enormous, this just demonstrates how to change
			// the output video resolution from 320x240 to something larger.
			// Changing the resolution affects coding performance, it is likely that the encoder will
			// start to drop frames after a while.  Using WM9 codecs will probably improve performance
			// and that has been left to the reader... ;o)
			//
			// get a profile manager
			CComPtr<IWMProfileManager> pIWMProfileManager = NULL;
			hr = WMCreateProfileManager(&pIWMProfileManager);
			if (SUCCEEDED(hr))
			{
				// load a system profile to modify
				CComPtr<IWMProfile> pIWMProfile = NULL;
				// NOTE: Any WMProfile_XXX could be used here, or create a custom profile from scratch
				hr = pIWMProfileManager->LoadProfileByID(WMProfile_V80_FAIRVBRVideo, &pIWMProfile);
				if (SUCCEEDED(hr))
				{
					// search the streams for the video stream and attempt to modify the video size
					DWORD cbStreams = 0;
					hr = pIWMProfile->GetStreamCount(&cbStreams);
					if (SUCCEEDED(hr))
					{
						IWMStreamConfig* pIWMStreamConfig = NULL;
						GUID streamType = {0};
						DWORD stream;
						
						if (m_bAudioMute)
						{
							// remove the audio stream for video only capture
							for (stream=0; stream<cbStreams; ++stream)
							{
								hr = pIWMProfile->GetStream(stream, &pIWMStreamConfig);
								if (SUCCEEDED(hr))
								{
									hr = pIWMStreamConfig->GetStreamType(&streamType);
									if (SUCCEEDED(hr))
									{
										if (MEDIATYPE_Audio == streamType)
										{
											if (SUCCEEDED(pIWMProfile->RemoveStream(pIWMStreamConfig)))
											{
												--cbStreams;
											}
											SAFE_RELEASE(pIWMStreamConfig);
											break;
										}
									}
								}
							}
						}

						for (stream=0; stream<cbStreams; ++stream)
						{
							hr = pIWMProfile->GetStream(stream, &pIWMStreamConfig);
							if (SUCCEEDED(hr))
							{
								hr = pIWMStreamConfig->GetStreamType(&streamType);
								if (SUCCEEDED(hr) && (MEDIATYPE_Video == streamType))
								{
									// found the video stream
									CComQIPtr<IWMMediaProps, &IID_IWMMediaProps> pIWMMediaProps = pIWMStreamConfig;
									if (pIWMMediaProps)
									{
										// get the size of the media type
										WM_MEDIA_TYPE* pMediaType = NULL;
										DWORD cbMediaType = 0;
										hr = pIWMMediaProps->GetMediaType(pMediaType, &cbMediaType);
										if (SUCCEEDED(hr))
										{
											pMediaType = (WM_MEDIA_TYPE*)new char [cbMediaType];
											if (pMediaType)
											{
												hr = pIWMMediaProps->GetMediaType(pMediaType, &cbMediaType);
												if (SUCCEEDED(hr))
												{
													BITMAPINFOHEADER* pbmih = NULL;
													if (WMFORMAT_VideoInfo == pMediaType->formattype)
													{
														WMVIDEOINFOHEADER* pvih = (WMVIDEOINFOHEADER*)pMediaType->pbFormat;
														pbmih = &pvih->bmiHeader;
													}
													else if (WMFORMAT_MPEG2Video == pMediaType->formattype)
													{
														
														WMVIDEOINFOHEADER2* pvih = (WMVIDEOINFOHEADER2*)&((WMMPEG2VIDEOINFO*)pMediaType->pbFormat)->hdr;
														pbmih = &pvih->bmiHeader;
													}
													
													if (pbmih)
													{
														// modify the video dimensions, set the property, reconfigure the stream
														// and then configure the ASF writer with this modified profile
														pbmih->biWidth = 640;	// was 320;
														pbmih->biHeight = 480;	// was 240;
														pbmih->biSizeImage = pbmih->biWidth * pbmih->biHeight * pbmih->biBitCount / 8;	// NOTE: This calculation is not correct for all bit depths
														hr = pIWMMediaProps->SetMediaType(pMediaType);
														if (SUCCEEDED(hr))
														{
															// config the ASF writer filter to use this modified system profile
															hr = pIWMProfile->ReconfigStream(pIWMStreamConfig);
															if (SUCCEEDED(hr))
															{
																hr = pICW->ConfigureFilterUsingProfile(pIWMProfile);
															}
														}
													}
												}
												delete [] (char*)pMediaType;
											}
										}
									}
								}
								SAFE_RELEASE(pIWMStreamConfig);
							}
						}
					}
				}
			}
/*
			// modify other ASF writer properties
			IServiceProvider* pProvider = NULL;
			hr = pASFWriter->QueryInterface(IID_IServiceProvider, reinterpret_cast<void**>(&pProvider));
			if (SUCCEEDED(hr))
			{
				IID_IWMWriterAdvanced2* pWMWA2 = NULL;
				hr = pProvider->QueryService(IID_IID_IWMWriterAdvanced2, IID_IID_IWMWriterAdvanced2, reinterpret_cast<void**>(&pWMWA2));
				if (SUCCEEDED(hr))
				{
					// set the deinterlace mode
					pWMWA2->GetInputSetting(...);
					:
					:
					SAFE_RELEASE(pWMWA2);
				}
				SAFE_RELEASE(pProvider);
			}
*/
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CreateMJCaptureGraph
// Create an optimum MJ capture graph
HRESULT CDecklinkCaptureDlg::CreateMJCaptureGraph()
{
	HRESULT hr = S_OK;

	// locate the DeckLink MJPEG encoder, AVI mux and file writer filters and add them to the graph
	CComPtr<IBaseFilter> pEncoder = NULL;
	hr = CDSUtils::AddFilter(m_pGraph, CLSID_DecklinkMJPEGEncoderFilter, L"Decklink MJPEG Compressor", &pEncoder);
	if (SUCCEEDED(hr))
	{
		CComPtr<IBaseFilter> pAVIMux = NULL;
		hr = CDSUtils::AddFilter(m_pGraph, CLSID_AviDest , L"AVI Mux", &pAVIMux);
		if (SUCCEEDED(hr))
		{
			CComPtr<IBaseFilter> pFileWriter = NULL;
			hr = CDSUtils::AddFilter(m_pGraph, CLSID_FileWriter, L"File writer", &pFileWriter);
			if (SUCCEEDED(hr))
			{
				// set the output filename
				CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pIFS = pFileWriter;
				if (pIFS)
				{
					USES_CONVERSION;	// for T2W macro
					hr = pIFS->SetFileName(T2W(m_captureFile), NULL);
					if (SUCCEEDED(hr))
					{
						// configure the MJPEG encoder
						CComQIPtr<IAMVideoCompression, &IID_IAMVideoCompression> pIVC = pEncoder;
						if (pIVC)
						{
							long Capabilities;
							hr = pIVC->GetInfo(0, 0, 0, 0, 0, 0, 0, &Capabilities);
							if (SUCCEEDED(hr) && (Capabilities & CompressionCaps_CanQuality))
							{
								hr = pIVC->put_Quality(0.8);
							}
						}

						if (SUCCEEDED(hr))
						{
							// connect the smart-T capture pin to the MJPEG Encoder
							hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, L"Capture", pEncoder, NULL);
							if (SUCCEEDED(hr))
							{
								// connect the MJPEG encoder output to the AVI mux
								hr = CDSUtils::ConnectFilters(m_pGraph, pEncoder, NULL, pAVIMux, NULL);
								if (SUCCEEDED(hr))
								{
									// connect the mux to the file writer
									hr = CDSUtils::ConnectFilters(m_pGraph, pAVIMux, NULL, pFileWriter, NULL);
									if (SUCCEEDED(hr))
									{
										// video path connected now optionally connect the audio path
										if (FALSE == m_bAudioMute)
										{
											// connect the Decklink audio capture pin to the mux
											hr = CDSUtils::ConnectFilters(m_pGraph, m_pAudioCapture, L"Capture", pAVIMux, NULL);
										}

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
	}

	return hr;
}

//-----------------------------------------------------------------------------
// DestroyGraph
// Remove all intermediate filters, keep any Decklink and video render filters as
// these are used by all the graphs.
HRESULT CDecklinkCaptureDlg::DestroyGraph()
{
	HRESULT hr = S_OK;

	if (m_pGraph && m_pControl)
	{
		m_pControl->Stop();

		// release our outstanding reference on this filter so it can be removed from the graph
		m_pSmartT = NULL;

		// retrieve the name of the capture device, don't remove it in this method
		PWSTR pNameVideoCapture = (PWSTR)m_videoDeviceCtrl.GetItemData(m_videoDeviceCtrl.GetCurSel());
		PWSTR pNameAudioCapture = (PWSTR)m_audioDeviceCtrl.GetItemData(m_audioDeviceCtrl.GetCurSel());

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
					
					if ((NULL == wcsstr(filterInfo.achName, pNameVideoCapture)) && (NULL == wcsstr(filterInfo.achName, pNameAudioCapture)) && (NULL == wcsstr(filterInfo.achName, L"Video Renderer")))
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
// PopulateDeviceControl
// Fill device combo box with available devices of the specified category
HRESULT CDecklinkCaptureDlg::PopulateDeviceControl(const GUID* pCategory, CComboBox* pCtrl)
{
	HRESULT hr = S_OK;
	if (pCategory && pCtrl)
	{
		// first enumerate the system devices for the specifed class and filter name
		CComPtr<ICreateDevEnum> pSysDevEnum = NULL;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pSysDevEnum));

		if (SUCCEEDED(hr))
		{
			CComPtr<IEnumMoniker> pEnumCat = NULL;
			hr = pSysDevEnum->CreateClassEnumerator(*pCategory, &pEnumCat, 0);

			if (S_OK == hr)
			{
				IMoniker* pMoniker = NULL;
				bool Loop = true;
				while ((S_OK == pEnumCat->Next(1, &pMoniker, NULL)) && Loop)
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
							pCtrl->SetItemData(pCtrl->AddString(buf), (DWORD)pName);
						}

						VariantClear(&varName);
						
						// contained within a loop, decrement the reference count
						SAFE_RELEASE(pPropBag);
					}
					SAFE_RELEASE(pMoniker);
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
// PopulateVideoControl
// Fill video format combo box with supported video formats using the IAMStreamConfig
// interface.
HRESULT CDecklinkCaptureDlg::PopulateVideoControl()
{
	HRESULT hr = S_OK;

	if (m_pVideoCapture)
	{
		// free mediatypes attached to format controls
		int count = m_videoFormatCtrl.GetCount();
		if (count)
		{
			for (int item=0; item<count; ++item)
			{
				DeleteMediaType((AM_MEDIA_TYPE*)m_videoFormatCtrl.GetItemData(item));
			}
			m_videoFormatCtrl.ResetContent();
		}

		// locate the video capture pin and QI for stream control
		CComPtr<IAMStreamConfig> pISC = NULL;
		hr = CDSUtils::FindPinInterface(m_pVideoCapture, &MEDIATYPE_Video, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
	    if (SUCCEEDED(hr))
	    {
			// loop through all the capabilities (video formats) and populate the control
			int count, size;
			hr = pISC->GetNumberOfCapabilities(&count, &size);
			if (SUCCEEDED(hr))
			{
				if (sizeof(VIDEO_STREAM_CONFIG_CAPS) == size)
				{
					AM_MEDIA_TYPE* pmt = NULL;
					VIDEO_STREAM_CONFIG_CAPS vscc;
					VIDEOINFOHEADER* pvih = NULL;

					for (int index=0; index<count; ++index)
					{
						hr = pISC->GetStreamCaps(index, &pmt, reinterpret_cast<BYTE*>(&vscc));
						if (SUCCEEDED(hr))
						{
							TCHAR		buffer[128];
							float		frameRate;
							char*		pixelFormatString;
							
							ZeroMemory(buffer, sizeof(buffer));

							pvih = (VIDEOINFOHEADER*)pmt->pbFormat;
							//
							if (pvih->bmiHeader.biBitCount == 16)
								pixelFormatString = TEXT("8 bit 4:2:2 YUV");
							else if (pvih->bmiHeader.biBitCount == 20)
								pixelFormatString = TEXT("10 bit 4:2:2 YUV");
							else if (pvih->bmiHeader.biBitCount == 30)
								pixelFormatString = TEXT("10 bit 4:4:4 RGB");
							else
								pixelFormatString = TEXT("");			// Unknown pixel format
							
							// provide a useful description of the formats
							if (486 == pvih->bmiHeader.biHeight)
							{
								if (417083 == pvih->AvgTimePerFrame)
								{
									StringCbPrintf(buffer, sizeof(buffer), TEXT("NTSC - %s (3:2 pulldown removal)"), pixelFormatString);
								}
								else
								{
									StringCbPrintf(buffer, sizeof(buffer), TEXT("NTSC - %s"), pixelFormatString);
								}
							}
							else if (576 == pvih->bmiHeader.biHeight)
							{
								StringCbPrintf(buffer, sizeof(buffer), TEXT("PAL - %s"), pixelFormatString);
							}
							else
							{
								frameRate = (float)UNITS / pvih->AvgTimePerFrame;

								if (720 == pvih->bmiHeader.biHeight)
								{
									// 720p
									if ((frameRate - (int)frameRate) > 0.01)
									{
										StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 720p %.2f - %s"), frameRate, pixelFormatString);
									}
									else
									{
										StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 720p %.0f - %s"), frameRate, pixelFormatString);
									}
								}
								else if (1080 == pvih->bmiHeader.biHeight)
								{
									if ((frameRate < 25.00) || (frameRate >= 50.0))		// 1080p23, 1080p24, 1080p50, 1080p5994, 1080p60
									{
										// Progressive 1080
										if ((frameRate - (int)frameRate) > 0.01)
										{
											StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 1080p %.2f - %s"), frameRate, pixelFormatString);
										}
										else
										{
											StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 1080p %.0f - %s"), frameRate, pixelFormatString);
										}
									}
									else
									{
										// Interlaced 1080
										if ((frameRate - (int)frameRate) > 0.01)
										{
											StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 1080i %.2f - %s"), frameRate*2.0f, pixelFormatString);
										}
										else
										{
											StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 1080i %.0f - %s"), frameRate*2.0f, pixelFormatString);
										}
									}
								}
								else if (1556 == pvih->bmiHeader.biHeight)
								{
									if ((frameRate - (int)frameRate) > 0.01)
									{
										StringCbPrintf(buffer, sizeof(buffer), TEXT("2K 1556p %.2f - %s"), frameRate, pixelFormatString);
									}
									else
									{
										StringCbPrintf(buffer, sizeof(buffer), TEXT("2K 1556p %.0f - %s"), frameRate, pixelFormatString);
									}
								}
							}
							
							// If the display mode was recognized, add it to the listbox UI
							if (buffer[0] != 0)
							{
								// add the item description to combo box
								int n = m_videoFormatCtrl.AddString(buffer);
								// store media type pointer in item's data section
								m_videoFormatCtrl.SetItemData(n, (DWORD_PTR)pmt);
								
								// set default format
								if ((pvih->AvgTimePerFrame == m_vihDefault.AvgTimePerFrame) &&
									(pvih->bmiHeader.biWidth == m_vihDefault.bmiHeader.biWidth) &&
									(pvih->bmiHeader.biHeight == m_vihDefault.bmiHeader.biHeight) &&
									(pvih->bmiHeader.biBitCount == m_vihDefault.bmiHeader.biBitCount))
								{
									m_videoFormatCtrl.SetCurSel(n);
									pISC->SetFormat(pmt);
								}
							}
							else
							{
								DeleteMediaType(pmt);
							}
						}
					}
				}
				else
				{
					m_videoFormatCtrl.AddString(TEXT("ERROR: Unable to retrieve video formats"));
				}
			}
		}

		// as the device is being changed, update the IDecklinkInputStatus interface
		{
			CAutoLock lock(&m_csInputStatusLock);	// prevent thread from using this interface while it is changed
		
			m_pIDecklinkStatus = m_pVideoCapture;
			if (m_pIDecklinkStatus)
			{
				m_pIDecklinkStatus->RegisterVideoStatusChangeEvent((unsigned long)m_hInputStatusChangeEvent);	// register the event 
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
// PopulateAudioControl
// Fill audio format combo box with supported audio formats using the IAMStreamConfig
// interface.
HRESULT CDecklinkCaptureDlg::PopulateAudioControl()
{
	HRESULT hr = S_OK;

	if (m_pAudioCapture)
	{
		// free mediatypes attached to format controls
		int count = m_audioFormatCtrl.GetCount();
		if (count)
		{
			for (int item=0; item<count; ++item)
			{
				DeleteMediaType((AM_MEDIA_TYPE*)m_audioFormatCtrl.GetItemData(item));
			}
			m_audioFormatCtrl.ResetContent();
		}

		// locate the audio capture pin and QI for stream control
		CComPtr<IAMStreamConfig> pISC = NULL;
		hr = CDSUtils::FindPinInterface(m_pAudioCapture, &MEDIATYPE_Audio, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
	    if (SUCCEEDED(hr))
	    {
			// loop through all the capabilities (audio formats) and populate the control
			int count, size;
			hr = pISC->GetNumberOfCapabilities(&count, &size);
			if (SUCCEEDED(hr))
			{
				if (sizeof(AUDIO_STREAM_CONFIG_CAPS) == size)
				{
					AM_MEDIA_TYPE* pmt = NULL;
					AUDIO_STREAM_CONFIG_CAPS ascc;
					WAVEFORMATEX* pwfex = NULL;

					for (int index=0; index<count; ++index)
					{
						hr = pISC->GetStreamCaps(index, &pmt, reinterpret_cast<BYTE*>(&ascc));
						if (SUCCEEDED(hr))
						{
							TCHAR buffer[32];
							
							ZeroMemory(buffer, sizeof(buffer));

							pwfex = (WAVEFORMATEX*)pmt->pbFormat;

							// provide a useful description of the formats
							if (1 == pwfex->nChannels)
							{
								StringCbPrintf(buffer, sizeof(buffer), TEXT("%d channel, %2.1fkHz, %d-bit"), (int)pwfex->nChannels, (float)pwfex->nSamplesPerSec / 1000, (int)pwfex->wBitsPerSample);
							}
							else
							{
								StringCbPrintf(buffer, sizeof(buffer), TEXT("%d channels, %2.1fkHz, %d-bit"), (int)pwfex->nChannels, (float)pwfex->nSamplesPerSec / 1000, (int)pwfex->wBitsPerSample);
							}

							// add the item description to combo box
							int n = m_audioFormatCtrl.AddString(buffer);
							// store media type pointer in item's data section
							m_audioFormatCtrl.SetItemData(n, (DWORD_PTR)pmt);

							// set default format
							if ((pwfex->wFormatTag == m_wfexDefault.wFormatTag) &&
								(pwfex->nChannels == m_wfexDefault.nChannels) &&
								(pwfex->nSamplesPerSec == m_wfexDefault.nSamplesPerSec) &&
								(pwfex->nAvgBytesPerSec == m_wfexDefault.nAvgBytesPerSec))
							{
								m_audioFormatCtrl.SetCurSel(n);
								pISC->SetFormat(pmt);
							}
						}
					}
				}
				else
				{
					m_audioFormatCtrl.AddString(TEXT("ERROR: Unable to retrieve audio formats"));
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
// PopulateCompressionControl
// Fill compression control with a selection of video compressors, locate the
// encoders and add them to the combo box if they exist.
HRESULT CDecklinkCaptureDlg::PopulateCompressionControl()
{
	int n = m_compressionCtrl.AddString(TEXT("Uncompressed"));
	m_compressionCtrl.SetItemData(n, (DWORD_PTR)ENC_NONE);

	// search for the DV encoder, WM encoder and MJPEG encoder
	IBaseFilter* pFilter = NULL;
	HRESULT hr = CoCreateInstance(CLSID_DVVideoEnc, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pFilter));
	if (SUCCEEDED(hr))
	{
		n = m_compressionCtrl.SetCurSel(m_compressionCtrl.AddString(TEXT("DV Video Encoder")));
		m_compressionCtrl.SetItemData(n, (DWORD_PTR)ENC_DV);
		SAFE_RELEASE(pFilter);
	}

	hr = CoCreateInstance(CLSID_WMAsfWriter, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pFilter));
	if (SUCCEEDED(hr))
	{
		n = m_compressionCtrl.SetCurSel(m_compressionCtrl.AddString(TEXT("Windows Media Encoder")));
		m_compressionCtrl.SetItemData(n, (DWORD_PTR)ENC_WM);
		SAFE_RELEASE(pFilter);
	}

	hr = CoCreateInstance(CLSID_DecklinkMJPEGEncoderFilter, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pFilter));
	if (SUCCEEDED(hr))
	{
		n = m_compressionCtrl.SetCurSel(m_compressionCtrl.AddString(TEXT("Decklink MJPEG Encoder")));
		m_compressionCtrl.SetItemData(n, (DWORD_PTR)ENC_MJ);
		SAFE_RELEASE(pFilter);
	}

	m_compressionCtrl.SetCurSel(m_compressor);

	return S_OK;
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboVideodevice
// Rebuild graph with selected capture device
void CDecklinkCaptureDlg::OnCbnSelchangeComboVideodevice()
{
	m_pVideoCapture = NULL;	// release our outstanding reference
	// remove intermediate filters, since the device selection has changed the capture device will also be removed
	HRESULT hr = DestroyGraph();
	if (SUCCEEDED(hr))
	{
		// rebuild graph with new capture device selection
		PWSTR pName = (PWSTR)m_videoDeviceCtrl.GetItemData(m_videoDeviceCtrl.GetCurSel());
		if (pName)
		{
			hr = CDSUtils::AddFilter2(m_pGraph, CLSID_VideoInputDeviceCategory, pName, &m_pVideoCapture);
			if (SUCCEEDED(hr))
			{
				// as the device has changed get the current operating format so that the control
				// and display this as the current selection
				CComPtr<IAMStreamConfig> pISC = NULL;
				hr = CDSUtils::FindPinInterface(m_pVideoCapture, &MEDIATYPE_Video, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
				if (SUCCEEDED(hr))
				{
					// get the current format of the device to set the current selection of the control
					AM_MEDIA_TYPE* pamt = NULL;
					hr = pISC->GetFormat(&pamt);
					if (SUCCEEDED(hr))
					{
						if (pamt->pbFormat)
						{
							m_vihDefault = *(VIDEOINFOHEADER*)pamt->pbFormat;
						}
						DeleteMediaType(pamt);
					}
				}

				hr = PopulateVideoControl();	// repopulate the control with formats from the new device
				if (SUCCEEDED(hr))
				{
					hr = CreatePreviewGraph();	// rebuild the graph with the new device
				}
			}
		}
		else
		{
			hr = E_POINTER;
		}
	}
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboAudiodevice
// Rebuild graph with selected capture device
void CDecklinkCaptureDlg::OnCbnSelchangeComboAudiodevice()
{
	m_pAudioCapture = NULL;	// release our outstanding reference
	// remove intermediate filters, since the device selection has changed the capture device will also be removed
	HRESULT hr = DestroyGraph();
	if (SUCCEEDED(hr))
	{
		PWSTR pName = (PWSTR)m_audioDeviceCtrl.GetItemData(m_audioDeviceCtrl.GetCurSel());
		if (pName)
		{
			hr = CDSUtils::AddFilter2(m_pGraph, CLSID_AudioInputDeviceCategory, pName, &m_pAudioCapture);
			if (SUCCEEDED(hr))
			{
				// as the device has changed get the current operating format so that the control
				// and display this as the current selection
				CComPtr<IAMStreamConfig> pISC = NULL;
				hr = CDSUtils::FindPinInterface(m_pAudioCapture, &MEDIATYPE_Audio, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
				if (SUCCEEDED(hr))
				{
					// get the current format of the device to set the current selection of the control
					AM_MEDIA_TYPE* pamt = NULL;
					hr = pISC->GetFormat(&pamt);
					if (SUCCEEDED(hr))
					{
						if (pamt->pbFormat)
						{
							m_wfexDefault = *(WAVEFORMATEX*)pamt->pbFormat;
						}
						DeleteMediaType(pamt);
					}
				}

				hr = PopulateAudioControl();	// repopulate the control with formats from the new device
				if (SUCCEEDED(hr))
				{
					hr = CreatePreviewGraph();	// rebuild the graph with the new device
				}
			}
		}
		else
		{
			hr = E_POINTER;
		}
	}
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboVideoformats
// Rebuild preview graph if format selection changed
void CDecklinkCaptureDlg::OnCbnSelchangeComboVideoformats()
{
	HRESULT hr = DestroyGraph();

	if (SUCCEEDED(hr))
	{
		// locate the video capture pin and QI for stream control
		CComPtr<IAMStreamConfig> pISC = NULL;
		hr = CDSUtils::FindPinInterface(m_pVideoCapture, &MEDIATYPE_Video, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
	    if (SUCCEEDED(hr))
	    {
			// set the new media format
			AM_MEDIA_TYPE* pmt = (AM_MEDIA_TYPE*)m_videoFormatCtrl.GetItemData(m_videoFormatCtrl.GetCurSel());
			m_vihDefault = *(VIDEOINFOHEADER*)pmt->pbFormat;
			ASSERT(sizeof(VIDEOINFOHEADER) <= pmt->cbFormat);
			hr = pISC->SetFormat(pmt);
			if (SUCCEEDED(hr))
			{
				// save the new format
				EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("VideoFormat"), reinterpret_cast<const BYTE*>(&m_vihDefault), sizeof(m_vihDefault)));

				// update compression control, we don't have an HD compression filter so disable compression for HD formats
				if (576 < m_vihDefault.bmiHeader.biHeight)
				{
					m_compressor = 0;
					m_compressionCtrl.SetCurSel(m_compressor);
					// save the new state
					EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("VideoCompressor"), reinterpret_cast<const BYTE*>(&m_compressor), sizeof(m_compressor)));
					m_bEnableCompressionCtrl = FALSE;
				}
				else
				{
					m_bEnableCompressionCtrl = TRUE;
				}
				EnableControls();

				// rebuild the graph
				hr = CreatePreviewGraph();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboAudioformats
// Rebuild preview graph if format selection changed
void CDecklinkCaptureDlg::OnCbnSelchangeComboAudioformats()
{
	HRESULT hr = DestroyGraph();
	if (SUCCEEDED(hr))
	{
		// locate the audio capture pin and QI for stream control
		CComPtr<IAMStreamConfig> pISC = NULL;
		hr = CDSUtils::FindPinInterface(m_pAudioCapture, &MEDIATYPE_Audio, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
	    if (SUCCEEDED(hr))
	    {
			// set the new media format
			AM_MEDIA_TYPE* pmt = (AM_MEDIA_TYPE*)m_audioFormatCtrl.GetItemData(m_audioFormatCtrl.GetCurSel());
			m_wfexDefault = *(WAVEFORMATEX*)pmt->pbFormat;
			ASSERT(sizeof(WAVEFORMATEX) == pmt->cbFormat);
			hr = pISC->SetFormat(pmt);
			if (SUCCEEDED(hr))
			{
				// save the new format
				EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("AudioFormat"), reinterpret_cast<const BYTE*>(&m_wfexDefault), sizeof(m_wfexDefault)));

				// rebuild the graph
				hr = CreatePreviewGraph();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboCompression
// Rebuild preview graph if compression selection changed
void CDecklinkCaptureDlg::OnCbnSelchangeComboCompression()
{
	HRESULT hr = DestroyGraph();
	if (SUCCEEDED(hr))
	{
		// save the new state
		m_compressor = m_compressionCtrl.GetCurSel();
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("VideoCompressor"), reinterpret_cast<const BYTE*>(&m_compressor), sizeof(m_compressor)));
		// rebuild the graph
		hr = CreatePreviewGraph();
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedCheckAudiomute
// Rebuild the capture graph to reflect the new audio setting
void CDecklinkCaptureDlg::OnBnClickedCheckAudiomute()
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_AUDIOMUTE);
	if (pCheck)
	{
		m_bAudioMute = pCheck->GetState() & 0x0003;

		HRESULT hr = DestroyGraph();
		if (SUCCEEDED(hr))
		{
			// save the new state
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("AudioMute"), reinterpret_cast<const BYTE*>(&m_bAudioMute), sizeof(m_bAudioMute)));

			// rebuild the graph which reflects the new audio setting
			hr = CreatePreviewGraph();
		}
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonBrowse
// Create a file open dialog to browse for a file location
void CDecklinkCaptureDlg::OnBnClickedButtonBrowse()
{
	char BASED_CODE szFilters[] = "Windows Media Files|*.avi;*.asf;*.wmv|All Files (*.*)|*.*||";

	TCHAR* pExt[] = {"*.avi", "*.avi", "*.asf;*.wmv"};

	CFileDialog FileDlg(TRUE, "Windows Media Files", pExt[m_compressor], 0, szFilters, this);

	if (FileDlg.DoModal() == IDOK)
	{
		m_captureFile = FileDlg.GetPathName();
		m_captureFileCtrl.SetWindowText(m_captureFile);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonCapture
// Create a capture graph a start capture
void CDecklinkCaptureDlg::OnBnClickedButtonCapture()
{
	HRESULT hr = CreateCaptureGraph();
	if (SUCCEEDED(hr))
	{
		if (m_pControl)
		{
			hr = m_pControl->Run();
			if (SUCCEEDED(hr))
			{
				DisableControls();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonStop
// Stop capture and revert to preview
void CDecklinkCaptureDlg::OnBnClickedButtonStop()
{
	HRESULT hr = DestroyGraph();
	if (SUCCEEDED(hr))
	{
		hr = CreatePreviewGraph();
		if (SUCCEEDED(hr))
		{
			EnableControls();
		}
	}
}

//-----------------------------------------------------------------------------
// EnableControls
//
void CDecklinkCaptureDlg::EnableControls(void)
{
	CWnd* pWnd = GetDlgItem(IDC_COMBO_VIDEOFORMATS);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_COMBO_AUDIOFORMATS);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_CHECK_AUDIOMUTE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_COMBO_COMPRESSION);
	m_bEnableCompressionCtrl = (576 < m_vihDefault.bmiHeader.biHeight) ? FALSE : TRUE;	// don't have an HDV codec do disable compression control for HD formats
	pWnd->EnableWindow(m_bEnableCompressionCtrl);
	pWnd = GetDlgItem(IDC_EDIT_CAPTUREFILE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_BROWSE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_CAPTURE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_STOP);
	pWnd->EnableWindow(FALSE);
}

//-----------------------------------------------------------------------------
// DisableControls
//
void CDecklinkCaptureDlg::DisableControls(void)
{
	CWnd* pWnd = GetDlgItem(IDC_COMBO_VIDEOFORMATS);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_COMBO_AUDIOFORMATS);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_CHECK_AUDIOMUTE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_COMBO_COMPRESSION);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_EDIT_CAPTUREFILE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_BROWSE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_CAPTURE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_STOP);
	pWnd->EnableWindow(TRUE);
}

//-----------------------------------------------------------------------------
// QueryRegistry
// retrieve previous media formats from registry
void CDecklinkCaptureDlg::QueryRegistry(void)
{
	if (ERROR_SUCCESS == m_regUtils.Open(TEXT("DecklinkCaptureSample")))
	{
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetBinary(TEXT("VideoFormat"), reinterpret_cast<LPBYTE>(&m_vihDefault), sizeof(m_vihDefault)));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetBinary(TEXT("AudioFormat"), reinterpret_cast<LPBYTE>(&m_wfexDefault), sizeof(m_wfexDefault)));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetBinary(TEXT("AudioMute"), reinterpret_cast<LPBYTE>(&m_bAudioMute), sizeof(m_bAudioMute)));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetBinary(TEXT("VideoCompressor"), reinterpret_cast<LPBYTE>(&m_compressor), sizeof(m_compressor)));

		WCHAR captureFile[MAX_PATH];
		ZeroMemory(captureFile, sizeof(captureFile));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.GetString(TEXT("CaptureFile"), reinterpret_cast<LPBYTE>(captureFile), sizeof(captureFile)));
		m_captureFile = captureFile;
	}
	else
	{
		// create the key and registry values
		if (ERROR_SUCCESS == m_regUtils.Create(TEXT("DecklinkCaptureSample")))
		{
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("VideoFormat"), reinterpret_cast<const BYTE*>(&m_vihDefault), sizeof(m_vihDefault)));
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("AudioFormat"), reinterpret_cast<const BYTE*>(&m_wfexDefault), sizeof(m_wfexDefault)));
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("AudioMute"), reinterpret_cast<const BYTE*>(&m_bAudioMute), sizeof(m_bAudioMute)));
			EXECUTE_ASSERT(ERROR_SUCCESS == m_regUtils.SetBinary(TEXT("VideoCompressor"), reinterpret_cast<const BYTE*>(&m_compressor), sizeof(m_compressor)));
		}
	}

	// update mute audio check box control
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_AUDIOMUTE);
	pButton->SetCheck(m_bAudioMute);
}

//-----------------------------------------------------------------------------
// InputStatusThreadWrapper
// Wrapper for the class thread.
DWORD WINAPI CDecklinkCaptureDlg::InputStatusThreadWrapper(LPVOID lpParam)
{
	DWORD ret = 0;

	if (lpParam)
	{
		CDecklinkCaptureDlg* pDlg = reinterpret_cast<CDecklinkCaptureDlg*>(lpParam);
		ret = pDlg->InputStatusThread();
	}
	else
	{
		ret = 1;
	}
	
	return ret;
}

//-----------------------------------------------------------------------------
// InputStatusThreadWrapper
// Wrapper for the class thread.
DWORD CDecklinkCaptureDlg::InputStatusThread(void)
{
	bool bRunning = true;
	
	while (bRunning)
	{
		switch (WaitForSingleObject(m_hStopInputStatusThreadEvent, 10))
		{
			case WAIT_TIMEOUT:
				{
					CAutoLock lock(&m_csInputStatusLock);	// prevent the app from changing the interface
					if (m_pIDecklinkStatus)
					{
						if (WAIT_OBJECT_0 == WaitForSingleObject(m_hInputStatusChangeEvent, 0))
						{
							int		videoStatus, genlockStatus;
							
							ResetEvent(m_hInputStatusChangeEvent);
							if (SUCCEEDED(m_pIDecklinkStatus->GetVideoInputStatus(&videoStatus, &genlockStatus)))
							{
								CStatic* pStatic = reinterpret_cast<CStatic*>(GetDlgItem(IDC_STATIC_INPUTSTATUS));
								if (pStatic)
								{
									pStatic->SetWindowText((DECKLINK_INPUT_PRESENT == videoStatus) ? TEXT("OK") : TEXT("**"));
								}

								pStatic = reinterpret_cast<CStatic*>(GetDlgItem(IDC_STATIC_GENLOCKSTATUS));
								if (pStatic)
								{
									TCHAR* pGenlockText[] = {TEXT("Not supported"), TEXT("Not connnected"), TEXT("Locked"), TEXT("Not locked")};
									pStatic->SetWindowText(pGenlockText[genlockStatus]);
								}
							}
						}
					}
				}
				break;
				
			default:
				bRunning = false;
				break;
		}
	}

	return 0;
}
