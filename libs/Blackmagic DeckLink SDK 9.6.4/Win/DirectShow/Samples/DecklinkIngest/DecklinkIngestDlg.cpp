//-----------------------------------------------------------------------------
// DecklinkIngestDlg.cpp
//
// Desc: DirectShow ingest sample
//
// Copyright (c) Blackmagic Design 2007. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DecklinkIngest.h"
#include "DecklinkIngestDlg.h"
#include "PreferencesDlg.h"
#include "DeviceInfoDlg.h"
#include "Timecode.h"
#include "StaticURL.h"
#include <xprtdefs.h>	// device control
#include "CMX3600EDLReader.h"

#include "DecklinkFilters_h.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
	CStaticURL m_LinkGDCL;

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
	m_LinkGDCL.SubclassDlgItem(IDC_STATIC_URLGDCL, this);
	return FALSE;
}

//-----------------------------------------------------------------------------
// CDecklinkIngestDlg dialog
//-----------------------------------------------------------------------------
const int CDecklinkIngestDlg::MAXDEVICES = 256;

static LPCWSTR SMARTT_NAME = L"Smart Tee";
static LPCWSTR VIDEORENDERER_NAME = L"Video Renderer";
static LPCWSTR SAMPLEGRABBERSTILLS_NAME = L"Sample Grabber Stills";
static LPCWSTR SAMPLEGRABBERTIMECODE_NAME = L"Sample Grabber Timecode";
static LPCWSTR NULLRENDERER_NAME = L"Null Renderer";

static const int LogClipCtrlIDs[] = {IDC_BUTTON_REMOVECLIP, IDC_BUTTON_MOVECLIPUP, IDC_BUTTON_MOVECLIPDOWN};
static const int CaptureCtrlIDs[] = {IDC_BUTTON_CAPTURENOW, IDC_BUTTON_BATCHCAPTURE, IDC_BUTTON_CAPTURESTILL};
static const int TransportCtrlIDs[] = {IDC_BUTTON_FFWD, IDC_BUTTON_FREW, IDC_BUTTON_PLAY, IDC_BUTTON_STOP, IDC_BUTTON_EJECT, IDC_BUTTON_BATCHCAPTURE };

//-----------------------------------------------------------------------------
// Constructor
//
CDecklinkIngestDlg::CDecklinkIngestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecklinkIngestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-----------------------------------------------------------------------------
// DoDataExchange
//
void CDecklinkIngestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_LOGCLIP, m_BtnAddClip);
	DDX_Control(pDX, IDC_BUTTON_REMOVECLIP, m_BtnRemoveClip);
	DDX_Control(pDX, IDC_BUTTON_MOVECLIPUP, m_BtnMoveClipUp);
	DDX_Control(pDX, IDC_BUTTON_MOVECLIPDOWN, m_BtnMoveClipDown);
	DDX_Control(pDX, IDC_LIST_CLIPS, m_ClipList);
	DDX_Control(pDX, IDC_STATIC_TIMECODE, m_TimecodeCtrl);
	DDX_Control(pDX, IDC_EDIT_INPOINT, m_InpointCtrl);
	DDX_Control(pDX, IDC_EDIT_OUTPOINT, m_OutpointCtrl);
	DDX_Control(pDX, IDC_STATIC_DURATION, m_DurationCtrl);
	DDX_Control(pDX, IDC_BUTTON_FREW, m_BtnFRew);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_BtnPlay);
	DDX_Control(pDX, IDC_BUTTON_FFWD, m_BtnFFwd);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_BtnStop);
	DDX_Control(pDX, IDC_BUTTON_EJECT, m_BtnEjct);
	DDX_Control(pDX, IDC_EDIT_TAPENAME, m_TapenameCtrl);
	DDX_Control(pDX, IDC_EDIT_CLIPNAME, m_ClipnameCtrl);
}

//-----------------------------------------------------------------------------
// Message map
//
BEGIN_MESSAGE_MAP(CDecklinkIngestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_IMPORTEDL, OnFileImportEDL)
	ON_COMMAND(ID_EDIT_PREFERENCES, OnEditPreferences)
	ON_COMMAND_RANGE(IDC_DEVICE_BASE, IDC_DEVICE_BASE + CDecklinkIngestDlg::MAXDEVICES - 1, OnDeviceMenu)
	ON_COMMAND(IDC_DEVICE_INFO, OnDeviceInfo)
	ON_COMMAND(IDM_ABOUTBOX, OnAboutBox)
	ON_WM_INITMENUPOPUP()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_LOGCLIP, OnBnClickedButtonLogclip)
	ON_BN_CLICKED(IDC_BUTTON_REMOVECLIP, OnBnClickedButtonRemoveclip)
	ON_BN_CLICKED(IDC_BUTTON_MOVECLIPUP, OnBnClickedButtonMoveclipup)
	ON_BN_CLICKED(IDC_BUTTON_MOVECLIPDOWN, OnBnClickedButtonMoveclipdown)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURESTILL, OnBnClickedButtonCaptureStill)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURENOW, OnBnClickedButtonCaptureNow)
	ON_BN_CLICKED(IDC_BUTTON_BATCHCAPTURE, OnBnClickedButtonBatchCapture)
	ON_BN_CLICKED(IDC_BUTTON_ABORT, OnBnClickedButtonAbort)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_FREW, OnBnClickedButtonFrew)
	ON_BN_CLICKED(IDC_BUTTON_FFWD, OnBnClickedButtonFfwd)
	ON_BN_CLICKED(IDC_BUTTON_EJECT, OnBnClickedButtonEject)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// OnInitDialog
//
BOOL CDecklinkIngestDlg::OnInitDialog()
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
	// Create the sample grabber callback handlers.
	m_pStillGrabber = new CStillGrabber;
	m_pTimecodeGrabber = new CTimecodeGrabber;

	m_pDeviceNameVideo = NULL;

	CreateStatusBarControl();

	m_hThread = NULL;
	m_hExitEvent = NULL;

	// Set the defaults capture location and filename.
	TCHAR szPath[MAX_PATH] = {0};
	if (S_OK == SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szPath))
	{
		if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(szPath))
		{
			PathAppend(szPath, TEXT("My Videos"));
			m_CaptureFilePath = szPath;
		}
	}
	m_CaptureFilename = TEXT("DecklinkIngest");
	m_CaptureTapename = TEXT("Untitled Tape");

	// Set the default video capture format.
	m_VideoFormat.InitMediaType();
	m_VideoFormat.SetType(&MEDIATYPE_Video);
	m_VideoFormat.SetSubtype(&MEDIASUBTYPE_UYVY);
	m_VideoFormat.SetFormatType(&FORMAT_VideoInfo);
	VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)m_VideoFormat.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
	if (pvih)
	{
		ZeroMemory(pvih, sizeof(VIDEOINFOHEADER));
		pvih->AvgTimePerFrame = 333667;
		pvih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pvih->bmiHeader.biWidth = 720;
		pvih->bmiHeader.biHeight = 486;
		pvih->bmiHeader.biPlanes = 1;
		pvih->bmiHeader.biBitCount = 16;
		pvih->bmiHeader.biCompression = 'YVYU';
		pvih->bmiHeader.biSizeImage = pvih->bmiHeader.biWidth * pvih->bmiHeader.biHeight * pvih->bmiHeader.biBitCount / 8;
		m_VideoFormat.SetSampleSize(pvih->bmiHeader.biSizeImage);
		pvih->dwBitRate = pvih->bmiHeader.biSizeImage * 8 * 30;
	}

	// Set the default audio capture format.
	m_AudioFormat.InitMediaType();
	m_AudioFormat.SetType(&MEDIATYPE_Audio);
	m_AudioFormat.SetSubtype(&MEDIASUBTYPE_PCM);
	m_AudioFormat.SetFormatType(&FORMAT_WaveFormatEx);
	WAVEFORMATEX* pwfex = (WAVEFORMATEX*)m_AudioFormat.AllocFormatBuffer(sizeof(WAVEFORMATEX));
	if (pwfex)
	{
		ZeroMemory(pwfex, sizeof(WAVEFORMATEX));
		pwfex->wFormatTag = WAVE_FORMAT_PCM;
		pwfex->nChannels = 2;
		pwfex->nSamplesPerSec = 48000;
		pwfex->wBitsPerSample = 16;
		pwfex->nBlockAlign = pwfex->wBitsPerSample * pwfex->nChannels / 8;
		pwfex->nAvgBytesPerSec = pwfex->nSamplesPerSec * pwfex->nBlockAlign;
	}

	m_bMuteAudio = false;
	m_Compression = ENC_NONE;
	m_Device = 0;
	
	ResetFileIndex();

	// load bitmaps for file buttons
	m_BtnAddClip.LoadBitmaps(IDB_BITMAP_ADDFILEU, IDB_BITMAP_ADDFILED, IDB_BITMAP_ADDFILEF, IDB_BITMAP_ADDFILEX);
	m_BtnRemoveClip.LoadBitmaps(IDB_BITMAP_REMOVEFILEU, IDB_BITMAP_REMOVEFILED, IDB_BITMAP_REMOVEFILEF, IDB_BITMAP_REMOVEFILEX);
	m_BtnMoveClipUp.LoadBitmaps(IDB_BITMAP_MOVEUPFILEU, IDB_BITMAP_MOVEUPFILED, IDB_BITMAP_MOVEUPFILEF, IDB_BITMAP_MOVEUPFILEX);
	m_BtnMoveClipDown.LoadBitmaps(IDB_BITMAP_MOVEDOWNFILEU, IDB_BITMAP_MOVEDOWNFILED, IDB_BITMAP_MOVEDOWNFILEF, IDB_BITMAP_MOVEDOWNFILEX);

	// set the columns in the list control
	int ColumnWidth = m_ClipList.GetStringWidth(TEXT("00:00:00:00")) + 12;
	m_ClipList.InsertColumn(0, TEXT("Clip name"), LVCFMT_CENTER, 150);
	m_ClipList.InsertColumn(1, TEXT("In-point"), LVCFMT_CENTER, ColumnWidth);
	m_ClipList.InsertColumn(2, TEXT("Out-point"), LVCFMT_CENTER, ColumnWidth);
	m_ClipList.InsertColumn(3, TEXT("Duration"), LVCFMT_CENTER, ColumnWidth);
	m_ClipList.InsertColumn(4, TEXT("Tape name"), LVCFMT_CENTER, 150);

	m_InpointCtrl.SetWindowText(TEXT("00:00:00:00"));
	m_OutpointCtrl.SetWindowText(TEXT("00:00:00:00"));
	m_TapenameCtrl.SetWindowText(TEXT("Untitled Tape"));
	m_ClipnameCtrl.SetWindowText(TEXT("Untitled Clip"));

	// load bitmaps for the deck controls
	m_BtnFRew.LoadBitmaps(IDB_BITMAP_FREWU, IDB_BITMAP_FREWD, IDB_BITMAP_FREWF, IDB_BITMAP_FREWX);
	m_BtnPlay.LoadBitmaps(IDB_BITMAP_PLAYU, IDB_BITMAP_PLAYD, IDB_BITMAP_PLAYF, IDB_BITMAP_PLAYX);
	m_BtnFFwd.LoadBitmaps(IDB_BITMAP_FFWDU, IDB_BITMAP_FFWDD, IDB_BITMAP_FFWDF, IDB_BITMAP_FFWDX);
	m_BtnStop.LoadBitmaps(IDB_BITMAP_STOPU, IDB_BITMAP_STOPD, IDB_BITMAP_STOPF, IDB_BITMAP_STOPX);
	m_BtnEjct.LoadBitmaps(IDB_BITMAP_EJECTU, IDB_BITMAP_EJECTD, IDB_BITMAP_EJECTF, IDB_BITMAP_EJECTX);

	m_bBatchCapture = false;

	QueryRegistry();	// Retrieve the previous state.

	SetFileExtension(m_CaptureFilename);

	CreateDeviceMenu();	// Discover system capture devices and create appropriate menu items.
#ifdef _DEBUG
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 1"), TEXT("01:01:00:00"), TEXT("01:01:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 2"), TEXT("01:01:30:00"), TEXT("01:01:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 3"), TEXT("01:02:00:00"), TEXT("01:02:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 4"), TEXT("01:02:30:00"), TEXT("01:02:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 5"), TEXT("01:03:00:00"), TEXT("01:03:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 6"), TEXT("01:03:30:00"), TEXT("01:03:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 7"), TEXT("01:04:00:00"), TEXT("01:04:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 8"), TEXT("01:04:30:00"), TEXT("01:04:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 9"), TEXT("01:05:00:00"), TEXT("01:05:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 10"), TEXT("01:05:30:00"), TEXT("01:05:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 11"), TEXT("01:06:00:00"), TEXT("01:06:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 12"), TEXT("01:06:30:00"), TEXT("01:06:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 13"), TEXT("01:07:00:00"), TEXT("01:07:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 14"), TEXT("01:07:30:00"), TEXT("01:07:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 15"), TEXT("01:08:00:00"), TEXT("01:08:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 16"), TEXT("01:08:30:00"), TEXT("01:08:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 17"), TEXT("01:09:00:00"), TEXT("01:09:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 18"), TEXT("01:09:30:00"), TEXT("01:09:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 19"), TEXT("01:10:00:00"), TEXT("01:10:09:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
	m_ClipList.AddItem(new CClip(TEXT("Tape 1"), TEXT("Untitled Clip 20"), TEXT("01:10:30:00"), TEXT("01:10:39:24"), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
#endif
	// Create a source side capture graph for previewing the audio and video inputs.
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, __uuidof(IGraphBuilder), reinterpret_cast<void**>(&m_pSourceGraph));
	if (SUCCEEDED(hr))
	{
#ifdef _DEBUG
		hr = CDSUtils::AddGraphToRot(m_pSourceGraph, &m_ROTRegisterSource);
#endif
		m_pIMediaEvent = m_pSourceGraph;
		ASSERT(m_pIMediaEvent);

		if (SUCCEEDED(hr))
		{
			hr = CDSUtils::AddFilter(m_pSourceGraph, CLSID_SmartTee, SMARTT_NAME, &m_pSmartT);
			if (SUCCEEDED(hr))
			{
				// Locate video screen renderer for the preview window.
				hr = CDSUtils::AddFilter(m_pSourceGraph, CLSID_VideoRendererDefault, VIDEORENDERER_NAME, &m_pVideoRenderer);
				if (SUCCEEDED(hr))
				{
					// Configure the renderer to be in windowless mode.
					CComQIPtr<IVMRFilterConfig, &IID_IVMRFilterConfig> pIVMRFilterConfig = m_pVideoRenderer;
					if (pIVMRFilterConfig)
					{
						hr = pIVMRFilterConfig->SetRenderingMode(VMRMode_Windowless);
					}

					// Configure the renderer to be in windowless mode.
					CComQIPtr<IVMRWindowlessControl, &IID_IVMRWindowlessControl> pIVMRWindowlessCtrl = m_pVideoRenderer;
					if (pIVMRWindowlessCtrl)
					{
						CWnd* pWnd = GetDlgItem(IDC_STATIC_PREVIEW);
						hr = pIVMRWindowlessCtrl->SetVideoClippingWindow(pWnd->GetSafeHwnd());	// Set the bounds of the video to the preview window.
						RECT rcDst = {0};
						pWnd->GetClientRect(&rcDst);
						SetRect(&rcDst, 0, 0, rcDst.right, rcDst.bottom);
						hr = pIVMRWindowlessCtrl->SetVideoPosition(NULL, &rcDst);	// Show the whole of the source frame in the whole of the client area of the control.
						hr = pIVMRWindowlessCtrl->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);	// Maintain the aspect ratio of the video.
						hr = pIVMRWindowlessCtrl->SetBorderColor(GetSysColor(COLOR_BTNFACE));	// Set the colour of the letter or pillar boxed area.
					}
				}

				// The sample grabber can be used for capturing a still during preview.
				if (SUCCEEDED(hr))
				{
					hr = CDSUtils::AddFilter(m_pSourceGraph, CLSID_SampleGrabber, SAMPLEGRABBERSTILLS_NAME, &m_pSampleGrabberStills);
					if (SUCCEEDED(hr))
					{
						// query for the sample grabber interface
						m_pISampleGrabberStills = m_pSampleGrabberStills;
						ASSERT(m_pISampleGrabberStills);
					}
				}

				// The sample grabber can be used for capturing timecode for frame accurate capture.
				if (SUCCEEDED(hr))
				{
					hr = CDSUtils::AddFilter(m_pSourceGraph, CLSID_NullRenderer, NULLRENDERER_NAME, &m_pNullRenderer);
					if (SUCCEEDED(hr))
					{
						hr = CDSUtils::AddFilter(m_pSourceGraph, CLSID_SampleGrabber, SAMPLEGRABBERTIMECODE_NAME, &m_pSampleGrabberTimecode);
						if (SUCCEEDED(hr))
						{
							// query for the sample grabber interface
							m_pISampleGrabberTimecode = m_pSampleGrabberTimecode;
							if (m_pISampleGrabberTimecode)
							{
								hr = m_pISampleGrabberTimecode->SetCallback(m_pTimecodeGrabber, 0);
							}
						}
					}
				}

				OnDeviceMenu(IDC_DEVICE_BASE + m_Device);	// Set the device name and build a preview graph.
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		// Create a sink side capture graph for writing the audio and video to disk.
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, __uuidof(IGraphBuilder), reinterpret_cast<void**>(&m_pSinkGraph));
		if (SUCCEEDED(hr))
		{
#ifdef _DEBUG
			hr = CDSUtils::AddGraphToRot(m_pSinkGraph, &m_ROTRegisterSink);
#endif
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------
// DestroyWindow
// Called when the window is being destroyed, clean up and free all resources.
BOOL CDecklinkIngestDlg::DestroyWindow()
{
#ifdef _DEBUG
	CDSUtils::RemoveGraphFromRot(m_ROTRegisterSource);
	CDSUtils::RemoveGraphFromRot(m_ROTRegisterSink);
#endif
	// Cancel the sample grabber callbacks.
	EnableSampleGrabberCallback(false);
	if (m_pISampleGrabberTimecode)
	{
		m_pISampleGrabberTimecode->SetCallback(NULL, 0);
	}

	DestroyThreads();	// Destroy the thread which uses the source interfaces that are about to be released.
	DestroySinkGraph();
	DestroySourceGraph();

	m_RegUtils.Close();

	// Release the filter names attached to the menu item's data
	CMenu* pMenu = GetMenu();
	int Item = FindMenuItem(pMenu, TEXT("&Device"));
	if (-1 != Item)
	{
		CMenu* pSubMenu = pMenu->GetSubMenu(Item);
		int Count = pSubMenu->GetMenuItemCount();
		for (int Device=0; Device<Count; ++Device)
		{
			UINT State = pSubMenu->GetMenuState(IDC_DEVICE_BASE + Device, MF_BYCOMMAND);
			if (0xFFFFFFFF != State)
			{
				MENUITEMINFO MenuItemInfo = {0};
				MenuItemInfo.cbSize = sizeof(MenuItemInfo);
				MenuItemInfo.fMask = MIIM_DATA;
				if (pSubMenu->GetMenuItemInfo(IDC_DEVICE_BASE + Device, &MenuItemInfo))
				{
					delete [] (LPWSTR*)MenuItemInfo.dwItemData;
				}
			}
		}
	}

	int cItems = m_ClipList.GetItemCount();
	for (int item=0; item<cItems; ++item)
	{
		CClip* pClip = reinterpret_cast<CClip*>(m_ClipList.GetItemData(item));
		if (pClip)
		{
			delete pClip;
		}
	}

	m_ClipList.DeleteAllItems();

	SAFE_DELETE(m_pTimecodeGrabber);
	SAFE_DELETE(m_pStillGrabber);

	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
// OnSysCommand
//
void CDecklinkIngestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.
void CDecklinkIngestDlg::OnPaint() 
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
HCURSOR CDecklinkIngestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-----------------------------------------------------------------------------
// OnEditPreferences
// Display the preferences dialog.
void CDecklinkIngestDlg::OnFileImportEDL()
{
	char BASED_CODE szFilters[] = "Edit Decision List|*.edl|All Files (*.*)|*.*||";

	CFileDialog FileDlg(TRUE, "Edit Decision Lists", TEXT("*.edl"), 0, szFilters, this);

	if (FileDlg.DoModal() == IDOK)
	{
		try
		{
			HRESULT hr = S_OK;
			CBaseEDLReader* pEDLReader = new CCMX3600EDLReader(FileDlg.GetPathName(), &hr);
		}
		catch (std::bad_alloc)
		{
		}
	}
}

//-----------------------------------------------------------------------------
// OnEditPreferences
// Display the preferences dialog.
void CDecklinkIngestDlg::OnEditPreferences()
{
	CPreferencesDlg dlg(m_CaptureFilePath, m_CaptureFilename, m_pVideoDevice, m_VideoFormat, m_pAudioDevice, m_AudioFormat, m_bMuteAudio, m_Compression);
	if (IDOK == dlg.DoModal())
	{
		// Set the correct file extension for the compression.
		m_Compression = dlg.GetCompression();
		SetFileExtension(m_CaptureFilename);

		// Validate the capture folder location and reset the file index if the location has changed.
		basic_string<TCHAR> Temp = dlg.GetCaptureLocation();
		if ((INVALID_FILE_ATTRIBUTES != GetFileAttributes(Temp.c_str())) && (m_CaptureFilePath != Temp))
		{
			m_CaptureFilePath = Temp;
			ResetFileIndex();
		}

		// Reset the file index if the base file name has changed.
		Temp = dlg.GetCaptureFilename();
		if (m_CaptureFilename != Temp)
		{
			m_CaptureFilename = Temp;
			ResetFileIndex();
		}

		// Set the filename for the stills grabber.
		if (m_pStillGrabber)
		{
			m_pStillGrabber->SetFilename(m_CaptureFilename);
		}

		// Rebuild the source graph if the audio or video formats or the audio preference has changed.
		CMediaType VideoFormat(dlg.GetVideoFormat());
		CMediaType AudioFormat(dlg.GetAudioFormat());
		bool bMuteAudio = dlg.GetMuteAudio();
		if (!(m_VideoFormat == VideoFormat) || !(m_AudioFormat == AudioFormat) || (m_bMuteAudio != bMuteAudio))
		{
			m_VideoFormat = VideoFormat;
			m_AudioFormat = AudioFormat;
			m_bMuteAudio = bMuteAudio;
			CreateCaptureSourceGraph();
		}

		SavePreferencesToRegistry();
	}
}

//-----------------------------------------------------------------------------
// OnDeviceMenu
// Change the device to be used for ingesting.
void CDecklinkIngestDlg::OnDeviceMenu(UINT nID)
{
	CMenu* pMenu = GetMenu();
	int Item = FindMenuItem(pMenu, TEXT("&Device"));
	if (-1 != Item)
	{
		// Update the menu with the new selection.
		CMenu* pSubMenu = pMenu->GetSubMenu(Item);
		pSubMenu->CheckMenuRadioItem(IDC_DEVICE_BASE, IDC_DEVICE_BASE + CDecklinkIngestDlg::MAXDEVICES - 1, nID, MF_BYCOMMAND);

		// Update the currently selected filter name.
		int Count = pSubMenu->GetMenuItemCount();
		for (int Device=0; Device<Count; ++Device)
		{
			UINT State = pSubMenu->GetMenuState(IDC_DEVICE_BASE + Device, MF_BYCOMMAND);
			if ((0xFFFFFFFF != State) && (State & MF_CHECKED))
			{
				MENUITEMINFO MenuItemInfo = {0};
				MenuItemInfo.cbSize = sizeof(MenuItemInfo);
				MenuItemInfo.fMask = MIIM_DATA;
				if (pSubMenu->GetMenuItemInfo(IDC_DEVICE_BASE + Device, &MenuItemInfo))
				{
					// Retrieve the capture device name from the menu.
					m_pDeviceNameVideo = (LPWSTR)MenuItemInfo.dwItemData;

					// Rebuild the source graph with the new device selection.
					CreateCaptureSourceGraph();
				}

				m_Device = Device;
				EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("VideoCaptureDevice"), reinterpret_cast<LPBYTE>(&m_Device), sizeof(m_Device)));
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// OnDeviceInfo
// Display the IO features of the selected device.
void CDecklinkIngestDlg::OnDeviceInfo()
{
	CDeviceInfoDlg dlg(m_pDeviceNameVideo, &CLSID_VideoInputDeviceCategory);
	dlg.DoModal();
}

//-----------------------------------------------------------------------------
// OnAboutBox
// Display the about box.
void CDecklinkIngestDlg::OnAboutBox()
{
	OnSysCommand(IDM_ABOUTBOX, 0);
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonLogclip
// Create a file browser dialog to retrieve the name of a clip to add to the filelist.
void CDecklinkIngestDlg::OnBnClickedButtonLogclip()
{
	CString Tapename, Clipname, Inpoint, Outpoint;
	m_TapenameCtrl.GetWindowText(Tapename);
	m_ClipnameCtrl.GetWindowText(Clipname);
	m_InpointCtrl.GetWindowText(Inpoint);
	m_OutpointCtrl.GetWindowText(Outpoint);

	// Append an appropriate file extension.
	basic_string<TCHAR> clipname = Clipname;
	SetFileExtension(clipname);

	try
	{
		m_ClipList.AddItem(new CClip(Tapename.GetBuffer(), clipname.c_str(), Inpoint.GetBuffer(), Outpoint.GetBuffer(), (WORD)(UNITS / CUtils::GetAvgTimePerFrame(&m_VideoFormat))));
		if (0 != m_ClipList.GetItemCount())
		{
			EnableWindows(LogClipCtrlIDs, SIZEOF_ARRAY(LogClipCtrlIDs), TRUE);
		}
	}
	catch (std::bad_alloc)
	{
		// memory allocation error
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonRemovefile
// Remove the selected file from the filelist.
void CDecklinkIngestDlg::OnBnClickedButtonRemoveclip()
{
	m_ClipList.RemoveItem();

	// if there are no items in the list, disable the export and stop buttons
	if (0 == m_ClipList.GetItemCount())
	{
		EnableWindows(LogClipCtrlIDs, SIZEOF_ARRAY(LogClipCtrlIDs), FALSE);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonMovefileup
// Move the selected file up in the filelist.
void CDecklinkIngestDlg::OnBnClickedButtonMoveclipup()
{
	m_ClipList.MoveItemUp();
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonMovefiledown
// Move the selected file down in the filelist.
void CDecklinkIngestDlg::OnBnClickedButtonMoveclipdown()
{
	m_ClipList.MoveItemDown();
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonPlay
//
void CDecklinkIngestDlg::OnBnClickedButtonPlay()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_Mode(ED_MODE_PLAY);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonStop
//
void CDecklinkIngestDlg::OnBnClickedButtonStop()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_Mode(ED_MODE_STOP);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonFrew
//
void CDecklinkIngestDlg::OnBnClickedButtonFrew()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_Mode(ED_MODE_REW);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonFfwd
//
void CDecklinkIngestDlg::OnBnClickedButtonFfwd()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_Mode(ED_MODE_FF);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonEject
//
void CDecklinkIngestDlg::OnBnClickedButtonEject()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_MediaState(ED_MEDIA_UNLOAD);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonCaptureStill
//
void CDecklinkIngestDlg::OnBnClickedButtonCaptureStill()
{
	if (m_pStillGrabber)
	{
		m_pStillGrabber->Capture();
		
		// Create a string for the status control.
		basic_string<TCHAR> Message = TEXT("Capturing still to: \"\"");
		basic_string<TCHAR>::size_type Index = Message.rfind(_T('\"'));
		Message.insert(Index, m_pStillGrabber->GetFilePath());
		VERIFY(m_StatusBarCtrl.SetText(Message.c_str(), 0, 0));
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonCaptureNow
// Start capturing from the device immediately.
void CDecklinkIngestDlg::OnBnClickedButtonCaptureNow()
{
	TCHAR szCaptureFilename[MAX_PATH] = {0};
	StringCchPrintf(szCaptureFilename, MAX_PATH, TEXT("%s %d"), m_CaptureFilename.c_str(), m_FileIndex++);

	if (SUCCEEDED(CreateCaptureSinkGraph(szCaptureFilename)))
	{
		// Start the capture streams.
		REFERENCE_TIME rtStop = MAXLONGLONG;
		if (SUCCEEDED(ConfigureSourceGraph(NULL, &rtStop)))	// Start the streams immediately and cancel any pending stop requests.
		{
			EnableSampleGrabberCallback(false);	// Cancel the sample grabber callback during capture.

			StartCapture();

			EnableWindows(CaptureCtrlIDs, SIZEOF_ARRAY(CaptureCtrlIDs), FALSE);
			CWnd* pWnd = GetDlgItem(IDC_BUTTON_ABORT);
			pWnd->EnableWindow(TRUE);
			pWnd->SetWindowText(TEXT("Stop"));

			// Create a string for the status control.
			basic_string<TCHAR> Message = TEXT("Capturing to: \"\"");
			basic_string<TCHAR>::size_type Index = Message.rfind(_T('\"'));
			Message.insert(Index, szCaptureFilename);
			VERIFY(m_StatusBarCtrl.SetText(Message.c_str(), 0, 0));
		}
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonBatchCapture
// Start capturing from the device using the batch list.
void CDecklinkIngestDlg::OnBnClickedButtonBatchCapture()
{
	CClip* pClip = reinterpret_cast<CClip*>(m_ClipList.GetItemData(0));
	basic_string<TCHAR> Message = TEXT("Please load the tape ");
	Message.append(pClip->Tapename());
	Message.append(TEXT(" into the deck."));
	if (IDOK == MessageBox(Message.c_str(), TEXT("Check/Load tape"), MB_ICONEXCLAMATION | MB_OKCANCEL))
	{
		m_CaptureTapename = pClip->Tapename();
		
		EnableSampleGrabberCallback(false);	// Cancel the sample grabber callback during capture.

		EnableWindows(CaptureCtrlIDs, SIZEOF_ARRAY(CaptureCtrlIDs), FALSE);
		CWnd* pWnd = GetDlgItem(IDC_BUTTON_ABORT);
		pWnd->EnableWindow(TRUE);
		pWnd->SetWindowText(TEXT("Abort"));

		m_ClipList.SetSelectedItem(0);

		m_bBatchCapture = true;
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonAbort
// Stop capture.
void CDecklinkIngestDlg::OnBnClickedButtonAbort()
{
	m_bBatchCapture = false;

	StopCapture();

	EnableSampleGrabberCallback(true);	// Enable the sample grabber callback during preview.

	EnableWindows(CaptureCtrlIDs, SIZEOF_ARRAY(CaptureCtrlIDs), TRUE);
	CWnd* pWnd = GetDlgItem(IDC_BUTTON_ABORT);
	pWnd->EnableWindow(FALSE);
	pWnd->SetWindowText(TEXT("Abort"));

	VERIFY(m_StatusBarCtrl.SetText(TEXT("Capture stopped."), 0, 0));
	m_ProgressCtrl.ShowWindow(SW_HIDE);
}

//-----------------------------------------------------------------------------
// Private methods.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// CreateDeviceMenu
//
HRESULT CDecklinkIngestDlg::CreateDeviceMenu(void)
{
	HRESULT hr = S_OK;
	const GUID* pCategory = &CLSID_VideoInputDeviceCategory;

	list< basic_string<WCHAR> > DeviceList;
	CDSUtils::EnumerateDevices(pCategory, DeviceList);

	// Create a device menu item for each product present in the system.
	CMenu menuDevice;
	VERIFY(menuDevice.CreatePopupMenu());
	if (!DeviceList.empty())
	{
		int cDevices = 0;
		CComPtr<IGraphBuilder> pGraph;
		if (SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, __uuidof(IGraphBuilder), reinterpret_cast<void**>(&pGraph))))
		{
			for (list< basic_string<WCHAR> >::iterator it = DeviceList.begin(); it != DeviceList.end(); ++it)
			{
				IBaseFilter* pFilter = NULL;
				if (SUCCEEDED(CDSUtils::AddFilter2(pGraph, *pCategory, it->c_str(), &pFilter)))
				{
					WCHAR buf[256] = {0};
					LPWSTR pVendorInfo = NULL;

					pFilter->QueryVendorInfo(&pVendorInfo);
					if (pVendorInfo)
					{
						StringCbPrintfW(buf, sizeof(buf), L"%d. %s", cDevices + 1, pVendorInfo);
						CoTaskMemFree(reinterpret_cast<LPVOID>(pVendorInfo));
					}
					else
					{
						StringCbPrintfW(buf, sizeof(buf), L"%d. %s", cDevices + 1, it->c_str());
					}

					if (menuDevice.AppendMenu(MF_STRING, IDC_DEVICE_BASE + cDevices, CW2A(buf)))
					{
						MENUITEMINFO MenuItemInfo = {0};
						MenuItemInfo.cbSize = sizeof(MenuItemInfo);
						MenuItemInfo.fMask = MIIM_DATA;

						basic_string<WCHAR>::size_type len = it->length() + 1;
						LPWSTR pName = new WCHAR[len];
						ZeroMemory(pName, len);
						StringCchCopyW(pName, len, it->c_str());
						
						MenuItemInfo.dwItemData = (DWORD)(__int64)pName;
						menuDevice.SetMenuItemInfo(IDC_DEVICE_BASE + cDevices, &MenuItemInfo);
					}

					pGraph->RemoveFilter(pFilter);
					SAFE_RELEASE(pFilter);
					
					if (++cDevices >= CDecklinkIngestDlg::MAXDEVICES)
					{
						// Cannot support more than MAXDEVICES.
						break;
					}
				}
			}
		}

		if (0 < cDevices)
		{
			menuDevice.AppendMenu(MF_SEPARATOR, 0, TEXT("Separator"));
			menuDevice.AppendMenu(MF_STRING, IDC_DEVICE_INFO, TEXT("Device &Info...\tCtrl+I"));
			
			if (m_Device >= cDevices)
			{
				m_Device = 0;
				EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("VideoCaptureDevice"), reinterpret_cast<LPBYTE>(&m_Device), sizeof(m_Device)));
			}
			menuDevice.CheckMenuRadioItem(IDC_DEVICE_BASE, IDC_DEVICE_BASE + CDecklinkIngestDlg::MAXDEVICES - 1, IDC_DEVICE_BASE + m_Device, MF_BYCOMMAND);
		}
		else
		{
			// It was not possible to instantiate any capture devices.
			menuDevice.AppendMenu(MF_STRING | MF_GRAYED, 0, TEXT("<No Devices>"));
			hr = S_FALSE;
		}
	}
	else
	{
		// No capture devices are present in the system.
		menuDevice.AppendMenu(MF_STRING | MF_GRAYED, 0, TEXT("<No Devices>"));
		hr = S_FALSE;
	}

	// Insert the newly created device menu into the main menu.
	CMenu* pMenu = GetMenu();
	int Item = FindMenuItem(pMenu, TEXT("&Help"));
	if (-1 != Item)
	{
		hr = pMenu->InsertMenu(Item, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)menuDevice.GetSafeHmenu(), TEXT("&Device")) ? S_OK : E_FAIL;
	}
	else
	{
		hr = E_FAIL;
	}

	menuDevice.Detach();

	return hr;
}

//-----------------------------------------------------------------------------
// FindMenuItem
//
int CDecklinkIngestDlg::FindMenuItem(CMenu* pMenu, LPCTSTR MenuString)
{
	int Item = -1;

	if (pMenu)
	{
		ASSERT(::IsMenu(pMenu->GetSafeHmenu()));

		int Count = pMenu->GetMenuItemCount();
		for (int item=0; item<Count; ++item)
		{
			CString str;
			if (pMenu->GetMenuString(item, str, MF_BYPOSITION) && (strcmp(str, CString(MenuString)) == 0))
			{
				Item = item;
				break;
			}
		}
	}

	return Item;
}

//-----------------------------------------------------------------------------
// CreateThread
//
HRESULT CDecklinkIngestDlg::CreateThreads(void)
{
	HRESULT hr = S_OK;

	if (NULL == m_hExitEvent)
	{
		m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
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
	}
	else
	{
		// Thread has already been created.
		hr = S_FALSE;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// DestroyThread
//
HRESULT CDecklinkIngestDlg::DestroyThreads(void)
{
	HRESULT hr = S_OK;

	if (m_hExitEvent)
	{
		// signal the thread to exit
		SetEvent(m_hExitEvent);

		// wait for thread to exit
		EXECUTE_ASSERT(WAIT_OBJECT_0 == WaitForSingleObject(m_hThread, 10000));

		CloseHandle(m_hExitEvent);
		m_hExitEvent = NULL;
		
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// ThreadProc
// Static function that wraps class thread implementation
DWORD WINAPI CDecklinkIngestDlg::ThreadProc(LPVOID lpParameter)
{
	ASSERT(lpParameter);
	CDecklinkIngestDlg* pDlg = reinterpret_cast<CDecklinkIngestDlg*>(lpParameter);
	return pDlg->Thread();
}

//-----------------------------------------------------------------------------
// Thread
// Actual class thread
DWORD CDecklinkIngestDlg::Thread(void)
{
	HRESULT hr = S_OK;
	enum { MODE_NEWCLIP, MODE_SEEKING_TO_PREROLL, MODE_PREROLLING, MODE_CAPTURING };
	int State = MODE_NEWCLIP, CurrentItem, Preroll = 500;
	CClip* pClip = NULL;

	CTimecode Timecode;
	REFERENCE_TIME rtTimecodeStart, rtTimecodeEnd;

	static double LastRate = 0.0;

	bool bRunning = true;
	while (bRunning)
	{
		switch (WaitForSingleObject(m_hExitEvent, 5))
		{
			default:
			case WAIT_ABANDONED:
			case WAIT_OBJECT_0:
				bRunning = false;
				break;

			case WAIT_TIMEOUT:
				// Periodically check the transport.
				CheckTransport();
				if (m_pIExtTransport)
				{
					basic_string<TCHAR> TimecodeSource;
					if (S_OK == m_pTimecodeGrabber->GetTimecode(Timecode, &rtTimecodeStart, &rtTimecodeEnd, &TimecodeSource))
					{
						UpdateTimecodeCtrl(Timecode);

						static basic_string<TCHAR> LastTimecodeSource;
						if (LastTimecodeSource != TimecodeSource)
						{
							VERIFY(m_StatusBarCtrl.SetText(TimecodeSource.c_str(), 1, 0));
							LastTimecodeSource = TimecodeSource;
						}

						if (m_bBatchCapture)
						{
							switch (State)
							{
								case MODE_NEWCLIP:
									// setup a new clip, if transport is available preroll to the inpoint
									CurrentItem = m_ClipList.GetSelectedItem();
									pClip = reinterpret_cast<CClip*>(m_ClipList.GetItemData(CurrentItem));
									if (pClip)
									{
										// Build a sink side capturegraph and specify the stream start and stop times for frame accurate capture.
										hr = CreateCaptureSinkGraph(pClip->Clipname().c_str());
										if (SUCCEEDED(hr))
										{
											// update the inpoint and outpoint ctrls with the current clip
											m_InpointCtrl.SetWindowText(pClip->Inpoint().TimecodeToString());
											m_OutpointCtrl.SetWindowText(pClip->Outpoint().TimecodeToString());
											m_DurationCtrl.SetWindowText(pClip->Duration().TimecodeToString());

											if (m_CaptureTapename != pClip->Tapename())
											{
												basic_string<TCHAR> Message = TEXT("Please check/load the tape ");
												Message.append(pClip->Tapename());
												Message.append(TEXT(" into the deck."));
												if (IDOK == MessageBox(Message.c_str(), TEXT("Check/Load tape"), MB_ICONEXCLAMATION | MB_OKCANCEL))
												{
													m_CaptureTapename = pClip->Tapename();
												}
												else
												{
													hr = E_FAIL;
												}
											}
										}
										
										if (SUCCEEDED(hr))
										{
											LastRate = -1000.0;
											State = MODE_SEEKING_TO_PREROLL;
											VERIFY(m_StatusBarCtrl.SetText(TEXT("Seeking to inpoint..."), 0, 0));
										}
										else
										{
											// A serious problem occurred building the sink side capture graph.
											OnBnClickedButtonAbort();
										}
									}
									else
									{
										// No more clips.
										OnBnClickedButtonAbort();
									}
									break;

								case MODE_SEEKING_TO_PREROLL:
									// At this point the transport is seeking to the preroll inpoint,
									// wait for the deck to hit the preroll inpoint before rolling the transport.
									{
										// Calculate the 'distance' from the inpoint.
										CTimecode PreRoll(pClip->Inpoint() - Preroll), Distance;
										double Direction, Rate;

										if (PreRoll > Timecode)
										{
											Distance = PreRoll - Timecode;
											Direction = 1.0;	// Move transport forwards.
										}
										else
										{
											Distance = Timecode - PreRoll;
											Direction = -1.0;	// Move transport backwards.
										}

										if (Distance < 200)	// 2 seconds from preroll.
										{
											// Move the transport at 1x.
											Rate = Direction;
										}
										else if (Distance < 1000)	// 10 seconds from preroll.
										{
											// Move the transport at 10x.
											Rate = Direction * 10.0;
										}
										else if (Distance < 3000)	// 30 seconds from preroll.
										{
											// Move the transport at 30x.
											Rate = Direction * 30.0;
										}
										else	// 30+ seconds from preroll.
										{
											// Move the transport at 50x.
											Rate = Direction * 50.0;
										}

										if (LastRate != Rate)
										{
											// Change the shuttle rate of the transport.
											m_pIExtTransport->put_Rate(Rate);
											m_pIExtTransport->put_Mode(ED_MODE_SHUTTLE);
											LastRate = Rate;
										}

										if ((Distance == 0) && (1 == abs(Rate)))
										{
											// At the preroll inpoint, roll the transport and start prerolling.
											m_pIExtTransport->put_Mode(ED_MODE_PLAY);
											State = MODE_PREROLLING;
											VERIFY(m_StatusBarCtrl.SetText(TEXT("Prerolling to inpoint..."), 0, 0));
										}
									}
									break;
								
								case MODE_PREROLLING:
									// At this point the transport is rolling to the inpoint,
									// determine the stream time of the inpoint and set the 
									// stream start times using IAMStreamControl.
									{
										CTimecode Distance = pClip->Inpoint() - Timecode;
										if (MAXLONGLONG != rtTimecodeStart)
										{
											// The timecode grabber was able to provide a stream time for the current timecode.
											// A short period before the inpoint, now that the deck is rolling, examine the stream
											// time for the timecode sample and determine the stream time of the video frame that
											// corresponds to the inpoint.
											if (Distance == 10)
											{
												REFERENCE_TIME rtAvgTimePerFrame = CUtils::GetAvgTimePerFrame(&m_VideoFormat);
												REFERENCE_TIME rtStart = rtTimecodeStart + (10 * rtAvgTimePerFrame);
												REFERENCE_TIME rtEnd = rtStart + ((pClip->Duration().GetFrameCount() - 1) * rtAvgTimePerFrame);
												ConfigureSourceGraph(&rtStart, &rtEnd);	// Set the window of the stream times of samples that will cross the bridge.
												StartCapture();	// Close the bridge and unblock the capture streams.
												State = MODE_CAPTURING;
												TRACE(TEXT("CDecklinkIngestDlg::Thread() ***** INPOINT is [%10I64d  %10I64d] ***** %s  [%10I64d  %10I64d]\r\n"), rtStart, rtEnd, Timecode.TimecodeToString(), rtTimecodeStart, rtTimecodeEnd);
											}
										}
										else
										{
											// The timecode grabber was not able to provide a stream time for the current timecode
											// so wait for the inpoint before starting capture.  This is likely to suffer from
											// inaccuracies.
											if (Distance == 0)
											{
												StartCapture();	// Close the bridge and unblock the capture streams.
												State = MODE_CAPTURING;
												TRACE(TEXT("CDecklinkIngestDlg::Thread() ***** INPOINT is NOW ***** %s\r\n"), Timecode.TimecodeToString());
											}
										}
									}
									break;
									
								case MODE_CAPTURING:
									// Wait for the sink side capture graph to issue an EC_STREAM_CONTROL_STARTED and
									// EC_STREAM_CONTROL_STOPPED event.  What the latter is received continue with the next clip.
									{
										// Provide some metrics on the capture progess.
										static CTimecode LastCaptured;
										CTimecode Captured = Timecode - pClip->Inpoint();
										CTimecode Remainder = pClip->Duration() - Captured;
										if (LastCaptured != Captured)
										{
											LastCaptured = Captured;
											basic_string<TCHAR> CaptureProgress = TEXT("Capturing...");
											CaptureProgress.append(Captured.TimecodeToString());
											CaptureProgress.append(TEXT("                                                    -"));
											CaptureProgress.append(Remainder.TimecodeToString());
											VERIFY(m_StatusBarCtrl.SetText(CaptureProgress.c_str(), 0, 0));
											m_ProgressCtrl.ShowWindow(SW_SHOWNORMAL);	// Show the progress control.
											m_ProgressCtrl.SetPos(Captured.GetFrameCount() * 100 / pClip->Duration().GetFrameCount());
										}

										long lEventCode;
										LONG_PTR lParam1, lParam2;
										HRESULT hr = S_OK;
										while (hr = m_pIMediaEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0), SUCCEEDED(hr))
										{
											hr = m_pIMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);

											if (EC_STREAM_CONTROL_STARTED == lEventCode)
											{
												TRACE(TEXT("CDecklinkIngestDlg::Thread() - Stream started event\r\n"));
											}
											else if (EC_STREAM_CONTROL_STOPPED == lEventCode)
											{
												TRACE(TEXT("CDecklinkIngestDlg::Thread() - Stream stopped event\r\n"));
												m_pIExtTransport->put_Mode(ED_MODE_STOP);	// Stop the transport.

												if (0 == m_ClipList.SetSelectedItem(++CurrentItem))
												{
													// No more clips.
													OnBnClickedButtonAbort();
													MessageBox(TEXT("Batch capture completed."), TEXT("Batch capture"), MB_ICONINFORMATION | MB_OK);
												}
												else
												{
													StopCapture();	// Open the bridge and block the capture streams.
													m_ProgressCtrl.ShowWindow(SW_HIDE);	// Hide the progress control.
												}

												State = MODE_NEWCLIP;
												break;
											}
										}
									}
									break;
							}
						}
						else
						{
							if (MODE_NEWCLIP != State)
							{
								State = MODE_NEWCLIP;
							}
						}
					}
					else if (m_bBatchCapture)
					{
						// If failed to get timecode in the middle of a batch capture, abort.
						OnBnClickedButtonAbort();
						State = MODE_NEWCLIP;
					}
				}
				break;		
		}	
	}

	return 0;
}

//-----------------------------------------------------------------------------
// CheckTransport
// Make some rudimentary checks of the transport, such as are there comms (is
// the RS232 connected), is it in remote mode, is the tape loaded and is record
// inhibit disabled.
HRESULT CDecklinkIngestDlg::CheckTransport(void)
{
	HRESULT hr = S_FALSE;

	static int bLastEnableTimecodeCtrl = -1;
	static int bLastEnableTransportCtrls = -1;
	int bEnableTimecodeCtrl = FALSE;
	int bEnableTransportCtrls = FALSE;

	if (m_pIExtTransport)
	{
		// Test to see if there are comms with the deck.
		long state;
		hr = m_pIExtTransport->get_LocalControl(&state);
		if (SUCCEEDED(hr))
		{
			if (OAFALSE == state)
			{
				// The deck is in remote control mode.
				hr = m_pIExtTransport->get_MediaState(&state);
				if (SUCCEEDED(hr) && (ED_MEDIA_UNLOAD != state))
				{
					// Enable the controls as there are comms with the deck,
					// it is in remote mode and there is a tape loaded.
					bEnableTimecodeCtrl = bEnableTransportCtrls = TRUE;
					hr = S_OK;
				}
			}
		}
	}
	else
	{
		hr = E_FAIL;
	}

	CWnd* pWnd = NULL;
	if (bLastEnableTimecodeCtrl != bEnableTimecodeCtrl)
	{
		pWnd = GetDlgItem(IDC_STATIC_TIMECODE);
		pWnd->EnableWindow(bEnableTimecodeCtrl ? TRUE : FALSE);
		bLastEnableTimecodeCtrl = bEnableTimecodeCtrl;
	}

	if (bLastEnableTransportCtrls != bEnableTransportCtrls)
	{
		EnableWindows(TransportCtrlIDs, SIZEOF_ARRAY(TransportCtrlIDs), bEnableTransportCtrls ? TRUE : FALSE);
		bLastEnableTransportCtrls = bEnableTransportCtrls;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// UpdateTimecodeCtrl
//
void CDecklinkIngestDlg::UpdateTimecodeCtrl(CTimecode& Timecode)
{
	static CTimecode LastTimecode;

	if (LastTimecode != Timecode)
	{
		DWORD dwStyle = m_TimecodeCtrl.GetStyle();
		if (!(dwStyle & WS_DISABLED))
		{
			m_TimecodeCtrl.SetWindowText(Timecode.TimecodeToString());
			LastTimecode = Timecode;
		}
	}
}

//-----------------------------------------------------------------------------
// CreateCaptureSourceGraph
// Build a full source side capture graph but with the capture path diabled.  This
// will preview the capture stream until a capture option is selected.  The capture
// stream will then be enabled in order to write to disk.
//
// The following source side graph is built:
//
// Decklink Video Capture (Capture) -> Smart T (Capture) -> (Input 1) GMFBridge Sink
//                                     Smart T (Preview) -> AVI Decompressor -> Stills Sample Grabber -> Video Renderer
// Decklink Video Capture (~Timecode) -> Timecode Sample Grabber -> Null Renderer
// Decklink Audio Capture (Capture) -> Smart T (Capture) -> (Input 2) GMFBridge Sink
//
//
// TODO: HDV Capture, device is 'Microsoft AV/C Tape Subunit Device' with output pin 'MPEG2TS Out'
//		 DV Capture, device is 'Microsoft DV Camera and VCR' with output pins 'DV Vid Out' and 'DV A/V Out'
//		 NOTE: These devices can come and go so it will be necessary to keep checking that the device is available.
//
HRESULT CDecklinkIngestDlg::CreateCaptureSourceGraph(void)
{
	DestroyThreads();	// Destroy the thread which uses the source interfaces that are about to be released.
	DestroySinkGraph();
	DestroySourceGraph();

	HRESULT hr = CreateThreads(), hr2 = S_OK;
	if (SUCCEEDED(hr))
	{
		// Create a bridge controller object.
		hr = CoCreateInstance(__uuidof(GMFBridgeController), NULL, CLSCTX_INPROC_SERVER, __uuidof(IGMFBridgeController), reinterpret_cast<void**>(&m_pIGMFBridgeController));
		if (SUCCEEDED(hr))
		{
			// Add two streams to the bridge controller, one for audio and one for video.
			hr = m_pIGMFBridgeController->AddStream(TRUE, eMuxInputs, TRUE);
			
			if (false == m_bMuteAudio)
			{
				hr2 = m_pIGMFBridgeController->AddStream(FALSE, eMuxInputs, TRUE);
			}

			if (SUCCEEDED(hr) && SUCCEEDED(hr2))
			{
				// Add a bridge sink filter for the audio and the video streams.
				CComPtr<IUnknown> pIUnknown;
				hr = m_pIGMFBridgeController->InsertSinkFilter(m_pSourceGraph, &pIUnknown);
				if (SUCCEEDED(hr))
				{
					CMediaType MediaType;
					MediaType.InitMediaType();

					m_pBridgeSink = pIUnknown;

					// Add the video capture filter.
					hr = CDSUtils::AddFilter2(m_pSourceGraph, CLSID_VideoInputDeviceCategory, m_pDeviceNameVideo, &m_pVideoDevice);
					if (SUCCEEDED(hr))
					{
						m_pIExtTransport = m_pVideoDevice;	// QI capture source for the transport control interface.
						m_pTimecodeGrabber->SetTimecodeFilter(m_pVideoDevice);	// Set the filter for reading timecode from VITC, RP188 and RS422
						
						// Set the video format.
						CComPtr<IAMStreamConfig> pIAMStreamConfig;
						hr = CDSUtils::FindPinInterface(m_pVideoDevice, L"Capture", __uuidof(IAMStreamConfig), reinterpret_cast<void**>(&pIAMStreamConfig));
						if (SUCCEEDED(hr))
						{
							hr = pIAMStreamConfig->SetFormat(&m_VideoFormat);
						}
						
						// Set the timecode format.
						pIAMStreamConfig = NULL;
						hr = CDSUtils::FindPinInterface(m_pVideoDevice, L"~Timecode", __uuidof(IAMStreamConfig), reinterpret_cast<void**>(&pIAMStreamConfig));
						if (SUCCEEDED(hr))
						{
							MediaType.SetType(&MEDIATYPE_Timecode);
							MediaType.SetSampleSize(sizeof(TIMECODE_SAMPLE));
							MediaType.SetFormatType(&FORMAT_None);
							hr = pIAMStreamConfig->SetFormat(&MediaType);
						}
					}
						
					// Add the audio capture filter by creating the audio capture filter name from the name of the video capture filter.
					basic_string<WCHAR> DeviceNameAudio = m_pDeviceNameVideo;
					basic_string<WCHAR>::size_type Index = DeviceNameAudio.find(L"Video");
					if (basic_string<WCHAR>::npos != Index)
					{
						DeviceNameAudio.replace(Index, 5, L"Audio");
					}
					hr = CDSUtils::AddFilter2(m_pSourceGraph, CLSID_AudioInputDeviceCategory, DeviceNameAudio.c_str(), &m_pAudioDevice);
					if (SUCCEEDED(hr))
					{
						// Set the audio format.
						CComPtr<IAMStreamConfig> pIAMStreamConfig;
						hr = CDSUtils::FindPinInterface(m_pAudioDevice, L"Capture", __uuidof(IAMStreamConfig), reinterpret_cast<void**>(&pIAMStreamConfig));
						if (SUCCEEDED(hr))
						{
							hr = pIAMStreamConfig->SetFormat(&m_AudioFormat);
						}
					}

					if (SUCCEEDED(hr))
					{
						// Attempt to render the timecode pin.
						hr = CDSUtils::ConnectFilters(m_pSourceGraph, m_pVideoDevice, L"~Timecode", m_pSampleGrabberTimecode, NULL);
						if (SUCCEEDED(hr))
						{
							hr = CDSUtils::ConnectFilters(m_pSourceGraph, m_pSampleGrabberTimecode, NULL, m_pNullRenderer, NULL);
						}

						// Attempt to render the video preview path.
						hr = CDSUtils::ConnectFilters(m_pSourceGraph, m_pVideoDevice, L"Capture", m_pSmartT, NULL);
						if (SUCCEEDED(hr))
						{
							// Set the media type for the still sample grabber by modifying a copy of the video capture format.
							MediaType.Set(m_VideoFormat);	// Copy the current video format.
							MediaType.SetSubtype(&MEDIASUBTYPE_RGB32);
							BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(&MediaType);
							if (pbmih)
							{
								// Set to BGRA as the still grabber can only handles this pixel format.
								pbmih->biBitCount = 32;
								pbmih->biCompression = BI_RGB;
								pbmih->biSizeImage = CUtils::GetImageSize(pbmih);
								MediaType.SetSampleSize(pbmih->biSizeImage);

								m_pStillGrabber->SetBMIH(pbmih);
								if (m_pISampleGrabberStills)
								{
									hr = m_pISampleGrabberStills->SetMediaType(&MediaType);
								}
							}

							// Connect the preview pin of the smart-T to the video renderer.
							hr = CDSUtils::ConnectFilters(m_pSourceGraph, m_pSmartT, L"Preview", m_pSampleGrabberStills, NULL);
							if (SUCCEEDED(hr))
							{
								hr = CDSUtils::ConnectFilters(m_pSourceGraph, m_pSampleGrabberStills, NULL, m_pVideoRenderer, NULL);
								if (SUCCEEDED(hr))
								{
									EnableSampleGrabberCallback(true);
								}
							}
						}

						// Attempt to render the video capture path.
						if (SUCCEEDED(hr))
						{
							hr = CDSUtils::ConnectFilters(m_pSourceGraph, m_pSmartT, L"Capture", m_pBridgeSink, L"Input 1");
						}

						// Attempt to render the audio path.
						if (SUCCEEDED(hr) && (false == m_bMuteAudio))
						{
							hr = CDSUtils::ConnectFilters(m_pSourceGraph, m_pAudioDevice, L"Capture", m_pBridgeSink, NULL);
						}

						// Run the source side capture graph until a change is made that requires the graph
						// to be rebuilt, such as a different capture device is selected.
						if (SUCCEEDED(hr))
						{
							// Turn off the capture streams.
							hr = ConfigureSourceGraph(NULL, NULL);

							CComQIPtr<IMediaControl, &IID_IMediaControl> pIMediaControl = m_pSourceGraph;
							if (pIMediaControl)
							{
								pIMediaControl->Run();
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
// ConfigureSourceGraph
// Sets the stream start and stop times on the input pins of the bridge sink filter.
// As all pins are configured to have the same start and stop times, only one pin is
// configured to issue EC_STREAM_CONTROL_STARTED and EC_STREAM_CONTROL_STOPPED event
// notifications.  These notifications are useful for monitoring the state of the
// stream and once EC_STREAM_CONTROL_STOPPED is received, the file writing graph
// can be stopped to finalise the file and complete the capture.
HRESULT CDecklinkIngestDlg::ConfigureSourceGraph(REFERENCE_TIME* prtStart, REFERENCE_TIME* prtStop)
{
	HRESULT hr = S_OK;

	if (m_pBridgeSink)
	{
		DWORD dwStartCookie = 0x100, dwStopCookie = 0x10000;

		// Process all the connected input pins of bridge source filter.
		CComPtr<IEnumPins> pIEnumPins = NULL;
		hr = m_pBridgeSink->EnumPins(&pIEnumPins);
		if (SUCCEEDED(hr))
		{
			CComQIPtr<IAMStreamControl, &IID_IAMStreamControl> pIAMStreamControl;

			IPin* pIPin = NULL;
			while ((S_OK == pIEnumPins->Next(1, &pIPin, NULL)) && SUCCEEDED(hr))
			{
				// Determine if the pin is connected.
				IPin* pIPinTemp = NULL;
				if (SUCCEEDED(pIPin->ConnectedTo(&pIPinTemp)))
				{
					SAFE_RELEASE(pIPinTemp);	// Release the outstanding reference.

					// Set the start and stop times on connected input pins and fail
					// if the connected input pin does not have a stream control interface.
					pIAMStreamControl = pIPin;
					if (pIAMStreamControl)
					{
						// Configure the stream with the given start and stop times.
						hr = pIAMStreamControl->StartAt(prtStart, dwStartCookie);
						if (SUCCEEDED(hr))
						{
							hr = pIAMStreamControl->StopAt(prtStop, FALSE, dwStopCookie);
						}
						
						// As all the streams will start and stop at the same time only one pin
						// needs to issue the EC_STREAM_CONTROL_STARTED and EC_STREAM_CONTROL_STOPPED
						// event notifications.
						dwStartCookie = dwStopCookie = 0;
					}
					else
					{
						hr = E_NOINTERFACE;
					}
				}

				SAFE_RELEASE(pIPin);
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
// CreateCaptureSinkGraph
// Create a sink side graph that will use the selected compression method before
// writing the streams to disk.  A new graph is created for each file and is independent
// of the source side graph until the bridge is closed.  This graph is not run until
// capture has been initiated.
HRESULT CDecklinkIngestDlg::CreateCaptureSinkGraph(LPCTSTR pszFilename)
{
	HRESULT hr = S_OK;
	
	if (pszFilename)
	{
		hr = DestroySinkGraph();
		if (SUCCEEDED(hr))
		{
			// Add a bridge source filter.
			CComPtr<IUnknown> pIUnknown;
			hr = m_pIGMFBridgeController->InsertSourceFilter(m_pBridgeSink, m_pSinkGraph, &pIUnknown);
			if (SUCCEEDED(hr))
			{
				m_pBridgeSource = pIUnknown;

				// From the compression type, add a mux and file writer where appropriate.
				CComPtr<IBaseFilter> pAVIMux, pFileWriter, pStreamControlFilter;
				if (ENC_WM == m_Compression)
				{
					hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_WMAsfWriter, L"WM ASF Writer", &pFileWriter);
					pStreamControlFilter = pFileWriter;
				}
				else if ((ENC_SEQ_CIN == m_Compression) || (ENC_SEQ_DPX == m_Compression) || (ENC_SEQ_TGA == m_Compression) || (ENC_SEQ_BMP == m_Compression))
				{
					hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_DecklinkStillSequenceSinkFilter, L"Decklink Still Sequence Sink", &pFileWriter);
					pStreamControlFilter = pFileWriter;
				}
				else
				{
					hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_AviDest, L"AVI Mux", &pAVIMux);
					if (SUCCEEDED(hr))
					{
						hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_FileWriter, L"File Writer", &pFileWriter);
					}
					pStreamControlFilter = pAVIMux;
				}

				// Build a file writing graph using the appropriate compressor.
				if (SUCCEEDED(hr))
				{
					// Obtain the IFileSinkFilter interface from the file writer filter and set the capture filename.
					m_pIFileSinkFilter = pFileWriter;
					if (m_pIFileSinkFilter)
					{
						// Create the full path of the capture filename.
						TCHAR szCaptureFilePath[MAX_PATH] = {0};
						StringCchPrintf(szCaptureFilePath, MAX_PATH, TEXT("%s"), m_CaptureFilePath.c_str());
						PathAppend(szCaptureFilePath, pszFilename);
						StringCchCat(szCaptureFilePath, MAX_PATH, m_CaptureFileExtension.c_str());

						hr = m_pIFileSinkFilter->SetFileName(CT2OLE(szCaptureFilePath), NULL);

						if (SUCCEEDED(hr))
						{
							switch (m_Compression)
							{
								default:
								case ENC_NONE:
									hr = CreateUncompressedCaptureSinkGraph(pAVIMux, pFileWriter);
									break;
									
								case ENC_DV:
									hr = CreateDVCaptureSinkGraph(pAVIMux, pFileWriter);
									break;
									
								case ENC_WM:
									hr = CreateWMCaptureSinkGraph(NULL, pFileWriter);
									break;
									
								case ENC_MJ:
									hr = CreateMJPEGCaptureSinkGraph(pAVIMux, pFileWriter);
									break;

								case ENC_SEQ_CIN:
								case ENC_SEQ_DPX:
								case ENC_SEQ_TGA:
								case ENC_SEQ_BMP:
									hr = CreateStillSequenceCaptureSinkGraph(NULL, pFileWriter);
									break;
							}

							if (SUCCEEDED(hr))
							{
								CComQIPtr<IMediaControl, &IID_IMediaControl> pIMediaControl = m_pSinkGraph;
								if (pIMediaControl)
								{
									pIMediaControl->Pause();
								}
							}
						}
					}
					else
					{
						hr = E_POINTER;
					}
				}
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	if (FAILED(hr))
	{
		MessageBox(TEXT("An error was detected while building\r\nthe compression and file writing graph."), TEXT("DirectShow Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CreateUncompressedCaptureSinkGraph
// Create a graph for writing uncompressed streams to disk using the AVI mux
// and file writer filters.
//
// The following sink side graph is built:
//
// GMFBridge Source (Output 1) -> (Input 01) AVI Mux -> File Writer
// GMFBridge Source (Output 2) -> (Input 02) AVI Mux
//
HRESULT CDecklinkIngestDlg::CreateUncompressedCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter)
{
	HRESULT hr = S_OK;

	if (pAVIMux && pFileWriter)
	{
		// Connect the bridge video output pin to the mux.
		hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 1", pAVIMux, NULL);
		if (SUCCEEDED(hr))
		{
			// Connect the mux to the file writer.
			hr = CDSUtils::ConnectFilters(m_pSinkGraph, pAVIMux, NULL, pFileWriter, NULL);

			// Video path connected now optionally connect the audio path.
			if (false == m_bMuteAudio)
			{
				// Connect the bridge audio output pin to the mux.
				hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 2", pAVIMux, NULL);
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
// CreateDVCaptureSinkGraph
// Create a graph for writing DV compressed streams to disk using the AVI mux
// and file writer filter.  A custom field swap filter is inserted for PAL
//
// The following sink side graph is built:
//
// GMFBridge Source (Output 1) -> AVI Decompressor -> DV Video Encoder -> (Input 01) AVI Mux -> File Writer
// GMFBridge Source (Output 2) -----------------------------------------> (Input 02) AVI Mux
//
HRESULT CDecklinkIngestDlg::CreateDVCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter)
{
	HRESULT hr = S_OK;
	BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(m_VideoFormat);	// This is required to configure the DV encoder.

	if (pAVIMux && pFileWriter && pbmih)
	{
		// Locate the DV encoder and add it to the graph.
		CComPtr<IBaseFilter> pEncoder = NULL;
		HRESULT hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_DVVideoEnc, L"DV Video Encoder", &pEncoder);
		if (SUCCEEDED(hr))
		{
			// Configure the DV encoder.
			CComQIPtr<IDVEnc, &IID_IDVEnc> pIDV = pEncoder;
			if (pIDV)
			{
				// Located a DV compression filter so set the format.
				int VideoFormat, DVFormat, Resolution;
				hr = pIDV->get_IFormatResolution(&VideoFormat, &DVFormat, &Resolution, FALSE, NULL);
				if (SUCCEEDED(hr))
				{
					ASSERT(DVENCODERFORMAT_DVSD == DVFormat);
					ASSERT(DVENCODERRESOLUTION_720x480 == Resolution);

					if ((DVENCODERVIDEOFORMAT_NTSC == VideoFormat) && (576 == pbmih->biHeight))
					{
						// Set the encoder to PAL if it is currently NTSC.
						VideoFormat = DVENCODERVIDEOFORMAT_PAL;
						hr = pIDV->put_IFormatResolution(VideoFormat, DVFormat, Resolution, FALSE, NULL);
					}
					else if ((DVENCODERVIDEOFORMAT_PAL == VideoFormat) && (486 == pbmih->biHeight))
					{
						// Set the encoder to NTSC if it is currently PAL.
						VideoFormat = DVENCODERVIDEOFORMAT_NTSC;
						hr = pIDV->put_IFormatResolution(VideoFormat, DVFormat, Resolution, FALSE, NULL);
					}
				}
			}

			if (SUCCEEDED(hr))
			{
				// If the format is PAL, insert the Decklink field swap filter, PAL DV is the opposite field order to PAL SD.
				if (576 == pbmih->biHeight)
				{
					CComPtr<IBaseFilter> pPALFieldSwap = NULL;
					hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_DecklinkFieldSwap, L"Decklink PAL Field Swap", &pPALFieldSwap);
					if (SUCCEEDED(hr))
					{
						// Connect the bridge video output pin to the PAL field swap filter.
						hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 1", pPALFieldSwap, NULL);
						if (SUCCEEDED(hr))
						{
							// Connect the field swap filter to the DV encoder.
							hr = CDSUtils::ConnectFilters(m_pSinkGraph, pPALFieldSwap, NULL, pEncoder, NULL);
						}
					}
				}
				else
				{
					// Connect the bridge video output pin to the DV Encoder.
					hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 1", pEncoder, NULL);
				}

				if (SUCCEEDED(hr))
				{
					// Connect the DV encoder output to the AVI mux.
					hr = CDSUtils::ConnectFilters(m_pSinkGraph, pEncoder, NULL, pAVIMux, NULL);
					if (SUCCEEDED(hr))
					{
						// Connect the mux to the file writer.
						hr = CDSUtils::ConnectFilters(m_pSinkGraph, pAVIMux, NULL, pFileWriter, NULL);
						if (SUCCEEDED(hr))
						{
							// Video path connected now optionally connect the audio path.
							if (false == m_bMuteAudio)
							{
								// Connect the bridge audio output pin to the mux.
								hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 2", pAVIMux, NULL);
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
// CreateWMCaptureSinkGraph
// Create a graph for writing WM compressed streams to disk using the WM ASF writer
// filter.
//
// The following sink side graph is built:
//
// GMFBridge Source (Output 1) -> (Video Input 1) WM ASF Writer
// GMFBridge Source (Output 2) -> (Audio Input 1) WM ASF Writer
//
HRESULT CDecklinkIngestDlg::CreateWMCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter)
{
	HRESULT hr = S_OK;

	if (pFileWriter)
	{
		hr = ConfigureWMEncoder(pFileWriter);
		if (SUCCEEDED(hr))
		{
			if (false == m_bMuteAudio)
			{
				// Connect the bridge audio output pin to the ASF writer.
				hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, pFileWriter, &MEDIATYPE_Audio);
			}
				
			if (SUCCEEDED(hr))
			{
				// Connect the bridge video output pin to the ASF writer.
				hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, pFileWriter, &MEDIATYPE_Video);
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
// ConfigureWMEncoder
// Configure the Windows Media encoder filter.
HRESULT CDecklinkIngestDlg::ConfigureWMEncoder(IBaseFilter* pASFWriter)
{
	HRESULT hr = S_OK;

	// Modify the video output resolution of a system profile.
	if (pASFWriter)
	{
		// Simple system profile encoding.
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
			// Get a profile manager.
			CComPtr<IWMProfileManager> pIWMProfileManager = NULL;
			hr = WMCreateProfileManager(&pIWMProfileManager);
			if (SUCCEEDED(hr))
			{
				// Load a system profile to modify.
				CComPtr<IWMProfile> pIWMProfile = NULL;
				// NOTE: Any WMProfile_XXX could be used here, or create a custom profile from scratch.
				hr = pIWMProfileManager->LoadProfileByID(WMProfile_V80_FAIRVBRVideo, &pIWMProfile);
				if (SUCCEEDED(hr))
				{
					// Search the streams for the video stream and attempt to modify the video size.
					DWORD cbStreams = 0;
					hr = pIWMProfile->GetStreamCount(&cbStreams);
					if (SUCCEEDED(hr))
					{
						IWMStreamConfig* pIWMStreamConfig = NULL;
						GUID streamType = {0};
						DWORD stream;
						
						if (m_bMuteAudio)
						{
							// Remove the audio stream for video only capture.
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
									// Found the video stream.
									CComQIPtr<IWMMediaProps, &IID_IWMMediaProps> pIWMMediaProps = pIWMStreamConfig;
									if (pIWMMediaProps)
									{
										// Get the size of the media type.
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
														// Modify the video dimensions, set the property, reconfigure the stream
														// and then configure the ASF writer with this modified profile.
														pbmih->biWidth = 640;	// was 320;
														pbmih->biHeight = 480;	// was 240;
														pbmih->biSizeImage = pbmih->biWidth * pbmih->biHeight * pbmih->biBitCount / 8;	// NOTE: This calculation is not correct for all bit depths.
														hr = pIWMMediaProps->SetMediaType(pMediaType);
														if (SUCCEEDED(hr))
														{
															// Config the ASF writer filter to use this modified system profile.
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
			// Modify other ASF writer properties.
			IServiceProvider* pProvider = NULL;
			hr = pASFWriter->QueryInterface(IID_IServiceProvider, reinterpret_cast<void**>(&pProvider));
			if (SUCCEEDED(hr))
			{
				IID_IWMWriterAdvanced2* pWMWA2 = NULL;
				hr = pProvider->QueryService(IID_IID_IWMWriterAdvanced2, IID_IID_IWMWriterAdvanced2, reinterpret_cast<void**>(&pWMWA2));
				if (SUCCEEDED(hr))
				{
					// Set the deinterlace mode.
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
// CreateMJPEGCaptureSinkGraph
// Create a graph for writing MJPEG compressed streams to disk using the AVI and
// file writer filters.
//
// The following sink side graph is built:
//
// GMFBridge Source (Output 1) -> Decklink MJPEG Compressor -> (Input 01) AVI Mux -> File Writer
// GMFBridge Source (Output 2) ------------------------------> (Input 02) AVI Mux
//
HRESULT CDecklinkIngestDlg::CreateMJPEGCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter)
{
	HRESULT hr = S_OK;

	if (pAVIMux && pFileWriter)
	{
		// Locate the DeckLink MJPEG encoder and add it to the graph.
		CComPtr<IBaseFilter> pEncoder = NULL;
		hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_DecklinkMJPEGEncoderFilter, L"Decklink MJPEG Compressor", &pEncoder);
		if (SUCCEEDED(hr))
		{
			// Configure the MJPEG encoder.
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
				// Connect the bridge video output pin to the MJPEG Encoder.
				hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 1", pEncoder, NULL);
				if (SUCCEEDED(hr))
				{
					// Connect the MJPEG encoder output to the AVI mux.
					hr = CDSUtils::ConnectFilters(m_pSinkGraph, pEncoder, NULL, pAVIMux, NULL);
					if (SUCCEEDED(hr))
					{
						// Connect the mux to the file writer.
						hr = CDSUtils::ConnectFilters(m_pSinkGraph, pAVIMux, NULL, pFileWriter, NULL);
						if (SUCCEEDED(hr))
						{
							// Video path connected now optionally connect the audio path.
							if (false == m_bMuteAudio)
							{
								// Connect the bridge audio output pin to the mux.
								hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 2", pAVIMux, NULL);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CreateStillSequenceCaptureSinkGraph
// Create a graph for writing a sequence of stills to disk using the DeckLink
// Still Sequence file writer filter.  The audio is written into a separate WAV
// file using the DirectShow sample WAVDest filter.
//
// The following sink side graph is built:
//
// GMFBridge Source (Output 1) -> Decklink Still Sequence Sink
// GMFBridge Source (Output 2) -> Wav Dest -> File Writer
//
HRESULT CDecklinkIngestDlg::CreateStillSequenceCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter)
{
	HRESULT hr = S_OK;

	if (pFileWriter)
	{
		// Create a directory using the base filename of the sequence of stills to capture the still sequence into.
		LPOLESTR pszFilename;
		hr = m_pIFileSinkFilter->GetCurFile(&pszFilename, NULL);
		if (SUCCEEDED(hr))
		{
			basic_string<TCHAR> FilePath = COLE2T(pszFilename);
			basic_string<TCHAR> Filename = FilePath;
			CoTaskMemFree(pszFilename);	// Release memory.
		
			// Create the directory name by removing the file extension from the filename.
			basic_string<TCHAR>::size_type Index = FilePath.rfind(_T('.'));
			if (basic_string<TCHAR>::npos != Index)
			{
				FilePath.erase(Index);
			}

			// Create the directory.
			BOOL ret = CreateDirectory(FilePath.c_str(), NULL);
			if (!ret && (ERROR_ALREADY_EXISTS != GetLastError()))
			{
				hr = E_FAIL;
			}

			if (SUCCEEDED(hr))
			{
				// Create the full filename including the newly created directory in the file path.
				Index = Filename.rfind(_T('\\'));
				Filename = Filename.substr(Index);
				// Insert white space at the end of the filename so that the index of the capture is not used by the still sequence writer.
				Index = Filename.rfind(_T('.'));
				Filename.insert(Index, TEXT(" "));

				TCHAR szCaptureFilePath[MAX_PATH] = {0};
				StringCchPrintf(szCaptureFilePath, MAX_PATH, TEXT("%s"), FilePath.c_str());
				PathAppend(szCaptureFilePath, Filename.c_str());

				// Set the new filename.
				hr = m_pIFileSinkFilter->SetFileName(CT2OLE(szCaptureFilePath), NULL);
				if (SUCCEEDED(hr))
				{
					// Connect the bridge video output pin to the still sequence writer.
					hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 1", pFileWriter, NULL);
				}

				// Video path connected now optionally connect the audio path.
				if (SUCCEEDED(hr) && (false == m_bMuteAudio))
				{
					// Add the Microsoft DirectShow SDK WAV Dest sample filter.
					CComPtr<IBaseFilter> pAudioDest;
					hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_WavDest, L"WAV Dest", &pAudioDest);
					if (SUCCEEDED(hr))
					{
						// Add a second file writer filter for the audio stream.
						CComPtr<IBaseFilter> pFileWriter;
						hr = CDSUtils::AddFilter(m_pSinkGraph, CLSID_FileWriter, L"File Writer", &pFileWriter);
						if (SUCCEEDED(hr))
						{
							// Connect the bridge audio output pin to the WAV Dest filter.
							hr = CDSUtils::ConnectFilters(m_pSinkGraph, m_pBridgeSource, L"Output 2", pAudioDest, NULL);
							if (SUCCEEDED(hr))
							{
								// Connect the WAV Dest filter to the file writer filter.
								hr = CDSUtils::ConnectFilters(m_pSinkGraph, pAudioDest, NULL, pFileWriter, NULL);
								if (SUCCEEDED(hr))
								{
									// Set the name of the audio file by using the base name of the sequence of stills.
									CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pIFileSinkFilter = pFileWriter;
									if (pIFileSinkFilter)
									{
										Filename = szCaptureFilePath;
										Index = Filename.rfind(_T('.'));
										if (basic_string<TCHAR>::npos != Index)
										{
											Filename.erase(Index);
										}
										Filename.append(TEXT(".wav"));

										hr = pIFileSinkFilter->SetFileName(CT2OLE(Filename.c_str()), NULL);
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
		hr = E_INVALIDARG;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// DestroySourceGraph
// Remove all filters from the source side graph being careful to release and
// associated resources.
HRESULT CDecklinkIngestDlg::DestroySourceGraph(void)
{
	HRESULT hr = S_OK;

	CComQIPtr<IMediaControl, &IID_IMediaControl> pIMediaControl = m_pSourceGraph;
	if (pIMediaControl)
	{
		// Stop playback.
		pIMediaControl->Stop();
	}

	// As the sources are to be removed, release the outstanding references on them.
	m_pIGMFBridgeController = NULL;
	m_pIExtTransport = (IAMExtTransport*)NULL;
	m_pVideoDevice = m_pAudioDevice = NULL;
	
	if (m_pSourceGraph)
	{
		CComPtr<IEnumFilters> pEnum = NULL;
		hr = m_pSourceGraph->EnumFilters(&pEnum);
		if (SUCCEEDED(hr))
		{
			IBaseFilter* pFilter = NULL;
			while (S_OK == pEnum->Next(1, &pFilter, NULL))
			{
				CDSUtils::DisconnectPins(m_pSourceGraph, pFilter);

				FILTER_INFO filterInfo = {0};
				hr = pFilter->QueryFilterInfo(&filterInfo);
				if (SUCCEEDED(hr))
				{
					SAFE_RELEASE(filterInfo.pGraph);
					
					if ((NULL == wcsstr(filterInfo.achName, SMARTT_NAME)) && (NULL == wcsstr(filterInfo.achName, VIDEORENDERER_NAME)) &&
						(NULL == wcsstr(filterInfo.achName, SAMPLEGRABBERSTILLS_NAME)) && (NULL == wcsstr(filterInfo.achName, SAMPLEGRABBERTIMECODE_NAME)) &&
						(NULL == wcsstr(filterInfo.achName, NULLRENDERER_NAME)))
					{
						hr = m_pSourceGraph->RemoveFilter(pFilter);
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
// DestroySinkGraph
// Remove all filters from the sink side graph.
HRESULT CDecklinkIngestDlg::DestroySinkGraph(void)
{
	HRESULT hr = S_OK;

	CComQIPtr<IMediaControl, &IID_IMediaControl> pIMediaControl = m_pSinkGraph;
	if (pIMediaControl)
	{
		// Stop playback.
		pIMediaControl->Stop();
	}
	
	if (m_pSinkGraph)
	{
		CComPtr<IEnumFilters> pEnum = NULL;
		hr = m_pSinkGraph->EnumFilters(&pEnum);
		if (SUCCEEDED(hr))
		{
			IBaseFilter* pFilter = NULL;
			while (S_OK == pEnum->Next(1, &pFilter, NULL))
			{
				CDSUtils::DisconnectPins(m_pSinkGraph, pFilter);

				FILTER_INFO filterInfo = {0};
				hr = pFilter->QueryFilterInfo(&filterInfo);
				if (SUCCEEDED(hr))
				{
					SAFE_RELEASE(filterInfo.pGraph);
					
					hr = m_pSinkGraph->RemoveFilter(pFilter);
					if (SUCCEEDED(hr))
					{
						hr = pEnum->Reset();
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
// StartCapture
//
HRESULT CDecklinkIngestDlg::StartCapture(void)
{
	HRESULT hr = S_OK;
	TRACE(TEXT("CDecklinkIngestDlg::StartCapture()\r\n"));

	CComQIPtr<IMediaControl, &IID_IMediaControl> pIMediaControl = m_pSinkGraph;
	if (pIMediaControl)
	{
		// Run the capture sink graph.
		hr = pIMediaControl->Run();
		TRACE(TEXT("CDecklinkIngestDlg::StartCapture() - Running sink graph.\r\n"));

		// Close the bridge.
		if (m_pIGMFBridgeController)
		{
			hr = m_pIGMFBridgeController->BridgeGraphs(m_pBridgeSink, m_pBridgeSource);
			TRACE(TEXT("CDecklinkIngestDlg::StartCapture() - Closing the bridge.\r\n"));
		}
	}

	return hr;
}

//-----------------------------------------------------------------------------
// StopCapture
//
HRESULT CDecklinkIngestDlg::StopCapture(void)
{
	HRESULT hr = S_OK;
	TRACE(TEXT("CDecklinkIngestDlg::StopCapture()\r\n"));
	// Open the bridge.
	if (m_pIGMFBridgeController)
	{
		TRACE(TEXT("CDecklinkIngestDlg::StopCapture() - Opening the bridge.\r\n"));
		hr = m_pIGMFBridgeController->BridgeGraphs(NULL, NULL);
	}

	// Stop the capture sink graph so that the file writing can be finalised.
	CComQIPtr<IMediaControl, &IID_IMediaControl> pIMediaControl = m_pSinkGraph;
	if (pIMediaControl)
	{
		TRACE(TEXT("CDecklinkIngestDlg::StopCapture() - Stopping sink graph.\r\n"));
		hr = pIMediaControl->Stop();
	}

	// Stop the capture streams.
	hr = ConfigureSourceGraph(NULL, NULL);

	return hr;
}

//-----------------------------------------------------------------------------
// EnableSampleGrabberCallback
// Enable or disable the sample grabber used for capturing image stills.
HRESULT CDecklinkIngestDlg::EnableSampleGrabberCallback(bool bEnable)
{
	HRESULT hr = S_OK;
	
	if (m_pISampleGrabberStills)
	{
		m_pISampleGrabberStills->SetCallback(bEnable ? m_pStillGrabber : NULL, 0);
	}
	else
	{
		hr = E_POINTER;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// EnableWindows
// Sets the state of a group of controls.
void CDecklinkIngestDlg::EnableWindows(const int* pCtrlIDs, int cCtrlIDs, BOOL bEnable)
{
	if (pCtrlIDs)
	{
		for (int Ctrl=0; Ctrl<cCtrlIDs; ++Ctrl)
		{
			CWnd* pWnd = GetDlgItem(pCtrlIDs[Ctrl]);
			if (pWnd)
			{
				pWnd->EnableWindow(bEnable);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// QueryRegistry
// retrieve previous media formats from registry
void CDecklinkIngestDlg::QueryRegistry(void)
{
	if (ERROR_SUCCESS == m_RegUtils.Open(TEXT("DecklinkIngestSample")))
	{
		TCHAR szBuffer[MAX_PATH];
		ZeroMemory(szBuffer, sizeof(szBuffer));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.GetString(TEXT("CaptureFilePath"), reinterpret_cast<LPBYTE>(szBuffer), sizeof(szBuffer)));
		m_CaptureFilePath = szBuffer;

		ZeroMemory(szBuffer, sizeof(szBuffer));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.GetString(TEXT("CaptureFilename"), reinterpret_cast<LPBYTE>(szBuffer), sizeof(szBuffer)));
		m_CaptureFilename = szBuffer;

		AM_MEDIA_TYPE amt;
		if (ERROR_SUCCESS == m_RegUtils.GetBinary(TEXT("VideoMediaType"), reinterpret_cast<LPBYTE>(&amt), sizeof(amt)))
		{
			VIDEOINFOHEADER vih = {0};
			if (ERROR_SUCCESS == m_RegUtils.GetBinary(TEXT("VideoFormat"), reinterpret_cast<LPBYTE>(&vih), sizeof(vih)))
			{
				amt.cbFormat = sizeof(vih);
				amt.pbFormat = reinterpret_cast<BYTE*>(&vih);
				m_VideoFormat.Set(amt);
			}
		}

		if (ERROR_SUCCESS == m_RegUtils.GetBinary(TEXT("AudioMediaType"), reinterpret_cast<LPBYTE>(&amt), sizeof(amt)))
		{
			WAVEFORMATEX wfex = {0};
			if (ERROR_SUCCESS == m_RegUtils.GetBinary(TEXT("AudioFormat"), reinterpret_cast<LPBYTE>(&wfex), sizeof(wfex)))
			{
				amt.cbFormat = sizeof(wfex);
				amt.pbFormat = reinterpret_cast<BYTE*>(&wfex);
				m_AudioFormat.Set(amt);
			}
		}

		EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.GetBinary(TEXT("AudioMute"), reinterpret_cast<LPBYTE>(&m_bMuteAudio), sizeof(m_bMuteAudio)));
		EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.GetBinary(TEXT("VideoCompressor"), reinterpret_cast<LPBYTE>(&m_Compression), sizeof(m_Compression)));
		
		EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.GetBinary(TEXT("VideoCaptureDevice"), reinterpret_cast<LPBYTE>(&m_Device), sizeof(m_Device)));
		if (CDecklinkIngestDlg::MAXDEVICES <= m_Device)
		{
			m_Device = 0;
		}
	}
	else
	{
		// create the key and registry values
		if (ERROR_SUCCESS == m_RegUtils.Create(TEXT("DecklinkIngestSample")))
		{
			SavePreferencesToRegistry();
			EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("VideoCaptureDevice"), reinterpret_cast<LPBYTE>(&m_Device), sizeof(m_Device)));
		}
	}
}

//-----------------------------------------------------------------------------
// SavePreferencesToRegistry
// retrieve previous media formats from registry
void CDecklinkIngestDlg::SavePreferencesToRegistry(void)
{
	EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetString(TEXT("CaptureFilePath"), reinterpret_cast<const BYTE*>(m_CaptureFilePath.c_str()), (DWORD)m_CaptureFilePath.length()));
	EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetString(TEXT("CaptureFilename"), reinterpret_cast<const BYTE*>(m_CaptureFilename.c_str()), (DWORD)m_CaptureFilename.length()));

	AM_MEDIA_TYPE amt = m_VideoFormat;
	amt.cbFormat = 0;
	amt.pbFormat = NULL;
	if (ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("VideoMediaType"), reinterpret_cast<LPBYTE>(&amt), sizeof(amt)))
	{
		EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("VideoFormat"), m_VideoFormat.Format(), sizeof(VIDEOINFOHEADER)));
	}

	amt = m_AudioFormat;
	amt.cbFormat = 0;
	amt.pbFormat = NULL;
	if (ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("AudioMediaType"), reinterpret_cast<LPBYTE>(&amt), sizeof(amt)))
	{
		EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("AudioFormat"), m_AudioFormat.Format(), m_AudioFormat.FormatLength()));
	}

	EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("AudioMute"), reinterpret_cast<LPBYTE>(&m_bMuteAudio), sizeof(m_bMuteAudio)));
	EXECUTE_ASSERT(ERROR_SUCCESS == m_RegUtils.SetBinary(TEXT("VideoCompressor"), reinterpret_cast<LPBYTE>(&m_Compression), sizeof(m_Compression)));
}

//-----------------------------------------------------------------------------
// SetFileExtension
//
void CDecklinkIngestDlg::SetFileExtension(basic_string<TCHAR>& Filename)
{
	basic_string<TCHAR>::size_type Index = Filename.rfind(_T('.'));
	if (basic_string<TCHAR>::npos != Index)
	{
		Filename.erase(Index);
	}

	if (ENC_WM == m_Compression)
	{
		m_CaptureFileExtension = TEXT(".wmv");
	}
	else if (ENC_SEQ_CIN == m_Compression)
	{
		m_CaptureFileExtension = TEXT(".cin");
	}
	else if (ENC_SEQ_DPX == m_Compression)
	{
		m_CaptureFileExtension = TEXT(".dpx");
	}
	else if (ENC_SEQ_TGA == m_Compression)
	{
		m_CaptureFileExtension = TEXT(".tga");
	}
	else if (ENC_SEQ_BMP == m_Compression)
	{
		m_CaptureFileExtension = TEXT(".bmp");
	}
	else
	{
		m_CaptureFileExtension = TEXT(".avi");
	}
}

//-----------------------------------------------------------------------------
// CreateStatusBarControl
//
void CDecklinkIngestDlg::CreateStatusBarControl(void)
{
	// Create and configure the status bar control.
	CRect Rect;
	GetWindowRect(&Rect);
	SetWindowPos(NULL, Rect.left, Rect.top, Rect.right - Rect.left, (Rect.bottom - Rect.top) + 25, 0);
	VERIFY(m_StatusBarCtrl.Create(WS_CHILD | WS_VISIBLE | CCS_BOTTOM, CRect(0, 0, 0, 0), this, IDC_STATUS_BAR));
	m_StatusBarCtrl.GetClientRect(&Rect);
	int Widths[] = { 350, -1 };
	VERIFY(m_StatusBarCtrl.SetParts(SIZEOF_ARRAY(Widths), Widths));

	// The idea is to have the progress control sandwiched between two timecodes,
	// one indicating the progess so far and the other the progress remaining.

	// First, determine the size of the text.
	CClientDC dc(&m_StatusBarCtrl);
	CFont* pOldFont = dc.SelectObject(m_StatusBarCtrl.GetFont());
	CSize LeftSide = dc.GetTextExtent(CString(TEXT("Capturing...00:00:00:00")));
	CSize Space = dc.GetTextExtent(CString(TEXT(" ")));
	dc.SelectObject(pOldFont);

	// Second, adjust the size of the progress control to fit between the timecode
	// strings indicating the current batch capture progress.
	m_StatusBarCtrl.GetRect(0, &Rect);
	Rect.DeflateRect(3, 3);
	Rect.left += LeftSide.cx;
	Rect.right = Rect.left + (51 * Space.cx);

	// Create and configure the status bar progess control.
	VERIFY(m_ProgressCtrl.Create(WS_CHILD, Rect, &m_StatusBarCtrl, IDC_STATUS_BAR_PROGRESS));
	m_ProgressCtrl.SetRange(0, 100);
	m_ProgressCtrl.SetStep(1);
}