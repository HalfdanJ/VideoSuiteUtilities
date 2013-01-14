// 
// GDCL Multigraph Framework
//
// GMFPreview Demo Application: implementation of main window class
//
// Copyright (c) GDCL 2004. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk


#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"
#include ".\maindlg.h"

// dialog to select capture device
class DeviceDlg : public CDialogImpl<DeviceDlg>
{
public:
	enum { IDD = IDD_DEVICE };

	BEGIN_MSG_MAP(DeviceDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_DEVICELIST, LBN_DBLCLK, OnLbnDblclkDevicelist)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	const TCHAR* DeviceName()
	{
		return m_strDevice;
	}
private:
	_bstr_t m_strDevice;
public:
	LRESULT OnLbnDblclkDevicelist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

LRESULT 
DeviceDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	SendDlgItemMessage(IDC_DEVICELIST, LB_RESETCONTENT, 0, 0);

    ICreateDevEnumPtr pCreate(CLSID_SystemDeviceEnum);
	IEnumMonikerPtr pEnum;
	HRESULT hr = pCreate->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr != S_OK) {
	    // hr == S_FALSE means pEnum == NULL because category empty
        return 0;
    }

	if (pEnum != NULL) {
        IMonikerPtr pMoniker;
        while(pEnum->Next(1, &pMoniker, NULL) == S_OK) {
			IPropertyBagPtr pBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);

			if (SUCCEEDED(hr)) {
				_variant_t var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL);
				if (SUCCEEDED(hr)) {
					_bstr_t str = var;
					SendDlgItemMessage(IDC_DEVICELIST, LB_ADDSTRING, 0, (LPARAM) (const TCHAR*)str);
				}
			}
        }
    }

	return TRUE;
}

LRESULT DeviceDlg::OnLbnDblclkDevicelist(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	OnOK(0, IDOK, NULL, bHandled);
	return 0;
}

LRESULT 
DeviceDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int idx = SendDlgItemMessage(IDC_DEVICELIST, LB_GETCURSEL, 0, 0);
	if (idx < 0)
	{
		m_strDevice = "";
	} else {
		int cch = SendDlgItemMessage(IDC_DEVICELIST, LB_GETTEXTLEN, idx, 0);
		smart_array<TCHAR> pName = new TCHAR[cch+1];
		SendDlgItemMessage(IDC_DEVICELIST, LB_GETTEXT, idx, (LPARAM)(TCHAR*)pName);
		m_strDevice = pName;
	}

	EndDialog(wID);
	return 0;
}

LRESULT 
DeviceDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}



// ---------------------------------


BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

    // check for non-registered DLLs rather than reporting obscure error codes later
    IGMFBridgeControllerPtr pBridge;
    HRESULT hr = pBridge.CreateInstance(__uuidof(GMFBridgeController));
    if (FAILED(hr))
    {
        MessageBox(TEXT("Please register GMFBridge.DLL with regsvr32"));
        CloseDialog(IDCANCEL);
    }
	return TRUE;
}


LRESULT CMainDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnBnClickedDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DeviceDlg dlg;
	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		RECT rc;
		::GetClientRect(GetDlgItem(IDC_PREVIEW), &rc);
		rc.top += 15;
		rc.left += 15;
		rc.right -= 15;
		rc.bottom -= 15;
		HRESULT hr = m_Previewer.SelectDevice(dlg.DeviceName(), GetDlgItem(IDC_PREVIEW), &rc);
		if (hr != S_OK)
		{
			TCHAR ach[256];
			wsprintf(ach, TEXT("SelectDevice failed (0x%x)"), hr);
			MessageBox(ach, TEXT("GMFPreview Error"));
		}
        if (m_strFolder.size() > 0)
        {
            MakeNewCaptureFile();
        }
	}
	return 0;
}

LRESULT CMainDlg::OnBnClickedFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// UI to select a folder to store capture files
    BROWSEINFO info;
    ZeroMemory(&info, sizeof(info));
    TCHAR  achDisplay[MAX_PATH];

	info.hwndOwner = m_hWnd;
    info.pszDisplayName = achDisplay;
    info.lpszTitle = TEXT("Select Folder for capture output files");
    info.ulFlags = BIF_EDITBOX;

    BOOL bOK = false;
    LPITEMIDLIST pidl = SHBrowseForFolder(&info);
	TCHAR achDir[MAX_PATH];
    if (pidl) {
		bOK = SHGetPathFromIDList(pidl, achDir);

		IMalloc* pMalloc;
		SHGetMalloc(&pMalloc);
		pMalloc->Free(pidl);
		pMalloc->Release();
    }

	if (bOK)
	{
		m_strFolder = achDir;
		m_indexFile = 0;

        if (m_Previewer.HasDevice())
        {
            MakeNewCaptureFile();
        }
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (!m_Previewer.HasDevice())
    {
        BOOL bOK;
        OnBnClickedDevice(0, 0, NULL, bOK);
        if (!m_Previewer.HasDevice())
        {
            return 0;
        }
    }
	if (m_strFolder.size() == 0)
	{
		BOOL bOK;
		OnBnClickedFolder(0, 0, NULL, bOK);
        if (m_strFolder.size() == 0)
        {
            return 0;
        }
	}

	HRESULT hr = m_Previewer.StartCapture();
	if (hr != S_OK)
	{
		TCHAR  ach[256];
		wsprintf(ach, TEXT("StartCapture failed (0x%x)"), hr);
		MessageBox(ach, TEXT("GMFPreview Error"));
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedStopcapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (!m_Previewer.HasDevice())
    {
        return 0;
    }
	HRESULT hr = m_Previewer.StopCapture();
	if (hr != S_OK)
	{
		TCHAR ach[256];
		wsprintf(ach, TEXT("StopCapture failed (0x%x)"), hr);
		MessageBox(ach, TEXT("GMFPreview Error"));
	} else {
		tstring s = m_Previewer.GetFilename();
		TCHAR ach[MAX_PATH * 2];
		wsprintf(ach, TEXT("Captured to %s"), s.c_str());
		MessageBox(ach, TEXT("GMFPreview capture"));
	}


	//  make new filename
	MakeNewCaptureFile();

	return 0;
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif


void
CMainDlg::MakeNewCaptureFile()
{
	// prepare next capture graph
	TCHAR achFile[MAX_PATH];
	while(true)
	{
		wsprintf(achFile, TEXT("%s\\Cap%d.avi"), m_strFolder.c_str(), ++m_indexFile);
		if (GetFileAttributes(achFile) == INVALID_FILE_ATTRIBUTES)
		{
			break;
		}
	}
	HRESULT hr = m_Previewer.SetNextFilename(achFile);
	if (hr != S_OK)
	{
		TCHAR ach[256];
		wsprintf(ach, TEXT("SetNextFilename failed (0x%x)"), hr);
		MessageBox(ach, TEXT("GMFPreview Error"));
	}
}


