//-----------------------------------------------------------------------------
// DecklinkHistogramDlg.cpp
//
// Desc: DirectShow histogram sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "DecklinkHistogram.h"
#include "DecklinkHistogramDlg.h"
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

//-----------------------------------------------------------------------------
// CDecklinkHistogramDlg dialog
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor
//
CDecklinkHistogramDlg::CDecklinkHistogramDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecklinkHistogramDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-----------------------------------------------------------------------------
// Destructor
//
void CDecklinkHistogramDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_deviceCtrl);
	DDX_Control(pDX, IDC_COMBO_VIDEOFORMAT, m_videoFormatCtrl);
}

//-----------------------------------------------------------------------------
// Message map
//
BEGIN_MESSAGE_MAP(CDecklinkHistogramDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, OnCbnSelchangeComboDevice)
	ON_CBN_SELCHANGE(IDC_COMBO_VIDEOFORMAT, OnCbnSelchangeComboVideoformat)
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// CDecklinkHistogramDlg message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// OnInitDialog
//
BOOL CDecklinkHistogramDlg::OnInitDialog()
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
	m_pVideoCapture = NULL;

	m_ROTRegister = 0;

	// create the custom histogram control
	m_pHistogramCtrl = new CHistogramCtrl();
	if (m_pHistogramCtrl)
	{
		// get the bounding box dimensions
		CStatic* pStaticPreview = (CStatic*)GetDlgItem(IDC_STATIC_PREVIEW);
		CStatic* pStaticHistogram = (CStatic*)GetDlgItem(IDC_STATIC_HISTOGRAM);
		if (pStaticPreview && pStaticHistogram)
		{
			RECT rcPreviewBounds;
			RECT rcHistogramBounds;
			RECT rcWindowsBounds;
			pStaticPreview->GetWindowRect(&rcPreviewBounds);
			pStaticHistogram->GetWindowRect(&rcHistogramBounds);
			GetClientRect(&rcWindowsBounds);

			// set histogram control size to be the same height as the preview window but
			// within the bounding box of the histogram group
			rcHistogramBounds.left += 10;	// indent the left hand edge of the control
			rcHistogramBounds.top = rcPreviewBounds.top;
			rcHistogramBounds.right -= 10;	// indent the right hand edge of the control
			rcHistogramBounds.bottom = rcPreviewBounds.bottom;
			ScreenToClient(&rcHistogramBounds);
			m_pHistogramCtrl->Create(NULL, SS_SUNKEN | WS_VISIBLE,  rcHistogramBounds, this, 3000);
		}
	}

	// create the sample grabber callback handler
	m_pSGCallback = new CSGCallbackHandler(m_pHistogramCtrl);

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
			// locate capture devices present in the system and add one to the graph
			hr = PopulateDeviceControl(&CLSID_VideoInputDeviceCategory, &m_deviceCtrl);
			if (SUCCEEDED(hr))
			{
				PWSTR pName = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.SetCurSel(0));
				if (pName)
				{
					hr = CDSUtils::AddFilter2(m_pGraph, CLSID_VideoInputDeviceCategory, pName, &m_pVideoCapture);
					if (SUCCEEDED(hr))
					{
						hr = PopulateVideoControl();	// populate the video format control with the video formats of the currently selected device
					}

					hr = CDSUtils::AddFilter(m_pGraph, CLSID_SampleGrabber, L"SampleGrabber", &m_pSampleGrabber);
					if (SUCCEEDED(hr))
					{
						// query for the sample grabber interface
						m_pISampleGrabber = m_pSampleGrabber;
						if (m_pISampleGrabber)
						{
							hr = CDSUtils::AddFilter(m_pGraph, CLSID_NullRenderer, L"Null Renderer", &m_pNullRenderer);
							if (SUCCEEDED(hr))
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

									hr = BuildGraph();

									// create polling thread
									m_hStopThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
									if (m_hStopThreadEvent)
									{
										m_hThread = CreateThread(NULL, 0, ThreadWrapper, this, 0, NULL);
										m_pollPeriod = 1;
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
// Called when the window is being destroyed, clean up and free all resources.
BOOL CDecklinkHistogramDlg::DestroyWindow()
{
	SetEvent(m_hStopThreadEvent);	// signal thread to exit
	EXECUTE_ASSERT(WAIT_OBJECT_0 == WaitForSingleObject(m_hThread, 10000));	// wait for thread
	CloseHandle(m_hThread);
	CloseHandle(m_hStopThreadEvent);

#ifdef _DEBUG
	CDSUtils::RemoveGraphFromRot(m_ROTRegister);
#endif
	DestroyGraph();

	SAFE_RELEASE(m_pVideoCapture);

	SAFE_DELETE(m_pSGCallback);
	SAFE_DELETE(m_pHistogramCtrl);

	// free mediatypes attached to format controls
	int count = m_videoFormatCtrl.GetCount();
	int item;
	for (item=0; item<count; ++item)
	{
		DeleteMediaType((AM_MEDIA_TYPE*)m_videoFormatCtrl.GetItemData(item));
	}

	// release the device names attached to the item's data
	count = m_deviceCtrl.GetCount();
	for (item=0; item<count; ++item)
	{
		PWSTR pName = (PWSTR)m_deviceCtrl.GetItemData(item);
		delete [] pName;
	}

	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
// OnSysCommand
//
void CDecklinkHistogramDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CDecklinkHistogramDlg::OnPaint() 
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
HCURSOR CDecklinkHistogramDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-----------------------------------------------------------------------------
// BuildGraph
// Construct a graph that incorporates and sets up the sample grabber so that our
// application can perform some analysis on the media samples passing through the graph.
HRESULT CDecklinkHistogramDlg::BuildGraph(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph && m_pControl)
	{
		// create the following:
		//
		//  Decklink Video Capture -> Smart-T -> AVI Decompressor -> Sample Grabber -> Null Renderer
		//									  -> AVI Decompressor -> Video Renderer
		//

		// locate smart-T
		// NOTE: The smart-T appears to hold references to its upstream connections even when its input pin
		// is diconnected.  The smart-T has to be removed from the graph in order to clear these references which
		// is why the filter is enumerated and added every time the preview graph is built and removed whenever
		// it is destroyed.
		ASSERT(!m_pSmartT);
		hr = CDSUtils::AddFilter(m_pGraph, CLSID_SmartTee, L"Smart Tee", &m_pSmartT);
		if (SUCCEEDED(hr))
		{
			// first connect the video capture pin to the smart-T
			hr = CDSUtils::ConnectFilters(m_pGraph, m_pVideoCapture, NULL, m_pSmartT, NULL);
			if (SUCCEEDED(hr))
			{
				// render the capture pin on the smart-T filter
				// first specify the format for the sample grabber to be RGB
				if (m_pISampleGrabber)
				{
					// locate the video pin and determine the connection format
					CComPtr<IPin> pIPin = NULL;
					hr = CDSUtils::GetPin(m_pVideoCapture, &MEDIATYPE_Video, PINDIR_OUTPUT, &pIPin);
					if (SUCCEEDED(hr))
					{
						AM_MEDIA_TYPE amt = {0};
						hr = pIPin->ConnectionMediaType(&amt);
						if (SUCCEEDED(hr))
						{
							BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(&amt);
							if (pbmih && m_pHistogramCtrl)
							{
								// set the desired media type to receive in the sample grabber
								// and recalculate the critical parameters
								amt.subtype = MEDIASUBTYPE_RGB24;
								pbmih->biBitCount = 24;
								pbmih->biCompression = BI_RGB;
								pbmih->biSizeImage = (pbmih->biWidth * pbmih->biHeight * pbmih->biBitCount) >> 3;	// NOTE: This calculation is incorrect for v210 or r210
								amt.lSampleSize = pbmih->biSizeImage;

								hr = m_pHistogramCtrl->SetType(pbmih);
								
								// set the polling period
								m_pollPeriod = (DWORD)((float)UNITS / CUtils::GetAvgTimePerFrame(&amt) + 0.5);
							}

							hr = m_pISampleGrabber->SetMediaType(&amt);
							if (SUCCEEDED(hr))
							{
								m_pISampleGrabber->SetCallback(m_pSGCallback, 0);	// callback with an IMediaSample interface
							}

							FreeMediaType(amt);
						}
					}
				}
				else
				{
					hr = E_POINTER;
				}

				if (SUCCEEDED(hr))
				{
					hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, L"Capture", m_pSampleGrabber, L"Input");
					if (SUCCEEDED(hr))
					{
						hr = CDSUtils::ConnectFilters(m_pGraph, m_pSampleGrabber, L"Output", m_pNullRenderer, L"In");
						if (SUCCEEDED(hr))
						{
							// now connect the preview pin of the smart-T to the video renderer
							hr = CDSUtils::ConnectFilters(m_pGraph, m_pSmartT, L"Preview", m_pVideoRenderer, NULL);
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
	else
	{
		hr = E_POINTER;
	}

	if (FAILED(hr))
	{
		MessageBox(TEXT("An error was detected when attempting to build the graph"), NULL, MB_OK | MB_ICONEXCLAMATION);
	}

	return hr;
}

//-----------------------------------------------------------------------------
// DestroyGraph
// Search for all non-requisite filters and remove them from the graph
HRESULT CDecklinkHistogramDlg::DestroyGraph(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph && m_pControl)
	{
		m_pControl->Stop();

		// release our outstanding reference on this filter so it can be removed from the graph
		m_pSmartT = NULL;

		// cancel the callback
		if (m_pISampleGrabber)
		{
			m_pISampleGrabber->SetCallback(NULL, 0);
		}

		// retrieve the name of the capture device, don't remove it in this method
		PWSTR pName = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());

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
					
					if ((NULL == wcsstr(filterInfo.achName, pName)) && (NULL == wcsstr(filterInfo.achName, L"SampleGrabber")) && (NULL == wcsstr(filterInfo.achName, L"Renderer")))
					{
						hr = m_pGraph->RemoveFilter(pFilter);
						if (SUCCEEDED(hr))
						{
							hr = pEnum->Reset();
						}
					}
					else
					{
						// disconnect the pins on this filter
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
// PopulateDeviceControl
// Fill device combo box with available devices of the specified category
HRESULT CDecklinkHistogramDlg::PopulateDeviceControl(const GUID* pCategory, CComboBox* pCtrl)
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
HRESULT CDecklinkHistogramDlg::PopulateVideoControl(void)
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
			// get the current video format
			AM_MEDIA_TYPE* pamtCurrent = NULL;
			hr = pISC->GetFormat(&pamtCurrent);
			if (SUCCEEDED(hr))
			{
				BITMAPINFOHEADER* pbmihCurrent = CUtils::GetBMIHeader(pamtCurrent);
				REFERENCE_TIME rtAvgTimePerFrame = CUtils::GetAvgTimePerFrame(pamtCurrent);
				
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
								TCHAR buffer[128];
								WORD PixelFormat;
								
								ZeroMemory(buffer, sizeof(buffer));
								pvih = (VIDEOINFOHEADER*)pmt->pbFormat;
								if (pvih->bmiHeader.biBitCount == 16) PixelFormat = 8;
								else if (pvih->bmiHeader.biBitCount == 20) PixelFormat = 10;
								else PixelFormat = pvih->bmiHeader.biBitCount;
								

								// provide a useful description of the formats
								float FrameRate = (float)UNITS / pvih->AvgTimePerFrame;
								if ((FrameRate - (int)FrameRate) > 0.01)
								{
									StringCbPrintf(buffer, sizeof(buffer), TEXT("%ldx%ld %.2f %d-bit"), pvih->bmiHeader.biWidth, pvih->bmiHeader.biHeight, FrameRate, PixelFormat);
								}
								else
								{
									StringCbPrintf(buffer, sizeof(buffer), TEXT("%ldx%ld %.0f %d-bit"), pvih->bmiHeader.biWidth, pvih->bmiHeader.biHeight, FrameRate, PixelFormat);
								}

								// add the item description to combo box
								int n = m_videoFormatCtrl.AddString(buffer);
								// store media type pointer in item's data section
								m_videoFormatCtrl.SetItemData(n, (DWORD_PTR)pmt);
								
								// set the current format
								if ((pvih->AvgTimePerFrame == rtAvgTimePerFrame) &&
									(pvih->bmiHeader.biWidth == pbmihCurrent->biWidth) &&
									(pvih->bmiHeader.biHeight == pbmihCurrent->biHeight) &&
									(pvih->bmiHeader.biBitCount == pbmihCurrent->biBitCount))
								{
									m_videoFormatCtrl.SetCurSel(n);
								}
							}
						}
					}
					else
					{
						m_videoFormatCtrl.AddString(TEXT("ERROR: Unable to retrieve video formats"));
					}
				}
				DeleteMediaType(pamtCurrent);
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
// OnCbnSelchangeComboDevice
// Rebuild graph with selected capture device
void CDecklinkHistogramDlg::OnCbnSelchangeComboDevice()
{
	SAFE_RELEASE(m_pVideoCapture);	// release our outstanding reference
	HRESULT hr = DestroyGraph();	// remove intermediate filters
	if (SUCCEEDED(hr))
	{
		PWSTR pName = (PWSTR)m_deviceCtrl.GetItemData(m_deviceCtrl.GetCurSel());
		if (pName)
		{
			hr = CDSUtils::AddFilter2(m_pGraph, CLSID_VideoInputDeviceCategory, pName, &m_pVideoCapture);
			if (SUCCEEDED(hr))
			{
				hr = PopulateVideoControl();	// repopulate the control with formats from the new device
				if (SUCCEEDED(hr))
				{
					hr = BuildGraph();	// rebuild the graph with the new device
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
// OnCbnSelchangeComboVideoformat
// Video format selection has changed, rebuild the graph with the new video format
void CDecklinkHistogramDlg::OnCbnSelchangeComboVideoformat()
{
	HRESULT hr = DestroyGraph();	// remove intermediate filters
	if (SUCCEEDED(hr))
	{
		CComPtr<IAMStreamConfig> pISC = NULL;
		hr = CDSUtils::FindPinInterface(m_pVideoCapture, &MEDIATYPE_Video, PINDIR_OUTPUT, IID_IAMStreamConfig, reinterpret_cast<void**>(&pISC));
	    if (SUCCEEDED(hr))
		{
			hr = pISC->SetFormat((AM_MEDIA_TYPE*)m_videoFormatCtrl.GetItemData(m_videoFormatCtrl.GetCurSel()));
			if (SUCCEEDED(hr))
			{
				hr = BuildGraph();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// ThreadWrapper
// Static method which wraps class thread method
DWORD WINAPI CDecklinkHistogramDlg::ThreadWrapper(LPVOID lpParameter)
{
	DWORD ret = 0;
	CDecklinkHistogramDlg* pDlg = (CDecklinkHistogramDlg*)lpParameter;
	if (pDlg)
	{
		ret = pDlg->Thread();
	}
	return ret;
}

//-----------------------------------------------------------------------------
// Thread
// The actual thread for the class, retrieve samples from the stream into our
// application and update the control
DWORD CDecklinkHistogramDlg::Thread(void)
{
	BOOL bRunning = TRUE;
	
	while (bRunning)
	{
		switch (WaitForSingleObject(m_hStopThreadEvent, m_pollPeriod))	// poll every frame interval
		{
			default:
			case WAIT_ABANDONED:
			case WAIT_OBJECT_0:
				bRunning = FALSE;
				break;

			case WAIT_TIMEOUT:
				if (m_pControl && m_pHistogramCtrl)
				{
					// test to see if the graph is running
					OAFilterState fs = {0};
					if (SUCCEEDED(m_pControl->GetState(0, &fs)))
					{
						if (State_Running == fs)
						{
							// only update the control if the graph is running
							m_pHistogramCtrl->RedrawWindow();
						}
					}
				}
				break;		
		}	
	}

	return 0;
}
