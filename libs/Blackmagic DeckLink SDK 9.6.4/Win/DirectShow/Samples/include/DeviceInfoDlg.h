#pragma once

#include "afxcmn.h"

// CDeviceInfoDlg dialog

class CDeviceInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CDeviceInfoDlg)

public:
	CDeviceInfoDlg(LPCWSTR pDeviceName, const GUID* pCategory, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDeviceInfoDlg();

// Dialog Data
	enum { IDD = IDD_DEVICEINFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CListCtrl m_DeviceInfoCtrl;
	basic_string<WCHAR> m_DeviceName;
	const GUID* m_pCategory;
};
