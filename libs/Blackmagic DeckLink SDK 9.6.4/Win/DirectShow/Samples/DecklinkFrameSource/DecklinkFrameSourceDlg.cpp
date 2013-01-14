//------------------------------------------------------------------------------
// DecklinkFrameSourceDlg.cpp
//
// Desc: DirectShow sample code - Application to deliver frames
//		 to a DirectShow graph with the Decklink video render filter
//		 via a custom interface on a push source filter.
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//
// Read readme.txt!
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "DecklinkFrameSource.h"
#include "DecklinkFrameSourceDlg.h"

#include "DecklinkFilters_h.h"
#include "Timecode.h"
#include "StaticURL.h"

#include <math.h>

#include <gdiplus.h>
using namespace Gdiplus;

//-------------------------------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-------------------------------------------------------------------------------------------------
DWORD WINAPI ThreadFn(LPVOID lpParam);

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

//-------------------------------------------------------------------------------------------------
// CDecklinkFrameSourceDlg dialog implementation
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// Constructor
//
CDecklinkFrameSourceDlg::CDecklinkFrameSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecklinkFrameSourceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-------------------------------------------------------------------------------------------------
// DoDataExchange
//
void CDecklinkFrameSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FRAMECOUNT, m_FrameCountCtrl);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_Preview);
	DDX_Control(pDX, IDC_COMBO_VIDEOFORMAT, m_videoFormatCtrl);
	DDX_Control(pDX, IDC_COMBO_AUDIOFORMAT, m_audioFormatCtrl);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_deviceCtrl);
}

//-------------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CDecklinkFrameSourceDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnCbnSelchangeComboDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_VIDEOFORMAT, OnCbnSelchangeComboVideoformat)
	ON_CBN_SELCHANGE(IDC_COMBO_AUDIOFORMAT, OnCbnSelchangeComboAudioformat)
END_MESSAGE_MAP()

//-------------------------------------------------------------------------------------------------
// CDecklinkFrameSourceDlg message handlers
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// OnInitDialog
// Provides dialog initialisation, builds graph and initialises dialog controls
BOOL CDecklinkFrameSourceDlg::OnInitDialog()
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
	m_hThreadEvent = NULL;

	// locate the video render devices
	if (SUCCEEDED(PopulateDeviceControl()))
	{
		m_deviceCtrl.EnableWindow();
		
		if (SUCCEEDED(PopulateVideoControl()))
		{
			m_videoFormatCtrl.EnableWindow();
		}

		if (SUCCEEDED(PopulateAudioControl()))
		{
			m_audioFormatCtrl.EnableWindow();
		}
	}
	else
	{
		// no devices could be located so disable all the dialog controls
		m_deviceCtrl.EnableWindow(FALSE);
		m_videoFormatCtrl.EnableWindow(FALSE);
		m_audioFormatCtrl.EnableWindow(FALSE);

		CWnd* pCtrl = GetDlgItem(IDC_BUTTON_PLAY);
		pCtrl->EnableWindow(FALSE);
		pCtrl = GetDlgItem(IDC_BUTTON_STOP);
		pCtrl->EnableWindow(FALSE);
	}

	m_FrameCountCtrl.SetWindowText("0");

	// Build the graph
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&m_pGraph));
	if (SUCCEEDED(hr))
	{
#ifdef _DEBUG
		hr = CDSUtils::AddGraphToRot(m_pGraph, &m_ROTRegister);
#endif
		m_pControl = m_pGraph;
		if (m_pControl)
		{
			// The graph:
			//	   --------------------- 	 ------------------     -----------------------
			//    | DecklinkVideoSource |-->|                  |-->| Decklink video render |
			//     ---------------------    | Infinite pin tee |    -----------------------
			//                              |                  |    ----------------
			//                              |                  |-->| Video renderer |
			//								 ------------------     ----------------
			//
			//	   --------------------- 	 -----------------------
			//    | DecklinkAudioSource |-->| Decklink audio render |
			//     ---------------------     -----------------------
			//

			hr = CDSUtils::AddFilter(m_pGraph, CLSID_DecklinkVideoSource, L"Decklink Video Source Filter", &m_pDecklinkVideoSource);
			if (SUCCEEDED(hr))
			{
				hr = CDSUtils::AddFilter(m_pGraph, CLSID_DecklinkAudioSource, L"Decklink Audio Source Filter", &m_pDecklinkAudioSource);
				if (SUCCEEDED(hr))
				{
					hr = CDSUtils::AddFilter(m_pGraph, CLSID_InfTee, L"Infinite Pin Tee Filter", &m_pInfinitePinT);
					if (SUCCEEDED(hr))
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

							hr = CreateGraph();
						}
					}
				}
			}
		}
	}

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    Status s = GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
    if (s != Ok)
    {
        return FALSE;
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//-------------------------------------------------------------------------------------------------
// DestroyWindow
// Override to release resources
BOOL CDecklinkFrameSourceDlg::DestroyWindow()
{
	SetEvent(m_hThreadEvent);

	DestroyGraph();
#ifdef _DEBUG
	CDSUtils::RemoveGraphFromRot(m_ROTRegister);
#endif

	// release the device names attached to the item's data
	while (m_deviceCtrl.GetCount())
	{
		PWSTR pName = (PWSTR)m_deviceCtrl.GetItemData(0);
		delete [] pName;
		m_deviceCtrl.DeleteString(0);
	}

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

	if (m_hThreadEvent)
	{
		CloseHandle(m_hThreadEvent);
	}
	
	if (m_hThread)
	{
		CloseHandle(m_hThread);
	}

    // Shut down GDI+
    GdiplusShutdown(m_gdiplusToken);

	return CDialog::DestroyWindow();
}

//-------------------------------------------------------------------------------------------------
// OnSysCommand
//
void CDecklinkFrameSourceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

//-------------------------------------------------------------------------------------------------
// OnPaint
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CDecklinkFrameSourceDlg::OnPaint() 
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

//-------------------------------------------------------------------------------------------------
// OnQueryDragIcon
// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR CDecklinkFrameSourceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-------------------------------------------------------------------------------------------------
// CreateGraph
//
HRESULT CDecklinkFrameSourceDlg::CreateGraph(void)
{
	// query for IAMStreamConfig interface and set the output format of the Decklink audio source filter
	WAVEFORMATEX* pwfex = NULL;
	CComPtr<IAMStreamConfig> pIAMStreamConfig = NULL;
	HRESULT hr = CDSUtils::FindPinInterface(m_pDecklinkAudioSource, L"Audio", IID_IAMStreamConfig, reinterpret_cast<void**>(&pIAMStreamConfig));
	if (SUCCEEDED(hr))
	{
		// set desired output audio format, using the IAMStreamConfig interface e.g.
		int Index = m_audioFormatCtrl.GetCurSel();
		if (CB_ERR != Index)
		{
			AM_MEDIA_TYPE* pmt = (AM_MEDIA_TYPE*)m_audioFormatCtrl.GetItemData(Index);
			hr = pIAMStreamConfig->SetFormat(pmt);
			pwfex = (WAVEFORMATEX*)pmt->pbFormat;
		}
	}

	// query for IAMStreamConfig interface and set the output format of the Decklink video source filter
	pIAMStreamConfig = NULL;
	hr = CDSUtils::FindPinInterface(m_pDecklinkVideoSource, L"Video", IID_IAMStreamConfig, reinterpret_cast<void**>(&pIAMStreamConfig));
	if (SUCCEEDED(hr))
	{
		// set desired output video format, using the IAMStreamConfig interface e.g.
		int Index = m_videoFormatCtrl.GetCurSel();
		if (CB_ERR != Index)
		{
			AM_MEDIA_TYPE* pmt = (AM_MEDIA_TYPE*)m_videoFormatCtrl.GetItemData(Index);
			hr = pIAMStreamConfig->SetFormat(pmt);
			if (SUCCEEDED(hr))
			{
				CComPtr<IAMBufferNegotiation> pIAMBufferNegotiation;
				hr = CDSUtils::FindPinInterface(m_pDecklinkAudioSource, L"Audio", IID_IAMBufferNegotiation, reinterpret_cast<void**>(&pIAMBufferNegotiation));
				if (SUCCEEDED(hr) && pIAMBufferNegotiation && pwfex)
				{
					ALLOCATOR_PROPERTIES AllocProp;
					AllocProp.cBuffers = 1;
					AllocProp.cbAlign = -1;
					AllocProp.cbPrefix = -1;

					REFERENCE_TIME rtAvgTimePerFrame = CUtils::GetAvgTimePerFrame(pmt);
					unsigned long FrameRate = (unsigned long)(UNITS / rtAvgTimePerFrame);
					if ((29 == FrameRate) || (59 == FrameRate))
					{
						// For non-integer frame rates (29.97/59.94) use IAMBufferNegotiation interface to allocate a slightly larger audio buffer.
						// E.g. We cannot create fractional frames, so in one second at 29.97fps we will actually create 30 frames.  At 29.97fps this represents
						// more than one second of audio, at 48kHz we need to create an extra 48 samples: 29.97 = 30 / 1.001, therefore 48000 * 1.001 = 48048.
						AllocProp.cbBuffer = (long)(pwfex->nAvgBytesPerSec + pwfex->nAvgBytesPerSec / 1000);
					}
					else
					{
						AllocProp.cbBuffer = (long)pwfex->nAvgBytesPerSec;
					}
					pIAMBufferNegotiation->SuggestAllocatorProperties(&AllocProp);
				}
			}
		}
	}

	// connect the frame source filter to the infinite-T
	hr = CDSUtils::ConnectFilters(m_pGraph, m_pDecklinkVideoSource, L"Video", m_pInfinitePinT, L"Input");
	if (SUCCEEDED(hr))
	{
		// add the video renderer
		int Index = m_deviceCtrl.GetCurSel();
		if (CB_ERR != Index)
		{
			PWSTR pNameVideo = (PWSTR)m_deviceCtrl.GetItemData(Index);
			CComPtr<IBaseFilter> pDecklinkVideoRenderer = NULL;
			hr = CDSUtils::AddFilter2(m_pGraph, CLSID_TransmitCategory, pNameVideo, &pDecklinkVideoRenderer);
			if (SUCCEEDED(hr))
			{
				// connect the infinite-T to the Decklink video renderer
				hr = CDSUtils::ConnectFilters(m_pGraph, m_pInfinitePinT, NULL, pDecklinkVideoRenderer, NULL);
				if (SUCCEEDED(hr))
				{
					// connect the infinite-T to the video renderer
					hr = CDSUtils::ConnectFilters(m_pGraph, m_pInfinitePinT, NULL, m_pVideoRenderer, NULL);
					if (SUCCEEDED(hr))
					{
						// now the output pin of the Decklink frame source is connected, query for the custom interface
						hr = CDSUtils::FindPinInterface(m_pDecklinkVideoSource, L"Video", IID_IDecklinkPushSource2, reinterpret_cast<void**>(&m_pIPushSourceVideo));
						if (SUCCEEDED(hr))
						{
							// add the audio renderer
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
								hr = CDSUtils::ConnectFilters(m_pGraph, m_pDecklinkAudioSource, L"Audio", pDecklinkAudioRenderer, NULL);
								if (SUCCEEDED(hr))
								{
									hr = CDSUtils::FindPinInterface(m_pDecklinkAudioSource, L"Audio", IID_IDecklinkPushSource2, reinterpret_cast<void**>(&m_pIPushSourceAudio));
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

//-------------------------------------------------------------------------------------------------
// DestroyGraph
// Disconnect all filters.
HRESULT CDecklinkFrameSourceDlg::DestroyGraph(void)
{
	HRESULT hr = S_OK;

	// release the outstanding interface reference
	m_pIPushSourceVideo = NULL;
	m_pIPushSourceAudio = NULL;

	if (m_pGraph && m_pControl)
	{
		m_pControl->Stop();

		// enumerate all the filters in the graph
		CComPtr<IEnumFilters> pEnum = NULL;
		hr = m_pGraph->EnumFilters(&pEnum);
		if (SUCCEEDED(hr))
		{
			IBaseFilter* pFilter = NULL;
			while (S_OK == pEnum->Next(1, &pFilter, NULL))
			{
				FILTER_INFO filterInfo;
				if (SUCCEEDED(pFilter->QueryFilterInfo(&filterInfo)))
				{
					SAFE_RELEASE(filterInfo.pGraph);
					if ((NULL == wcsstr(filterInfo.achName, L"Decklink Video Source Filter")) && (NULL == wcsstr(filterInfo.achName, L"Decklink Audio Source Filter")) && (NULL == wcsstr(filterInfo.achName, L"Infinite Pin Tee Filter")) && (NULL == wcsstr(filterInfo.achName, L"Video Renderer")))
					{
						// not the push source, infinite tee or renderer filter so remove from graph
						hr = m_pGraph->RemoveFilter(pFilter);
						if (SUCCEEDED(hr))
						{
							hr = pEnum->Reset();
						}
					}
					else
					{
						// enumerate all the pins on the filter
						CComPtr<IEnumPins> pIEnumPins = NULL;
						hr = pFilter->EnumPins(&pIEnumPins);
						if (SUCCEEDED(hr))
						{
							IPin* pIPin = NULL;
							while (S_OK == pIEnumPins->Next(1, &pIPin, NULL))
							{
								m_pGraph->Disconnect(pIPin);
								SAFE_RELEASE(pIPin);
							}
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
// PopulateVideoControl
// Fill video format combo box with supported video formats.
HRESULT CDecklinkFrameSourceDlg::PopulateDeviceControl(void)
{
	HRESULT hr = S_OK;

	// first enumerate the system devices for the specifed class and filter name
	CComPtr<ICreateDevEnum> pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pSysDevEnum));

	if (SUCCEEDED(hr))
	{
		CComPtr<IEnumMoniker> pEnumCat = NULL;
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_TransmitCategory, &pEnumCat, 0);

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

	if (1 > m_deviceCtrl.GetCount())
	{
		// found at least one device so remove the <No Devices> item
		m_deviceCtrl.AddString(TEXT("<No Devices>"));
		hr = E_FAIL;
	}

	m_deviceCtrl.SetCurSel(0);

	return hr;
}

//-----------------------------------------------------------------------------
// PopulateVideoControl
// Fill video format combo box with supported video formats.
HRESULT CDecklinkFrameSourceDlg::PopulateVideoControl(void)
{
	// NOTE: Using RGB32 in this sample so that GDI+ can draw text into our frame buffer,
	//		 other formats are available such as 8-bit YUV, 10-bit YUV and 10-bit RGB.  Also note
	//		 that the data rate of HD RGB is VERY HIGH and is unlikely to work well on most
	//		 systems.

	CComPtr<IGraphBuilder> pIGraphBuilder;
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&pIGraphBuilder));
	if (SUCCEEDED(hr))
	{
		// get the device name and convert to a capture filter name
		basic_string<WCHAR> VideoCaptureName = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());
		if (!VideoCaptureName.empty())
		{
			basic_string<WCHAR>::size_type index = VideoCaptureName.find(L"Render");
			if (basic_string<WCHAR>::npos != index)
			{
				VideoCaptureName.erase(index);
				VideoCaptureName.append(L"Capture");
			}
		}

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

		CComPtr<IBaseFilter> pDecklinkVideoCapture;
		hr = CDSUtils::AddFilter2(pIGraphBuilder, CLSID_VideoInputDeviceCategory, VideoCaptureName.c_str(), &pDecklinkVideoCapture);
		if (SUCCEEDED(hr))
		{
			CComPtr<IAMStreamConfig> pISC;
			hr = CDSUtils::FindPinInterface(pDecklinkVideoCapture, &MEDIATYPE_Video, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
			if (SUCCEEDED(hr))
			{
				// All DeckLink cards support 10-bit YUV, so use all the available 10-bit YUV formats as a template
				// for the RGB formats to be used in this sample.
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
								if (IID_MEDIASUBTYPE_v210a == pmt->subtype)
								{
									// convert media type to an RGB format
									pmt->subtype = MEDIASUBTYPE_RGB32;
									pvih = (VIDEOINFOHEADER*)pmt->pbFormat;
									pvih->bmiHeader.biBitCount = 32;
									pvih->bmiHeader.biCompression = BI_RGB;
									pvih->bmiHeader.biSizeImage = pvih->bmiHeader.biWidth * pvih->bmiHeader.biHeight * pvih->bmiHeader.biBitCount / 8;
									pmt->lSampleSize = pvih->bmiHeader.biSizeImage;
									pvih->dwBitRate = pvih->bmiHeader.biSizeImage * (DWORD)((float)UNITS / pvih->AvgTimePerFrame) * 8;

									float	frameRate = (float)UNITS / pvih->AvgTimePerFrame;
									TCHAR	buffer[128];
									ZeroMemory(buffer, sizeof(buffer));

									// provide a useful description of the formats
									if (486 == pvih->bmiHeader.biHeight)
									{
										if (417083 == pvih->AvgTimePerFrame)
										{
											StringCbPrintf(buffer, sizeof(buffer), TEXT("NTSC - RGB (3:2 pulldown removal)"));
										}
										else
										{
											StringCbPrintf(buffer, sizeof(buffer), TEXT("NTSC - RGB"));
										}
									}
									else if (576 == pvih->bmiHeader.biHeight)
									{
										StringCbPrintf(buffer, sizeof(buffer), TEXT("PAL - RGB"));
									}
									else
									{
										if (720 == pvih->bmiHeader.biHeight)
										{
											// 720p
											if ((frameRate - (int)frameRate) > 0.01)
											{
												StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 720p %.2f - RGB"), frameRate);
											}
											else
											{
												StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 720p %.0f - RGB"), frameRate);
											}
										}
										else if (1080 == pvih->bmiHeader.biHeight)
										{
											if ((frameRate < 25.0) || (frameRate >= 50.0))		// 1080p23, 1080p24, 1080p50, 1080p60
											{
												if ((frameRate - (int)frameRate) > 0.01)
												{
													StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 1080p %.2f - RGB"), frameRate);
												}
												else
												{
													StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 1080p %.0f - RGB"), frameRate);
												}
											}
											else
											{
												// 1080i
												if ((frameRate - (int)frameRate) > 0.01)
												{
													StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 1080i %.2f - RGB"), frameRate*2.0f);
												}
												else
												{
													StringCbPrintf(buffer, sizeof(buffer), TEXT("HD 1080i %.0f - RGB"), frameRate*2.0f);
												}
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

										if ((486 == pvih->bmiHeader.biHeight) && (29 == (int)frameRate))
										{
											m_videoFormatCtrl.SetCurSel(n);
										}
										
										// Don't release the mediatype
										pmt = NULL;
									}
								}
								
								if (pmt != NULL)
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
		}
	}

	return hr;
}

//-----------------------------------------------------------------------------
// PopulateAudioControl
// Fill audio format combo box with supported audio formats.  Build a temporary graph
// with the capture filter of the selected device and QI for the formats from this filter.
HRESULT CDecklinkFrameSourceDlg::PopulateAudioControl(void)
{
	CComPtr<IGraphBuilder> pIGraphBuilder;
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&pIGraphBuilder));
	if (SUCCEEDED(hr))
	{
		// get the device name and convert to a capture filter name
		basic_string<WCHAR> AudioCaptureName = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());
		if (!AudioCaptureName.empty())
		{
			basic_string<WCHAR>::size_type index = AudioCaptureName.find(L"Render");
			if (basic_string<WCHAR>::npos != index)
			{
				AudioCaptureName.erase(index);
				AudioCaptureName.append(L"Capture");
			}

			index = AudioCaptureName.find(L"Video");
			if (basic_string<WCHAR>::npos != index)
			{
				AudioCaptureName.replace(index, 5, L"Audio");
			}
		}

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

		CComPtr<IBaseFilter> pDecklinkAudioCapture;
		hr = CDSUtils::AddFilter2(pIGraphBuilder, CLSID_AudioInputDeviceCategory, AudioCaptureName.c_str(), &pDecklinkAudioCapture);
		if (SUCCEEDED(hr))
		{
			CComPtr<IAMStreamConfig> pISC;
			hr = CDSUtils::FindPinInterface(pDecklinkAudioCapture, &MEDIATYPE_Audio, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
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
								
								if (2 == pwfex->nChannels)
								{
									m_audioFormatCtrl.SetCurSel(n);
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
	}

	return hr;
}

//-------------------------------------------------------------------------------------------------
// OnBnClickedButtonPlay
// Play graph
void CDecklinkFrameSourceDlg::OnBnClickedButtonPlay()
{
	m_FrameCount = 0;
	m_SampleCount = 0;

	// create the thread that generates frames
	if (NULL == m_hThreadEvent)
	{
		m_hThreadEvent = CreateEvent(NULL, FALSE, FALSE, "DecklinkFrameSource Thread Event");
	}
	
	DWORD dwThreadID;
	if (NULL == m_hThread)
	{
		m_hThread = CreateThread(NULL, 0, ThreadFn, reinterpret_cast<LPVOID>(this), 0, &dwThreadID);
	}

	m_pControl->Run();
}

//-------------------------------------------------------------------------------------------------
// OnBnClickedButtonStop
// Stop graph
void CDecklinkFrameSourceDlg::OnBnClickedButtonStop()
{
	// stop delivery thread
	if (m_hThreadEvent)
	{
		SetEvent(m_hThreadEvent);
	}

	// kill thread
	if (m_hThread)
	{
		EXECUTE_ASSERT(CloseHandle(m_hThread));
		m_hThread = NULL;
	}

	if (m_hThreadEvent)
	{	
		EXECUTE_ASSERT(CloseHandle(m_hThreadEvent));
		m_hThreadEvent = NULL;
	}

	// stop graph
	m_pControl->Stop();
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboDevice
// Rebuild the graph with the new video device selection.
void CDecklinkFrameSourceDlg::OnCbnSelchangeComboDevice()
{
	OnBnClickedButtonStop();

	HRESULT hr = DestroyGraph();
	if (SUCCEEDED(hr))
	{
		hr = PopulateVideoControl();
		if (SUCCEEDED(hr))
		{
			hr = PopulateAudioControl();
			if (SUCCEEDED(hr))
			{
				hr = CreateGraph();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboVideoformat
// Rebuild the graph with the new video format selection.
void CDecklinkFrameSourceDlg::OnCbnSelchangeComboVideoformat()
{
	OnBnClickedButtonStop();

	HRESULT hr = DestroyGraph();
	if (SUCCEEDED(hr))
	{
		hr = CreateGraph();
	}
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboAudioformat
// Rebuild the graph with the new audio format selection.
void CDecklinkFrameSourceDlg::OnCbnSelchangeComboAudioformat()
{
	OnBnClickedButtonStop();

	HRESULT hr = DestroyGraph();
	if (SUCCEEDED(hr))
	{
		hr = CreateGraph();
	}
}

//-------------------------------------------------------------------------------------------------
// ThreadFn
// A wrapper for the dialog app thread
DWORD WINAPI CDecklinkFrameSourceDlg::ThreadFn(LPVOID lpParam)
{
	CDecklinkFrameSourceDlg* pAppDlg = reinterpret_cast<CDecklinkFrameSourceDlg*>(lpParam);
	if (pAppDlg)
	{
		return pAppDlg->Thread();
	}
	else
	{
		return -1;
	}
}

//-------------------------------------------------------------------------------------------------
// ThreadFn
// The application delivery thread delivers the rendered buffers
DWORD CDecklinkFrameSourceDlg::Thread(void)
{
	DWORD dwRet = 0;
	BOOL bRunning = TRUE;

	IMediaSample* pSample = NULL;
	BYTE* pBuffer = NULL, * pBuffer2 = NULL, * pFieldDst = NULL, * pFieldSrc = NULL;
	DWORD cbBuffer = 0;
	const int XPosEnd = -320, XPosInc = 2;	// The speed of the scrolling text.
	int XPos = XPosEnd;

	// get the connection format
	CComPtr<IAMStreamConfig> pISC = NULL;
	AM_MEDIA_TYPE* pamtVideo = NULL;
	AM_MEDIA_TYPE* pamtAudio = NULL;
	VIDEOINFOHEADER* pvih = NULL;
	WAVEFORMATEX* pwfex = NULL;

	// find the IAMStreamConfig interface on the output pin of the DecklinkVideoSource filter
	HRESULT hr = CDSUtils::FindPinInterface(m_pDecklinkVideoSource, L"Video", IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
	if (SUCCEEDED(hr))
	{
		hr = pISC->GetFormat(&pamtVideo);
		if (SUCCEEDED(hr))
		{
			ASSERT(FORMAT_VideoInfo == pamtVideo->formattype);
			ASSERT(sizeof(VIDEOINFOHEADER) <= pamtVideo->cbFormat);
			pvih = (VIDEOINFOHEADER*)pamtVideo->pbFormat;
			ASSERT(pvih);
		}
	}

	// find the IAMStreamConfig interface on the output pin of the DecklinkAudioSource filter
	pISC = NULL;
	hr = CDSUtils::FindPinInterface(m_pDecklinkAudioSource, L"Audio", IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
	if (SUCCEEDED(hr))
	{
		hr = pISC->GetFormat(&pamtAudio);
		if (SUCCEEDED(hr))
		{
			ASSERT(FORMAT_WaveFormatEx == pamtAudio->formattype);
			ASSERT(sizeof(WAVEFORMATEX) <= pamtAudio->cbFormat);
			pwfex = (WAVEFORMATEX*)pamtAudio->pbFormat;
			ASSERT(pwfex);
		}
	}

	if (SUCCEEDED(hr))
	{
		while (bRunning)
		{
			// deliver frames to the DirectShow graph as fast as possible.
			// NOTE: In a real app this would probably be a variable rate
			switch (WaitForSingleObject(m_hThreadEvent, 10))
			{
				default:
				case WAIT_OBJECT_0:
				case WAIT_ABANDONED:
				case WAIT_FAILED:
					bRunning = FALSE;
					break;

				case WAIT_TIMEOUT:
					// get a new video delivery buffer
					hr = m_pIPushSourceVideo->GetFrameBuffer(&pSample);
					if (SUCCEEDED(hr))
					{
						hr = pSample->GetPointer(&pBuffer);
						cbBuffer = pSample->GetActualDataLength();

						// Render the frame.
						// In this case the frame count is converted to timecode and rendered
						// as a text string.
						// To demonstrate a basic ticker (scrolling text) the text is scrolled
						// across the frame.  This is acheived but first testing to see if the
						// video format is interlaced.  This is important in order to make the
						// scrolling as smooth as possible.
						// An interlaced frame consists of two images from two different points
						// in time.  Half the lines from each image are combined to produce the
						// completed frame.  The odd lines and taken from one frame and the even
						// lines from the other in order to have the correct spatial separation.
						// We also have to be careful with the order of the lines since some
						// formats show the even lines first and others show the odd lines first.
						// If the order is wrong the scrolling will not look smooth.  Simply if
						// it does not look right, change the order to see if it improves.
						// A progressive frame consists of one image from one point in time and
						// is therefore easy to render when compared to an interlaced frame.

						// Allocate a buffer for the second frame.
						if (NULL == pBuffer2)
						{
							pBuffer2 = new BYTE [cbBuffer];
							ZeroMemory(pBuffer2, cbBuffer);
						}

						// Convert the frame count to timecode.
						CTimecode Timecode((WORD)(UNITS / pvih->AvgTimePerFrame), true, m_FrameCount);
						CString strTimecode = Timecode.TimecodeToString();

						// Render two complete frames, for interlaced formats, one will be used for
						// one field and the other frame for the second field.
						WriteToBuffer(XPos, 30, strTimecode.GetBuffer(), pBuffer, &pvih->bmiHeader);
						WriteToBuffer(XPos - XPosInc, 30, strTimecode.GetBuffer(), pBuffer2, &pvih->bmiHeader);
						
						// Update the position of the text.
						XPos -= (XPosInc << 1);
						if (XPosEnd >= XPos)
						{
							XPos = pvih->bmiHeader.biWidth;
						}

						// Determine the field order for interlaced formats.
						long rowBytes = cbBuffer / pvih->bmiHeader.biHeight;
						if (1920 == pvih->bmiHeader.biWidth)
						{
							// DeckLink supports Progessive Segmented Frame (PsF) which we will
							// interlace for smooth scrolling, upper field first.
							pFieldDst = pBuffer;
							pFieldSrc = pBuffer2;
						}
						else if (1280 == pvih->bmiHeader.biWidth)
						{
							// Progessive. Do not interlace.
						}
						else if ((720 == pvih->bmiHeader.biWidth) && (29 == (UNITS / pvih->AvgTimePerFrame)))
						{
							// NTSC.  Interlace.  Lower field first.
							pFieldDst = pBuffer + rowBytes;
							pFieldSrc = pBuffer2 + rowBytes;
						}
						else if ((720 == pvih->bmiHeader.biWidth) && (25 == (UNITS / pvih->AvgTimePerFrame)))
						{
							// PAL.  Interlaced.  Upper field first.
							pFieldDst = pBuffer;
							pFieldSrc = pBuffer2;
						}

						// Only copy the second field for video formats that are not progressive.
						if (1280 != pvih->bmiHeader.biWidth)
						{
							for (int line=0; line<pvih->bmiHeader.biHeight; line+=2)
							{
								memcpy(pFieldDst, pFieldSrc, rowBytes);
								pFieldSrc += (rowBytes << 1);
								pFieldDst += (rowBytes << 1);
							}
						}

						// deliver buffer, the custom allocator will block in this method until the buffer can be delivered
						if (SUCCEEDED(m_pIPushSourceVideo->Deliver(pSample)))
						{
							m_FrameCount += 1;

							CString text;
							text.Format("%ld frames", m_FrameCount);						
							m_FrameCountCtrl.SetWindowText(text);
						}
					}

					// get a new audio delivery buffer
					hr = m_pIPushSourceAudio->GetFrameBuffer(&pSample);
					if (SUCCEEDED(hr))
					{
						hr = pSample->GetPointer(&pBuffer);
						cbBuffer = pSample->GetActualDataLength();
						
						// Write the audio buffer
						hr = FillAudioBuffer(pwfex, &pvih->AvgTimePerFrame, pBuffer, cbBuffer);

						// deliver buffer, the custom allocator will block in this method until the buffer can be delivered
						if (SUCCEEDED(m_pIPushSourceAudio->Deliver(pSample)))
						{
							m_SampleCount += pwfex->nSamplesPerSec;
						}
					}
					break;
			}
		}
	}

	SAFE_DELETE(pBuffer2);
	DeleteMediaType(pamtVideo);
	DeleteMediaType(pamtAudio);

	return dwRet;
} 

//-----------------------------------------------------------------------------
// WriteToBuffer
// Fill the buffer with our image
HRESULT CDecklinkFrameSourceDlg::WriteToBuffer(int xpos, int ypos, LPCTSTR pszText, BYTE* pData, BITMAPINFOHEADER* pbmih)
{
	ASSERT(pbmih->biBitCount == 32);

	// create a GDI+ bitmap object to manage our image buffer.
	Bitmap bitmap((INT)pbmih->biWidth, (INT)pbmih->biHeight, CUtils::GetImageSize(pbmih) / abs(pbmih->biHeight), PixelFormat32bppRGB, pData);

    // create a GDI+ graphics object to manage the drawing.
	Graphics graphics(&bitmap);

	// Turn on anti-aliasing
//	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
//	graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

	// Erase the background
//	TODO: Determine how fast is this operation
//	graphics.Clear(Color(0x0, 0, 0, 0));

	// GDI+ is top-down by default, RGB +ve height is bottom up, so we need to set a transform on the Graphics object to flip the image. 
	if (0 < pbmih->biHeight)
	{
		graphics.ScaleTransform(1.0, -1.0); // Flip the image around the X axis
		graphics.TranslateTransform(0, (REAL)pbmih->biHeight, MatrixOrderAppend);
	}

	{
		SolidBrush brush(Color(255, 167, 119, 69));	// tan background for scrolling text
		graphics.FillRectangle(&brush, (INT)0, (INT)ypos, (INT)pbmih->biWidth, (INT)70);
	}

	SolidBrush brushText(Color(255, 255, 255, 255));	// white brush
	Font font(FontFamily::GenericSerif(), 48); // Big serif type
	RectF rcBounds((REAL)xpos, (REAL)ypos, (REAL)pbmih->biWidth, (REAL)pbmih->biHeight);

	graphics.DrawString(CT2CW(pszText), -1, &font, rcBounds, StringFormat::GenericDefault(), &brushText);	// draw the text

	return S_OK;
}

//------------------------------------------------------------------------------------
// FillAudioBuffer
// Creates a 1kHz tone in the audio buffer
HRESULT CDecklinkFrameSourceDlg::FillAudioBuffer(WAVEFORMATEX* pwfex, REFERENCE_TIME* prtAvgTimePerFrame, BYTE* pbData, DWORD cbData)
{
	HRESULT hr = S_OK;

	memset(pbData, 0, cbData);

	// 1kHz tone
	double Radian1kHzAcc = 0.0;
	double Radian1kHzInc = 2.0 * 3.1415926535 * 1000.0 / pwfex->nSamplesPerSec;
	int FrameRate = (int)(UNITS / *prtAvgTimePerFrame);
	unsigned SampleCount = 0;
	switch (FrameRate)
	{
		case 23: SampleCount = 2002; break;
		case 24: SampleCount = 2000; break;
		case 25: SampleCount = 1920; break;
		case 29: SampleCount = 1602; break;
		case 30: SampleCount = 1600; break;
		case 50: SampleCount = 960; break;
		case 59: SampleCount = 800; break;
		case 60: SampleCount = 800; break;
	}

	// 1kHz burst for one frame
	short* pData = (short*)pbData, Data;
	for (unsigned Sample=0; Sample<SampleCount; ++Sample, Radian1kHzAcc+=Radian1kHzInc)
	{
		Data = (short)(sin(Radian1kHzAcc) * (double)0x6FFF);
		for (unsigned channel=0; channel<pwfex->nChannels; ++channel)
		{
			*pData++ = Data;
		}
	}

	return hr;
}
