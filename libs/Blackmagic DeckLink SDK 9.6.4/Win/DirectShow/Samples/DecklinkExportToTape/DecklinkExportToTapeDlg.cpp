//-----------------------------------------------------------------------------
// DecklinkExportToTapeDlg.cpp
//
// Desc: DirectShow export to tape sample
//
// Copyright (c) Blackmagic Design 2006. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DecklinkExportToTape.h"
#include "DecklinkExportToTapeDlg.h"
#include "Timecode.h"
#include "StaticURL.h"

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
// CDecklinkExportToTapeDlg dialog
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor
//
CDecklinkExportToTapeDlg::CDecklinkExportToTapeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecklinkExportToTapeDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-----------------------------------------------------------------------------
// DoDataExchange
//
void CDecklinkExportToTapeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_ADDFILE, m_BtnAddFile);
	DDX_Control(pDX, IDC_BUTTON_REMOVEFILE, m_BtnRemoveFile);
	DDX_Control(pDX, IDC_BUTTON_MOVEFILEUP, m_BtnMoveFileUp);
	DDX_Control(pDX, IDC_BUTTON_MOVEFILEDOWN, m_BtnMoveFileDown);
	DDX_Control(pDX, IDC_BUTTON_SETINPOINT, m_BtnSetInpoint);
	DDX_Control(pDX, IDC_BUTTON_SETOUTPOINT, m_BtnSetOutpoint);
	DDX_Control(pDX, IDC_LIST_FILES, m_FileList);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_deviceCtrl);
	DDX_Control(pDX, IDC_STATIC_TIMECODE, m_TimecodeCtrl);
	DDX_Control(pDX, IDC_EDIT_INPOINT, m_InpointCtrl);
	DDX_Control(pDX, IDC_EDIT_OUTPOINT, m_OutpointCtrl);
	DDX_Control(pDX, IDC_STATIC_DURATION, m_DurationCtrl);
	DDX_Control(pDX, IDC_BUTTON_FREW, m_BtnFRew);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_BtnPlay);
	DDX_Control(pDX, IDC_BUTTON_FFWD, m_BtnFFwd);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_BtnStop);
	DDX_Control(pDX, IDC_BUTTON_EJECT, m_BtnEjct);
	DDX_Control(pDX, IDC_EDIT_OFFSET, m_OffsetCtrl);
}

//-----------------------------------------------------------------------------
//
//
BEGIN_MESSAGE_MAP(CDecklinkExportToTapeDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_ADDFILE, OnBnClickedButtonAddfile)
	ON_BN_CLICKED(IDC_BUTTON_REMOVEFILE, OnBnClickedButtonRemovefile)
	ON_BN_CLICKED(IDC_BUTTON_MOVEFILEUP, OnBnClickedButtonMovefileup)
	ON_BN_CLICKED(IDC_BUTTON_MOVEFILEDOWN, OnBnClickedButtonMovefiledown)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_SETINPOINT, OnBnClickedButtonSetinpoint)
	ON_BN_CLICKED(IDC_BUTTON_SETOUTPOINT, OnBnClickedButtonSetoutpoint)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnCbnSelchangeComboDevice)
	ON_BN_CLICKED(IDC_BUTTON_ABORT, OnBnClickedButtonAbort)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_FREW, OnBnClickedButtonFrew)
	ON_BN_CLICKED(IDC_BUTTON_FFWD, OnBnClickedButtonFfwd)
	ON_BN_CLICKED(IDC_BUTTON_EJECT, OnBnClickedButtonEject)
	ON_NOTIFY(LVN_INSERTITEM, IDC_LIST_FILES, OnLvnInsertitemListFiles)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// OnInitDialog
//
BOOL CDecklinkExportToTapeDlg::OnInitDialog()
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
	m_hThread = NULL;
	m_hExitEvent = NULL;

	// load bitmaps for file buttons
	m_BtnAddFile.LoadBitmaps(IDB_BITMAP_ADDFILEU, IDB_BITMAP_ADDFILED, IDB_BITMAP_ADDFILEF, IDB_BITMAP_ADDFILEX);
	m_BtnRemoveFile.LoadBitmaps(IDB_BITMAP_REMOVEFILEU, IDB_BITMAP_REMOVEFILED, IDB_BITMAP_REMOVEFILEF, IDB_BITMAP_REMOVEFILEX);
	m_BtnMoveFileUp.LoadBitmaps(IDB_BITMAP_MOVEUPFILEU, IDB_BITMAP_MOVEUPFILED, IDB_BITMAP_MOVEUPFILEF, IDB_BITMAP_MOVEUPFILEX);
	m_BtnMoveFileDown.LoadBitmaps(IDB_BITMAP_MOVEDOWNFILEU, IDB_BITMAP_MOVEDOWNFILED, IDB_BITMAP_MOVEDOWNFILEF, IDB_BITMAP_MOVEDOWNFILEX);
	m_BtnSetInpoint.LoadBitmaps(IDB_BITMAP_SETINPOINTU, IDB_BITMAP_SETINPOINTD, IDB_BITMAP_SETINPOINTF, IDB_BITMAP_SETINPOINTX);
	m_BtnSetOutpoint.LoadBitmaps(IDB_BITMAP_SETOUTPOINTU, IDB_BITMAP_SETOUTPOINTD, IDB_BITMAP_SETOUTPOINTF, IDB_BITMAP_SETOUTPOINTX);

	// set the columns in the list control
	int ColumnWidth = m_FileList.GetStringWidth(TEXT("00:00:00:00")) + 12;
	m_FileList.InsertColumn(0, TEXT("Clip Name"), LVCFMT_CENTER, 100);
	m_FileList.InsertColumn(1, TEXT("Frame Rate"), LVCFMT_CENTER, m_FileList.GetStringWidth(TEXT("Frame Rate")) + 16);
	m_FileList.InsertColumn(2, TEXT("Start"), LVCFMT_CENTER, ColumnWidth);
	m_FileList.InsertColumn(3, TEXT("End"), LVCFMT_CENTER, ColumnWidth);
	m_FileList.InsertColumn(4, TEXT("Duration"), LVCFMT_CENTER, ColumnWidth);
	m_FileList.InsertColumn(5, TEXT("In-point"), LVCFMT_CENTER, ColumnWidth);
	m_FileList.InsertColumn(6, TEXT("Out-point"), LVCFMT_CENTER, ColumnWidth);
	m_FileList.InsertColumn(7, TEXT("Clip Path"), LVCFMT_LEFT, 100);

	m_deviceCtrl.AddString(TEXT("<No Devices>"));

	m_InpointCtrl.SetWindowText(TEXT("00:00:00:00"));
	m_OutpointCtrl.SetWindowText(TEXT("00:00:00:00"));
	m_OffsetCtrl.SetWindowText(TEXT("0"));

	// load bitmaps for the deck controls
	m_BtnFRew.LoadBitmaps(IDB_BITMAP_FREWU, IDB_BITMAP_FREWD, IDB_BITMAP_FREWF, IDB_BITMAP_FREWX);
	m_BtnPlay.LoadBitmaps(IDB_BITMAP_PLAYU, IDB_BITMAP_PLAYD, IDB_BITMAP_PLAYF, IDB_BITMAP_PLAYX);
	m_BtnFFwd.LoadBitmaps(IDB_BITMAP_FFWDU, IDB_BITMAP_FFWDD, IDB_BITMAP_FFWDF, IDB_BITMAP_FFWDX);
	m_BtnStop.LoadBitmaps(IDB_BITMAP_STOPU, IDB_BITMAP_STOPD, IDB_BITMAP_STOPF, IDB_BITMAP_STOPX);
	m_BtnEjct.LoadBitmaps(IDB_BITMAP_EJECTU, IDB_BITMAP_EJECTD, IDB_BITMAP_EJECTF, IDB_BITMAP_EJECTX);

	// initialise the project media type
	WORD width = 720, height = 576;
	m_mtProject.InitMediaType();
	m_mtProject.majortype = MEDIATYPE_Video;
	m_mtProject.subtype = MEDIASUBTYPE_UYVY;
	m_mtProject.lSampleSize = width * height * 2;
	m_mtProject.formattype = FORMAT_VideoInfo;
	if (m_mtProject.AllocFormatBuffer(sizeof(VIDEOINFOHEADER)))
	{
		m_pvih = reinterpret_cast<VIDEOINFOHEADER*>(m_mtProject.pbFormat);
		ZeroMemory(m_pvih, m_mtProject.cbFormat);
		m_pvih->AvgTimePerFrame = UNITS / 25;
		m_pvih->dwBitRate = m_mtProject.lSampleSize * (DWORD)((float)UNITS / m_pvih->AvgTimePerFrame) * 8;
		m_pvih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_pvih->bmiHeader.biWidth = width;
		m_pvih->bmiHeader.biHeight = height;
		m_pvih->bmiHeader.biPlanes = 1;
		m_pvih->bmiHeader.biBitCount = 16;
		m_pvih->bmiHeader.biCompression = 'YVYU';
		m_pvih->bmiHeader.biSizeImage = m_mtProject.lSampleSize;
	}

	m_bExport = false;

	EnableControls(FALSE);
	EnumerateDevices();

	// create a skeleton playback graph
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&m_pGraph));
	if (SUCCEEDED(hr))
	{
#ifdef _DEBUG
		hr = CDSUtils::AddGraphToRot(m_pGraph, &m_ROTRegister);
#endif
		if (SUCCEEDED(hr))
		{
			m_pIMediaEvent = m_pGraph;
			m_pIMediaControl = m_pGraph;
			m_pIMediaSeeking = m_pGraph;

			m_pIMediaFilter = m_pGraph;
			ASSERT(m_pIMediaFilter);

			hr = CreateBaseGraph();
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-----------------------------------------------------------------------------
// DestroyWindow
// Called when the window is being destroyed, clean up and free all resources.
BOOL CDecklinkExportToTapeDlg::DestroyWindow()
{
#ifdef _DEBUG
	CDSUtils::RemoveGraphFromRot(m_ROTRegister);
#endif
	DestroyGraph(true);

	// release the device names attached to the item's data
	while (m_deviceCtrl.GetCount())
	{
		PWSTR pName = (PWSTR)m_deviceCtrl.GetItemData(0);
		delete [] pName;
		m_deviceCtrl.DeleteString(0);
	}

	int cItems = m_FileList.GetItemCount();
	for (int item=0; item<cItems; ++item)
	{
		CBaseClip* pClip = reinterpret_cast<CBaseClip*>(m_FileList.GetItemData(item));
		if (pClip)
		{
			delete pClip;
		}
	}

	m_FileList.DeleteAllItems();

	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
// OnSysCommand
//
void CDecklinkExportToTapeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CDecklinkExportToTapeDlg::OnPaint() 
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
HCURSOR CDecklinkExportToTapeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonAddfile
// Create a file browser dialog to retrieve the name of a clip to add to the filelist.
void CDecklinkExportToTapeDlg::OnBnClickedButtonAddfile()
{
	char BASED_CODE szFilters[] = "Windows Media Files|*.avi;*.mov;*.mpg;*.wmv|All Files (*.*)|*.*||";

	CFileDialog FileDlg(TRUE, "Windows Media Files", NULL, OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, szFilters, this);

	if (FileDlg.DoModal() == IDOK)
	{
		list< basic_string<TCHAR> > FileList;

		if (FileDlg.m_pOFN->nFileOffset && (_T('\0') == FileDlg.m_pOFN->lpstrFile[FileDlg.m_pOFN->nFileOffset - 1]))
		{
			// Multiple files have been selected so create a list of file names.
			basic_string<TCHAR> FilePath = FileDlg.m_pOFN->lpstrFile;
			FilePath.append(TEXT("\\"));
			
			size_t Offset = FileDlg.m_pOFN->nFileOffset;
			for (;;)
			{
				LPTSTR pszTemp = &FileDlg.m_pOFN->lpstrFile[Offset];
				size_t len = _tcslen(pszTemp);
				if (len)
				{
					FileList.push_back(FilePath + pszTemp);
					Offset += (len + 1);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			// Only one file has been selected.
			FileList.push_back(FileDlg.m_pOFN->lpstrFile);
		}
		
		try
		{
			for (list< basic_string<TCHAR> >::iterator it = FileList.begin(); it != FileList.end(); ++it)
			{
				m_FileList.AddItem(new CBaseClip(*it));
				
				// if there is at least one item in the list, enable the export and stop buttons
				if (1 == m_FileList.GetItemCount())
				{
					EnableControls(TRUE);
				}
			}
		}
		catch (std::bad_alloc)
		{
			// memory allocation error
		}
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonRemovefile
// Remove the selected file from the filelist.
void CDecklinkExportToTapeDlg::OnBnClickedButtonRemovefile()
{
	m_FileList.RemoveItem();

	// if there are no items in the list, disable the export and stop buttons
	if (0 == m_FileList.GetItemCount())
	{
		EnableControls(FALSE);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonMovefileup
// Move the selected file up in the filelist.
void CDecklinkExportToTapeDlg::OnBnClickedButtonMovefileup()
{
	m_FileList.MoveItemUp();
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonMovefiledown
// Move the selected file down in the filelist.
void CDecklinkExportToTapeDlg::OnBnClickedButtonMovefiledown()
{
	m_FileList.MoveItemDown();
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonSetinpoint
// Set the inpoint of the selected clip.
void CDecklinkExportToTapeDlg::OnBnClickedButtonSetinpoint()
{
	CString strTimecode;
	m_InpointCtrl.GetWindowText(strTimecode);
	int sel = m_FileList.GetSelectedItem();
	m_FileList.SetInpoint(sel, CTimecode((WORD)(UNITS / m_pvih->AvgTimePerFrame), true, strTimecode));
	// retrieve the corrected timecode
	CTimecode Timecode = m_FileList.Inpoint(sel);
	strTimecode = Timecode.TimecodeToString();
	m_InpointCtrl.SetWindowText(strTimecode);
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonSetoutpoint
// Set the outpoint of the selected clip.
void CDecklinkExportToTapeDlg::OnBnClickedButtonSetoutpoint()
{
	CString strTimecode;
	m_OutpointCtrl.GetWindowText(strTimecode);
	int sel = m_FileList.GetSelectedItem();
	m_FileList.SetOutpoint(sel, CTimecode((WORD)(UNITS / m_pvih->AvgTimePerFrame), true, strTimecode));
	// retrieve the corrected timecode
	CTimecode Timecode = m_FileList.Outpoint(sel);
	strTimecode = Timecode.TimecodeToString();
	m_OutpointCtrl.SetWindowText(strTimecode);
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboDevice
// Insert the new renderers into the playback graph.
void CDecklinkExportToTapeDlg::OnCbnSelchangeComboDevice()
{
	CreateBaseGraph();
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonPlay
//
void CDecklinkExportToTapeDlg::OnBnClickedButtonPlay()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_Mode(ED_MODE_PLAY);
	}

	m_pIMediaControl->Run();
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonStop
//
void CDecklinkExportToTapeDlg::OnBnClickedButtonStop()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_Mode(ED_MODE_STOP);
	}

	m_pIMediaControl->Stop();
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonFrew
//
void CDecklinkExportToTapeDlg::OnBnClickedButtonFrew()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_Mode(ED_MODE_REW);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonFfwd
//
void CDecklinkExportToTapeDlg::OnBnClickedButtonFfwd()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_Mode(ED_MODE_FF);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonEject
//
void CDecklinkExportToTapeDlg::OnBnClickedButtonEject()
{
	if (m_pIExtTransport)
	{
		m_pIExtTransport->put_MediaState(ED_MEDIA_UNLOAD);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonExport
// Start exporting to tape
void CDecklinkExportToTapeDlg::OnBnClickedButtonExport()
{
	CWnd* pWnd = GetDlgItem(IDC_BUTTON_ADDFILE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_REMOVEFILE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_MOVEFILEUP);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_MOVEFILEDOWN);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_SETINPOINT);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_SETOUTPOINT);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_COMBO_DEVICE);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_BUTTON_EXPORT);
	pWnd->EnableWindow(FALSE);

	m_FileList.SetSelectedItem(0);

	m_bExport = true;
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonAbort
// Stop exporting to tape
void CDecklinkExportToTapeDlg::OnBnClickedButtonAbort()
{
	CWnd* pWnd = GetDlgItem(IDC_BUTTON_ADDFILE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_REMOVEFILE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_MOVEFILEUP);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_MOVEFILEDOWN);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_SETINPOINT);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_SETOUTPOINT);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_COMBO_DEVICE);
	pWnd->EnableWindow(TRUE);
	pWnd = GetDlgItem(IDC_BUTTON_EXPORT);
	pWnd->EnableWindow(TRUE);

	m_bExport = false;

	OnBnClickedButtonStop();
}

//-----------------------------------------------------------------------------
// OnLvnInsertitemListFiles
// If an item has been inserted via drag and drop, update the UI.
void CDecklinkExportToTapeDlg::OnLvnInsertitemListFiles(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// if there is at least one item in the list, enable the export and stop buttons
	if (0 != m_FileList.GetItemCount())
	{
		EnableControls(TRUE);
	}

	*pResult = 0;
}

//-----------------------------------------------------------------------------
// EnableControls
//
void CDecklinkExportToTapeDlg::EnableControls(BOOL enable)
{
	CWnd* pWnd = GetDlgItem(IDC_BUTTON_REMOVEFILE);
	pWnd->EnableWindow(enable);
	pWnd = GetDlgItem(IDC_BUTTON_MOVEFILEUP);
	pWnd->EnableWindow(enable);
	pWnd = GetDlgItem(IDC_BUTTON_MOVEFILEDOWN);
	pWnd->EnableWindow(enable);
	pWnd = GetDlgItem(IDC_BUTTON_SETINPOINT);
	pWnd->EnableWindow(enable);
	pWnd = GetDlgItem(IDC_BUTTON_SETOUTPOINT);
	pWnd->EnableWindow(enable);
	pWnd = GetDlgItem(IDC_BUTTON_EXPORT);
	pWnd->EnableWindow(enable);
	pWnd = GetDlgItem(IDC_BUTTON_ABORT);
	pWnd->EnableWindow(enable);
}

//-----------------------------------------------------------------------------
// EnumerateDevices
// Find all the capture devices in the system
HRESULT CDecklinkExportToTapeDlg::EnumerateDevices(void)
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
			bool bDeviceFound = false;
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
						if (false == bDeviceFound)
						{
							m_deviceCtrl.ResetContent();
							bDeviceFound = true;
						}
						
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
// CreateThread
//
HRESULT CDecklinkExportToTapeDlg::CreateThreads(void)
{
	HRESULT hr = S_OK;

	m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, "Decklink Export to Tape Sample Event");
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
HRESULT CDecklinkExportToTapeDlg::DestroyThreads(void)
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
DWORD WINAPI CDecklinkExportToTapeDlg::ThreadProc(LPVOID lpParameter)
{
	ASSERT(lpParameter);
	CDecklinkExportToTapeDlg* pDlg = reinterpret_cast<CDecklinkExportToTapeDlg*>(lpParameter);
	return pDlg->Thread();
}

//-----------------------------------------------------------------------------
// Thread
// Actual class thread
DWORD CDecklinkExportToTapeDlg::Thread(void)
{
	HRESULT hr = S_OK;
	enum { MODE_NEWCLIP, MODE_PREROLLING, MODE_EXPORTING, MODE_PLAYING };
	int state = MODE_NEWCLIP, CurrentItem, FrameOffset = 0;
	CBaseClip* pClip = NULL;

	TIMECODE_SAMPLE TimecodeSample = {0};
	TimecodeSample.dwFlags = ED_DEVCAP_TIMECODE_READ;
	CTimecode Timecode = TimecodeSample.timecode;

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
				if (m_pIExtTransport && m_pITimecodeReader)
				{
					long Mode;
					if (S_OK == m_pIExtTransport->get_Mode(&Mode) && (ED_MODE_STOP != Mode))
					{
						if (S_OK == m_pITimecodeReader->GetTimecode(&TimecodeSample))
						{
							Timecode = TimecodeSample.timecode;
							UpdateTimecodeCtrl(Timecode);
						}
					}
				}

				if (m_bExport)
				{
					// attempt to use the DeckLink output for each clip in the file list
					switch (state)
					{
						case MODE_NEWCLIP:
							// setup a new clip, if transport is available preroll to the inpoint
							CurrentItem = m_FileList.GetSelectedItem();
							pClip = reinterpret_cast<CBaseClip*>(m_FileList.GetItemData(CurrentItem));
							if (pClip)
							{
								hr = CreateGraph(pClip->FilePath().c_str());	// build the playback graph
								if (SUCCEEDED(hr))
								{
									// update the inpoint and outpoint ctrls with the current clip
									m_InpointCtrl.SetWindowText(pClip->Inpoint().TimecodeToString());
									m_OutpointCtrl.SetWindowText(pClip->Outpoint().TimecodeToString());
									m_DurationCtrl.SetWindowText(pClip->Duration().TimecodeToString());
									// determine the deck specific frame offset
									TCHAR szOffset[16] = {0};
									m_OffsetCtrl.GetWindowText(szOffset, 16);
									FrameOffset = _tstoi(szOffset);

									hr = m_pIMediaControl->Pause();	// get the streams to allocate resources in preparation for playing as soon as the inpoint is reached
									if (SUCCEEDED(hr) && m_pIMediaSeeking)
									{
										// get the file IO subsytstem to cache the first few frames off disk
										GUID TimeFormat;
										hr = m_pIMediaSeeking->GetTimeFormat(&TimeFormat);
										if (SUCCEEDED(hr))
										{
											if (TimeFormat == TIME_FORMAT_MEDIA_TIME)
											{
												LONGLONG current = 0;
												hr = m_pIMediaSeeking->SetPositions(&current, AM_SEEKING_AbsolutePositioning, NULL, 0);
											}
										}
									}

									// if a transport control is present and functioning, preroll to the inpoint otherwise just play the clip
									hr = CheckTransport();
									if (S_OK == hr)
									{
										hr = CreateEditPropertySet(pClip->Inpoint(), pClip->Outpoint(), 100);
									}
								}

								if (SUCCEEDED(hr))
								{
									if (S_OK == hr)
									{
										// preroll the transport
										state = MODE_PREROLLING;
									}
									else
									{
										// no transport control so just play the list
										hr = m_pIMediaControl->Run();
										state = MODE_PLAYING;
									}
								}
								else
								{
									// a serious problem occurred building the playback graph or with the transport configuration
									OnBnClickedButtonAbort();
								}
							}
							else
							{
								// no more clips
								OnBnClickedButtonAbort();
							}
							break;

						case MODE_PREROLLING:
							// at this point an insert edit has begun and the transport is seeking to the inpoint preroll,
							// wait for the deck to hit the inpoint before starting graph playback
							if (SUCCEEDED(m_pITimecodeReader->GetTimecode(&TimecodeSample)))
							{
								Timecode = TimecodeSample.timecode;
								if (Timecode == (pClip->Inpoint() + FrameOffset))
								{
									hr = m_pIMediaControl->Run();
									state = MODE_EXPORTING;
								}
								else
								{
									UpdateTimecodeCtrl(Timecode);
								}
							}
							break;
							
						case MODE_EXPORTING:
							// wait for the outpoint of the clip
							// TODO: Could set the event in the edit property and wait for it here?
							if (SUCCEEDED(m_pITimecodeReader->GetTimecode(&TimecodeSample)))
							{
								Timecode = TimecodeSample.timecode;
								if (Timecode == pClip->Outpoint())
								{
									hr = m_pIMediaControl->Stop();
									state = MODE_NEWCLIP;
									if (0 == m_FileList.SetSelectedItem(++CurrentItem))
									{
										// no more clips
										OnBnClickedButtonAbort();
									}
								}
								else
								{
									UpdateTimecodeCtrl(Timecode);
								}
							}
							break;
							
						case MODE_PLAYING:
							// Wait for the graph to issue EC_COMPLETE before continuing to the next clip
							long lEventCode;
							LONG_PTR lParam1, lParam2;
							HRESULT hr;
							while (hr = m_pIMediaEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0), SUCCEEDED(hr))
							{
								hr = m_pIMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);

								if (EC_COMPLETE == lEventCode) 
								{ 
									m_pIMediaControl->Stop();
									state = MODE_NEWCLIP;
									if (0 == m_FileList.SetSelectedItem(++CurrentItem))
									{
										// no more clips
										OnBnClickedButtonAbort();
									}
									break;
								} 
							}
							
							if (m_pIMediaSeeking)
							{
								// Use the IMediaSeeking interface to update the timecode control
								LONGLONG duration = 0;
								hr = m_pIMediaSeeking->GetDuration(&duration);
								if (SUCCEEDED(hr))
								{
									LONGLONG current = 0;
									hr = m_pIMediaSeeking->GetCurrentPosition(&current);
									if (SUCCEEDED(hr))
									{
										if (0 == duration)
										{
											// avoid div zero error
											duration = 1;
										}

										DWORD FrameCount = (DWORD)(pClip->Inpoint().GetFrameCount() + ((double)current / UNITS * pClip->Inpoint().GetFrameRate()));
										CTimecode Timecode(pClip->Inpoint().GetFrameRate(), true, FrameCount);
										UpdateTimecodeCtrl(Timecode);
									}
								}

							}
							break;
					}
				}
				else
				{
					if (MODE_NEWCLIP != state)
					{
						state = MODE_NEWCLIP;
					}
				}
				break;		
		}	
	}

	return 0;
}

//-----------------------------------------------------------------------------
// CreateBaseGraph
// Build a graph with just the renderers.
HRESULT CDecklinkExportToTapeDlg::CreateBaseGraph(void)
{
	HRESULT hr = DestroyGraph(true);
	if (SUCCEEDED(hr))
	{
		hr = CreateThreads();
		if (SUCCEEDED(hr))
		{
			// add the video renderer
			PWSTR pNameVideo = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());
			hr = CDSUtils::AddFilter2(m_pGraph, CLSID_TransmitCategory, pNameVideo, &m_pVideoRenderer);
			if (SUCCEEDED(hr))
			{
				m_pIExtTransport = m_pVideoRenderer;	// QI renderer for the transport control interface
				m_pITimecodeReader = m_pVideoRenderer;	// QI renderer for the time code reader interface

				// set the graph reference clock to the hardware derived clock on the DeckLink card
				if (m_pIMediaFilter)
				{
					m_pIReferenceClock = m_pVideoRenderer;
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
				hr = CDSUtils::AddFilter2(m_pGraph, CLSID_AudioRendererCategory, pNameAudio, &m_pAudioRenderer);
				delete [] pNameAudio;
				if (SUCCEEDED(hr))
				{
					CComPtr<IBaseFilter> pPushSource = NULL;
					hr = CDSUtils::AddFilter(m_pGraph, CLSID_DecklinkPushSource, L"Decklink Push Source", &pPushSource);
					if (SUCCEEDED(hr))
					{
						// to read timecode before exporting, the video renderer must be connected to determine the frame rate
						CComPtr<IAMStreamConfig> pIAMStreamConfig = NULL;
						hr = CDSUtils::FindPinInterface(pPushSource, L"Video", IID_IAMStreamConfig, reinterpret_cast<void**>(&pIAMStreamConfig));
						if (pIAMStreamConfig)
						{
							hr = pIAMStreamConfig->SetFormat(&m_mtProject);
							if (SUCCEEDED(hr))
							{
								hr = CDSUtils::ConnectFilters(m_pGraph, pPushSource, NULL, m_pVideoRenderer, NULL);
								if (SUCCEEDED(hr))
								{
									if (m_pITimecodeReader)
									{
										Sleep(50);	// bit of a fudge, have to give the deck time to respond
										TIMECODE_SAMPLE tcSample = {0};
										tcSample.dwFlags = ED_DEVCAP_TIMECODE_READ;
										if (SUCCEEDED(m_pITimecodeReader->GetTimecode(&tcSample)))
										{
											CTimecode timecode = tcSample.timecode;
											m_TimecodeCtrl.SetWindowText(timecode.TimecodeToString());
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
// CreateGraph
// Build a playback graph.
HRESULT CDecklinkExportToTapeDlg::CreateGraph(const TCHAR* pFilename)
{
	HRESULT hr = S_OK;
	if (pFilename && m_pVideoRenderer)
	{
		hr = DestroyGraph(false);

		CComPtr<IBaseFilter> pSource = NULL;
		USES_CONVERSION;	// for T2W macro
		hr = m_pGraph->AddSourceFilter(T2W(pFilename), L"Video Source Filter", &pSource);
		if (SUCCEEDED(hr))
		{
			hr = CDSUtils::ConnectFilters(m_pGraph, pSource, NULL, m_pVideoRenderer, NULL);
			if (SUCCEEDED(hr))
			{
				// connect audio renderer, first assume the file source has both audio and video pins, e.g. the .asf reader
				hr = CDSUtils::ConnectFilters(m_pGraph, pSource, m_pAudioRenderer, &MEDIATYPE_Audio);
				if (FAILED(hr))
				{
					// try the output pins on the filter downstream of the source (typically a parser or splitter)
					CComPtr<IEnumPins> pIEnumPins;
					hr = pSource->EnumPins(&pIEnumPins);
					if (SUCCEEDED(hr))
					{
						// enumerate the output pins on the source filter for a connected video pin
						IPin* pIPin = NULL;
						while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
						{
							PIN_INFO pi = {0};
							if (SUCCEEDED(pIPin->QueryPinInfo(&pi)) && (PINDIR_OUTPUT == pi.dir))
							{
								IPin* pIPinDownstream = NULL;
								if (SUCCEEDED(pIPin->ConnectedTo(&pIPinDownstream)))
								{
									// attempt to locate an unconnected audio output pin the the filter downstream of the source filter
									PIN_INFO piDownstream = {0};
									if (SUCCEEDED(pIPinDownstream->QueryPinInfo(&piDownstream)) && piDownstream.pFilter)
									{
										CDSUtils::ConnectFilters(m_pGraph, piDownstream.pFilter, NULL, m_pAudioRenderer, NULL);
									}
									SAFE_RELEASE(piDownstream.pFilter);
									SAFE_RELEASE(pIPinDownstream);
								}
							}
							SAFE_RELEASE(pi.pFilter);
							SAFE_RELEASE(pIPin);
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
// Remove all filters from the graph apart from the renderers
HRESULT CDecklinkExportToTapeDlg::DestroyGraph(bool bRemoveRenderers)
{
	HRESULT hr = S_OK;

	if (m_pIMediaControl)
	{
		// stop playback
		m_pIMediaControl->Stop();
	}
	
	if (m_pGraph)
	{
		if (bRemoveRenderers)
		{
			if (m_pIMediaFilter)
			{
				m_pIMediaFilter->SetSyncSource(NULL);
			}

			DestroyThreads();	// destroy the thread which uses renderer interfaces that are about to be released
			// as the renderers are to be removed, release the outstanding references on them
			m_pIExtTransport = (IAMExtTransport*)NULL;
			m_pITimecodeReader = (IAMTimecodeReader*)NULL;
			m_pIReferenceClock = (IReferenceClock*)NULL;
			m_pVideoRenderer = NULL;

			m_pAudioRenderer = NULL;
		}

		PWSTR pNameVideo = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());
		size_t len = wcslen(pNameVideo) + 1;
		PWSTR pNameAudio = new WCHAR [len];
		wcsncpy(pNameAudio, pNameVideo, len);
		PWSTR pTemp = wcsstr(pNameAudio, L"Video");
		if (pTemp)
		{
			wcsncpy(pTemp, L"Audio", 5);
		}

		CComPtr<IEnumFilters> pEnum = NULL;
		hr = m_pGraph->EnumFilters(&pEnum);
		if (SUCCEEDED(hr))
		{
			IBaseFilter* pFilter = NULL;
			while (S_OK == pEnum->Next(1, &pFilter, NULL))
			{
				if (bRemoveRenderers)
				{
					// remove all filters
					hr = m_pGraph->RemoveFilter(pFilter);
					if (SUCCEEDED(hr))
					{
						hr = pEnum->Reset();
					}
				}
				else
				{
					// remove all but the renderers
					FILTER_INFO filterInfo = {0};
					hr = pFilter->QueryFilterInfo(&filterInfo);
					if (SUCCEEDED(hr))
					{
						SAFE_RELEASE(filterInfo.pGraph);
						
						if ((NULL == wcsstr(filterInfo.achName, pNameVideo)) && (NULL == wcsstr(filterInfo.achName, pNameAudio)))
						{
							hr = m_pGraph->RemoveFilter(pFilter);
							if (SUCCEEDED(hr))
							{
								hr = pEnum->Reset();
							}
						}
					}
				}
				SAFE_RELEASE(pFilter);
			}
		}

		SAFE_DELETE(pNameAudio);
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CheckTransport
// Make some rudimentary checks of the transport, such as are there comms (is
// the RS232 connected), is it in remote mode, is the tape loaded and is record
// inhibit disabled.
HRESULT CDecklinkExportToTapeDlg::CheckTransport(void)
{
	HRESULT hr = S_OK;
	
	if (m_pIExtTransport)
	{
		// test to see if there are comms with the deck
		long state;
		hr = m_pIExtTransport->get_LocalControl(&state);
		if (FAILED(hr))
		{
			int ret = IDTRYAGAIN;
			while ((IDTRYAGAIN == ret) && FAILED(hr))
			{
				ret = MessageBox(TEXT("Unable to communicate with device.\r\n\r\nPlease check connections and try again or cancel to abort."), TEXT("Device Control"), MB_ICONEXCLAMATION | MB_CANCELTRYCONTINUE);
				hr = m_pIExtTransport->get_LocalControl(&state);
			}

			if (IDCONTINUE == ret)
			{
				// disable the transport control checks
				m_pIExtTransport = (IAMExtTransport*)NULL;
				m_pITimecodeReader = (IAMTimecodeReader*)NULL;
				hr = S_FALSE;
			}
			else
			{
				hr = E_FAIL;
			}
		}

		// if there are comms with the deck, test to see if the deck is in Local control
		if ((S_OK == hr) && (OATRUE == state))
		{
			int ret = IDRETRY;
			while ((IDRETRY == ret) && SUCCEEDED(hr) && (OATRUE == state))
			{
				ret = MessageBox(TEXT("Device is in LOCAL control.\r\n\r\nPlease check and try again or cancel to abort."), TEXT("Device Control"), MB_ICONEXCLAMATION | MB_RETRYCANCEL);
				hr = m_pIExtTransport->get_LocalControl(&state);
			}
			
			if ((IDRETRY != ret) || FAILED(hr))
			{
				hr = E_FAIL;
			}			
		}

		if (S_OK == hr)
		{
			// if the deck is in remote control, test to see if a tape is loaded
			hr = m_pIExtTransport->get_MediaState(&state);
			if (SUCCEEDED(hr) && (ED_MEDIA_UNLOAD == state))
			{
				int ret = IDRETRY;
				while ((IDRETRY == ret) && SUCCEEDED(hr) && (ED_MEDIA_UNLOAD == state))
				{
					ret = MessageBox(TEXT("Device has no tape.\r\n\r\nPlease check and try again or cancel to abort."), TEXT("Device Control"), MB_ICONEXCLAMATION | MB_RETRYCANCEL);
					hr = m_pIExtTransport->get_MediaState(&state);
				}

				if ((IDRETRY != ret) || FAILED(hr))
				{
					hr = E_FAIL;
				}
			}
		}

		if (S_OK == hr)
		{
			// if a tape is loaded, test to see if the record inhibit is enabled
			hr = m_pIExtTransport->GetStatus(ED_RECORD_INHIBIT, &state);
			if (SUCCEEDED(hr) && (OATRUE == state))
			{
				int ret = IDRETRY;
				while ((IDRETRY == ret) && SUCCEEDED(hr) && (OATRUE == state))
				{
					ret = MessageBox(TEXT("Record Inhibit is enabled on device.\r\n\r\nPlease check and try again or cancel to abort."), TEXT("Device Control"), MB_ICONEXCLAMATION | MB_RETRYCANCEL);
					hr = m_pIExtTransport->GetStatus(ED_RECORD_INHIBIT, &state);
				}
				
				if ((IDRETRY != ret) || FAILED(hr))
				{
					hr = E_FAIL;
				}
			}
		}
	}
	else
	{
		hr = S_FALSE;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// CreateEditPropertySet
// Setup the transport to perform an Insert Edit, if successful this method returns
// with the transport seeking to the inpoint preroll.
// RE: Microsoft DirectShow SDK documentation - IAMExtTransport::SetEditProperty
// TODO: Work out how to set the preroll for the deck (currently defaults to 5 secs)
HRESULT CDecklinkExportToTapeDlg::CreateEditPropertySet(CTimecode& Inpoint, CTimecode& Outpoint, int Preroll)
{
	HRESULT hr = S_OK;

	if (m_pIExtTransport)
	{
		long EditID;
		hr = m_pIExtTransport->SetEditPropertySet(&EditID, ED_REGISTER);
		if (SUCCEEDED(hr))
		{
			// set the edit mode
			m_pIExtTransport->SetEditProperty(EditID, ED_EDIT_MODE, ED_EDIT_MODE_INSERT);
			// set the particulars about the event
			m_pIExtTransport->SetEditProperty(EditID, ED_EDIT_TRACK, ED_VIDEO | ED_AUDIO_ALL);
			m_pIExtTransport->SetEditProperty(EditID, ED_EDIT_REHEARSE_MODE, ED_EDIT_PERFORM);

			// set the record times
			m_pIExtTransport->SetEditProperty(EditID, ED_EDIT_REC_INPOINT, Inpoint.GetTimecode().dwFrames);
			m_pIExtTransport->SetEditProperty(EditID, ED_EDIT_REC_OUTPOINT, Outpoint.GetTimecode().dwFrames + 1);

			// activate the edit event
			hr = m_pIExtTransport->SetEditPropertySet(&EditID, ED_ACTIVE);
			if (SUCCEEDED(hr))
			{
				// cue the transport
				m_pIExtTransport->SetEditProperty(EditID, ED_EDIT_SEEK_MODE, ED_EDIT_SEEK_PREROLL);
				hr = m_pIExtTransport->SetEditProperty(EditID, ED_EDIT_SEEK, OATRUE);
			}
		}
	}
	else
	{
		hr = E_NOINTERFACE;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// UpdateTimecodeCtrl
//
void CDecklinkExportToTapeDlg::UpdateTimecodeCtrl(CTimecode& Timecode)
{
	static CTimecode LastTimecode;
	
	if (LastTimecode != Timecode)
	{
		m_TimecodeCtrl.SetWindowText(Timecode.TimecodeToString());
		LastTimecode = Timecode;
	}
}
