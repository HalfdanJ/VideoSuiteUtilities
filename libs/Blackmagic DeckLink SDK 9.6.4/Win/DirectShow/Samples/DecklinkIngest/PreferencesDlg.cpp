// PreferencesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DecklinkIngest.h"
#include "PreferencesDlg.h"


//-----------------------------------------------------------------------------
// CPreferencesDlg dialog
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CPreferencesDlg, CDialog)
//-----------------------------------------------------------------------------
// Construction
//
CPreferencesDlg::CPreferencesDlg(basic_string<TCHAR>& CaptureLocation, basic_string<TCHAR>& CaptureFilename, IBaseFilter* pVideoCapture, CMediaType& VideoFormat, IBaseFilter* pAudioCapture, CMediaType& AudioFormat, bool bMuteAudio, int Compression, CWnd* pParent /*=NULL*/)
	: CDialog(CPreferencesDlg::IDD, pParent)
	, m_CaptureLocation(CaptureLocation)
	, m_CaptureFilename(CaptureFilename)
	, m_pVideoCapture(pVideoCapture)
	, m_VideoFormat(VideoFormat)
	, m_pAudioCapture(pAudioCapture)
	, m_AudioFormat(AudioFormat)
	, m_bMuteAudio(bMuteAudio)
	, m_Compression(Compression)
{
}

//-----------------------------------------------------------------------------
// Destruction
//
CPreferencesDlg::~CPreferencesDlg()
{
}

//-----------------------------------------------------------------------------
// DoDataExchange
//
void CPreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CAPTURELOCATION, m_CaptureLocationCtrl);
	DDX_Control(pDX, IDC_EDIT_CAPTUREFILENAME, m_CaptureFilenameCtrl);
	DDX_Control(pDX, IDC_COMBO_COMPRESSION, m_CompressionCtrl);
	DDX_Control(pDX, IDC_CHECK_MUTEAUDIO, m_MuteAudioCtrl);
	DDX_Control(pDX, IDC_COMBO_VIDEOFORMAT, m_VideoFormatCtrl);
	DDX_Control(pDX, IDC_COMBO_AUDIOFORMAT, m_AudioFormatCtrl);
}

//-----------------------------------------------------------------------------
// Message map
//
BEGIN_MESSAGE_MAP(CPreferencesDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBnClickedButtonBrowse)
	ON_EN_CHANGE(IDC_EDIT_CAPTURELOCATION, OnEnChangeEditCapturelocation)
	ON_EN_CHANGE(IDC_EDIT_CAPTUREFILENAME, OnEnChangeEditCapturefilename)
	ON_CBN_SELCHANGE(IDC_COMBO_COMPRESSION, OnCbnSelchangeComboCompression)
	ON_BN_CLICKED(IDC_CHECK_MUTEAUDIO, OnBnClickedCheckMuteaudio)
	ON_CBN_SELCHANGE(IDC_COMBO_VIDEOFORMAT, OnCbnSelchangeComboVideoformat)
	ON_CBN_SELCHANGE(IDC_COMBO_AUDIOFORMAT, OnCbnSelchangeComboAudioformat)
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// CPreferencesDlg message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// BrowseCallbackProc
// Set the current selection of the browse for folder dialog.
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (BFFM_INITIALIZED == uMsg)
    {
        CPreferencesDlg* pDlg = (CPreferencesDlg*)lpData;
        ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CPreferencesDlg)));
		
		basic_string<TCHAR> CaptureLocation = pDlg->GetCaptureLocation();
		if (CaptureLocation.length())
		{
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR)CaptureLocation.c_str());
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
// OnInitDialog
//
BOOL CPreferencesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_CaptureLocationCtrl.SetWindowText(m_CaptureLocation.c_str());
	m_CaptureFilenameCtrl.SetWindowText(m_CaptureFilename.c_str());

	PopulateVideoFormatControl();
	PopulateAudioFormatControl();

	m_MuteAudioCtrl.SetCheck((true == m_bMuteAudio) ? BST_CHECKED : BST_UNCHECKED);

	PopulateCompressionControl();

	return TRUE;
}

//-----------------------------------------------------------------------------
// DestroyWindow
//
BOOL CPreferencesDlg::DestroyWindow()
{
	// Free the mediatypes attached to format controls
	int Count = m_VideoFormatCtrl.GetCount();
	if (Count)
	{
		for (int Item=0; Item<Count; ++Item)
		{
			DeleteMediaType((AM_MEDIA_TYPE*)m_VideoFormatCtrl.GetItemData(Item));
		}
		m_VideoFormatCtrl.ResetContent();
	}

	Count = m_AudioFormatCtrl.GetCount();
	if (Count)
	{
		for (int Item=0; Item<Count; ++Item)
		{
			DeleteMediaType((AM_MEDIA_TYPE*)m_AudioFormatCtrl.GetItemData(Item));
		}
		m_AudioFormatCtrl.ResetContent();
	}

	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
// OnBnClickedButtonBrowse
// Display a browse for folder dialog and use the callback to set the view
// to the current capture location folder.
void CPreferencesDlg::OnBnClickedButtonBrowse()
{
	TCHAR DisplayName[MAX_PATH] = {0};
	BROWSEINFO bi = {0};

	bi.hwndOwner = m_hWnd;
	bi.pszDisplayName = DisplayName;
	bi.lpszTitle = TEXT("Select capture location folder");
	bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = reinterpret_cast<LPARAM>(this);

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl)
	{
		TCHAR buf[MAX_PATH];
		if (SHGetPathFromIDList(pidl, buf))
		{
			m_CaptureLocation = buf;
			m_CaptureLocationCtrl.SetWindowText(m_CaptureLocation.c_str());
		}

		IMalloc* pMalloc;
		SHGetMalloc(&pMalloc);
		pMalloc->Free(pidl);
		pMalloc->Release();
	}
}

//-----------------------------------------------------------------------------
// OnEnChangeEditCapturelocation
// Update the location string as the edit control is modified.
void CPreferencesDlg::OnEnChangeEditCapturelocation()
{
	TCHAR buf[MAX_PATH];
	m_CaptureLocationCtrl.GetWindowText(buf, MAX_PATH);
	m_CaptureLocation = buf;
}

//-----------------------------------------------------------------------------
// OnEnChangeEditCapturefilename
// Update the file name string as the edit control is modified.
void CPreferencesDlg::OnEnChangeEditCapturefilename()
{
	TCHAR buf[MAX_PATH];
	m_CaptureFilenameCtrl.GetWindowText(buf, MAX_PATH);
	m_CaptureFilename = buf;
}

//-----------------------------------------------------------------------------
// PopulateVideoFormatControl
// Fill video format control with a selection of video formats supported by
// the capture device.
HRESULT CPreferencesDlg::PopulateVideoFormatControl()
{
	HRESULT hr = S_OK;

	if (m_pVideoCapture)
	{
		// locate the video capture pin and QI for stream control
		CComPtr<IAMStreamConfig> pISC = NULL;
		hr = CDSUtils::FindPinInterface(m_pVideoCapture, &MEDIATYPE_Video, PINDIR_OUTPUT, __uuidof(IAMStreamConfig), reinterpret_cast<void**>(&pISC));
	    if (SUCCEEDED(hr))
	    {
			// loop through all the capabilities (video formats) and populate the control
			int count, size;
			hr = pISC->GetNumberOfCapabilities(&count, &size);
			if (SUCCEEDED(hr))
			{
				if (sizeof(VIDEO_STREAM_CONFIG_CAPS) == size)
				{
					// Initialise the default video format.
					REFERENCE_TIME rtAvgTimePerFrameDefault = CUtils::GetAvgTimePerFrame(&m_VideoFormat);
					BITMAPINFOHEADER bmihDefault = {0};
					BITMAPINFOHEADER* pbmihDefault = CUtils::GetBMIHeader(&m_VideoFormat);
					if (rtAvgTimePerFrameDefault && pbmihDefault)
					{
						bmihDefault = *pbmihDefault;
					}
					else
					{
						// Default to 8-bit NTSC 29.97fps.
						rtAvgTimePerFrameDefault = 333667;
						bmihDefault.biWidth = 720;
						bmihDefault.biHeight = 486;
						bmihDefault.biBitCount = 16;
					}

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
								int n = m_VideoFormatCtrl.AddString(buffer);
								// store media type pointer in item's data section
								m_VideoFormatCtrl.SetItemData(n, (DWORD_PTR)pmt);
								
								// set default format
								if ((pvih->AvgTimePerFrame == rtAvgTimePerFrameDefault) &&
									(pvih->bmiHeader.biWidth == bmihDefault.biWidth) &&
									(pvih->bmiHeader.biHeight == bmihDefault.biHeight) &&
									(pvih->bmiHeader.biBitCount == bmihDefault.biBitCount))
								{
									m_VideoFormatCtrl.SetCurSel(n);
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
			}
		}
	}
	else
	{
		hr = E_POINTER;
	}

	if (0 == m_VideoFormatCtrl.GetCount())
	{
		m_VideoFormatCtrl.AddString(TEXT("<No capture device>"));
		m_VideoFormatCtrl.SetCurSel(0);
		m_VideoFormatCtrl.EnableWindow(FALSE);
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// PopulateAudioFormatControl
// Fill audio format control with a selection of audio formats supported by
// the capture device.
HRESULT CPreferencesDlg::PopulateAudioFormatControl()
{
	HRESULT hr = S_OK;

	if (m_pAudioCapture)
	{
		// locate the audio capture pin and QI for stream control
		CComPtr<IAMStreamConfig> pISC = NULL;
		hr = CDSUtils::FindPinInterface(m_pAudioCapture, &MEDIATYPE_Audio, PINDIR_OUTPUT, __uuidof(IAMStreamConfig), reinterpret_cast<void**>(&pISC));
	    if (SUCCEEDED(hr))
	    {
			// loop through all the capabilities (audio formats) and populate the control
			int count, size;
			hr = pISC->GetNumberOfCapabilities(&count, &size);
			if (SUCCEEDED(hr))
			{
				if (sizeof(AUDIO_STREAM_CONFIG_CAPS) == size)
				{
					// Initialise the default audio format.
					WAVEFORMATEX wfexDefault = {0};
					WAVEFORMATEX* pwfexDefault = (WAVEFORMATEX*)m_AudioFormat.Format();
					if (pwfexDefault)
					{
						wfexDefault = *pwfexDefault;
					}
					else
					{
						// Default to 2 channel 48k PCM audio.
						wfexDefault.wFormatTag = WAVE_FORMAT_PCM;
						wfexDefault.nChannels = 2;
						wfexDefault.nSamplesPerSec = 48000;
						wfexDefault.nAvgBytesPerSec = 192000;
					}
					
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
							int n = m_AudioFormatCtrl.AddString(buffer);
							// store media type pointer in item's data section
							m_AudioFormatCtrl.SetItemData(n, (DWORD_PTR)pmt);

							// set default format
							if ((pwfex->wFormatTag == wfexDefault.wFormatTag) &&
								(pwfex->nChannels == wfexDefault.nChannels) &&
								(pwfex->nSamplesPerSec == wfexDefault.nSamplesPerSec) &&
								(pwfex->nAvgBytesPerSec == wfexDefault.nAvgBytesPerSec))
							{
								m_AudioFormatCtrl.SetCurSel(n);
								pISC->SetFormat(pmt);
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

	if (0 == m_AudioFormatCtrl.GetCount())
	{
		m_AudioFormatCtrl.AddString(TEXT("<No capture device>"));
		m_AudioFormatCtrl.SetCurSel(0);
		m_AudioFormatCtrl.EnableWindow(FALSE);
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// PopulateCompressionControl
// Fill compression control with a selection of video compressors, locate the
// encoders and add them to the combo box if they exist.
HRESULT CPreferencesDlg::PopulateCompressionControl()
{
	m_CompressionCtrl.ResetContent();
	
	int n = m_CompressionCtrl.AddString(TEXT("Uncompressed"));
	m_CompressionCtrl.SetItemData(n, (DWORD)ENC_NONE);

	HRESULT hr = S_OK;
	IBaseFilter* pFilter = NULL;

	BITMAPINFOHEADER* pbmih = CUtils::GetBMIHeader(m_VideoFormat);	// For selectively displaying compression types depending upon the current video format.
	if (pbmih && (720 >= pbmih->biWidth))
	{
		// Search for the DV encoder, WM encoder, DeckLink MJPEG encoder and DeckLink still sequence writer filters.
		hr = CoCreateInstance(CLSID_DVVideoEnc, 0, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), reinterpret_cast<void**>(&pFilter));
		if (SUCCEEDED(hr))
		{
			n = m_CompressionCtrl.AddString(TEXT("DV Video Encoder"));
			m_CompressionCtrl.SetItemData(n, (DWORD)ENC_DV);
			SAFE_RELEASE(pFilter);
		}

		hr = CoCreateInstance(CLSID_WMAsfWriter, 0, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), reinterpret_cast<void**>(&pFilter));
		if (SUCCEEDED(hr))
		{
			n = m_CompressionCtrl.AddString(TEXT("Windows Media Encoder"));
			m_CompressionCtrl.SetItemData(n, (DWORD)ENC_WM);
			SAFE_RELEASE(pFilter);
		}
	}

	hr = CoCreateInstance(CLSID_DecklinkMJPEGEncoderFilter, 0, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), reinterpret_cast<void**>(&pFilter));
	if (SUCCEEDED(hr))
	{
		n = m_CompressionCtrl.AddString(TEXT("Decklink MJPEG Encoder"));
		m_CompressionCtrl.SetItemData(n, (DWORD)ENC_MJ);
		SAFE_RELEASE(pFilter);
	}

	hr = CoCreateInstance(CLSID_DecklinkStillSequenceSinkFilter, 0, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), reinterpret_cast<void**>(&pFilter));
	if (SUCCEEDED(hr))
	{
		if (pbmih && (720 < pbmih->biWidth) && (30 == pbmih->biBitCount) && ('012r' == pbmih->biCompression))
		{
			// Cineon and DPX sequences can only be used for 10-bit RGB HD video formats.
			n = m_CompressionCtrl.AddString(TEXT("Decklink Cineon Still Sequence"));
			m_CompressionCtrl.SetItemData(n, (DWORD)ENC_SEQ_CIN);
		
			n = m_CompressionCtrl.AddString(TEXT("Decklink DPX Still Sequence"));
			m_CompressionCtrl.SetItemData(n, (DWORD)ENC_SEQ_DPX);
		}

		n = m_CompressionCtrl.AddString(TEXT("Decklink TGA Still Sequence"));
		m_CompressionCtrl.SetItemData(n, (DWORD)ENC_SEQ_TGA);

		n = m_CompressionCtrl.AddString(TEXT("Decklink BMP Still Sequence"));
		m_CompressionCtrl.SetItemData(n, (DWORD)ENC_SEQ_BMP);

		SAFE_RELEASE(pFilter);
	}

	// Set the selection to the current compression.
	int Index = GetIndexForCompression();
	if (-1 != Index)
	{
		m_CompressionCtrl.SetCurSel(Index);
	}
	else
	{
		// The current compression is not available so use another as a default.
		if ((ENC_SEQ_CIN == m_Compression) || (ENC_SEQ_DPX == m_Compression))
		{
			m_Compression = ENC_SEQ_TGA;
		}
		else if ((ENC_DV == m_Compression) || (ENC_WM == m_Compression))
		{
			m_Compression = ENC_MJ;
		}
		m_CompressionCtrl.SetCurSel(GetIndexForCompression());
	} 
	
	return S_OK;
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboVideoformat
//
void CPreferencesDlg::OnCbnSelchangeComboVideoformat()
{
	int sel = m_VideoFormatCtrl.GetCurSel();
	if (CB_ERR != sel)
	{
		CMediaType* pMediaType = (CMediaType*)m_VideoFormatCtrl.GetItemData(sel);
		m_VideoFormat.Set(*pMediaType);
	
		// Re-populate the compression control to reflect the change of format.
		// DV and WME are only supported for SD formats while Cineon and DPX still sequences
		// are only supported for 10-bit RGB HD video formats.
		PopulateCompressionControl();
		OnCbnSelchangeComboCompression();	// Update the file extension.
	}
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboAudioformat
//
void CPreferencesDlg::OnCbnSelchangeComboAudioformat()
{
	int sel = m_AudioFormatCtrl.GetCurSel();
	if (CB_ERR != sel)
	{
		CMediaType* pMediaType = (CMediaType*)m_AudioFormatCtrl.GetItemData(sel);
		m_AudioFormat.Set(*pMediaType);
	}
}

//-----------------------------------------------------------------------------
// OnBnClickedCheckMuteaudio
//
void CPreferencesDlg::OnBnClickedCheckMuteaudio()
{
	m_bMuteAudio = (BST_CHECKED == m_MuteAudioCtrl.GetCheck()) ? true : false;
}

//-----------------------------------------------------------------------------
// OnCbnSelchangeComboCompression
//
void CPreferencesDlg::OnCbnSelchangeComboCompression()
{
	int Compression = (int)m_CompressionCtrl.GetItemData(m_CompressionCtrl.GetCurSel());
	if (m_Compression != Compression)
	{
		m_Compression = Compression;

		// Remove the file extension if one exists as only the base filename is required.
		basic_string<TCHAR>::size_type Index = m_CaptureFilename.find(_T('.'));
		if (basic_string<TCHAR>::npos != Index)
		{
			m_CaptureFilename.erase(Index);
		}

		m_CaptureFilenameCtrl.SetWindowText(m_CaptureFilename.c_str());
	}
}

//-----------------------------------------------------------------------------
// GetIndexForCompression
//
int CPreferencesDlg::GetIndexForCompression(void)
{
	int Index = -1;
	int cItems = m_CompressionCtrl.GetCount();
	for (int Item=0; Item<cItems; ++Item)
	{
		if (m_CompressionCtrl.GetItemData(Item) == m_Compression)
		{
			Index = Item;
		}
	}
	return Index;
}

