//-----------------------------------------------------------------------------
// DecklinkHistogramDlg.h
//
// Desc: DirectShow histogram sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include "HistogramCtrl.h"
#include "SGCallbackHandler.h"

// CDecklinkHistogramDlg dialog
class CDecklinkHistogramDlg : public CDialog
{
// Construction
public:
	CDecklinkHistogramDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DECKLINKHISTOGRAM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnCbnSelchangeComboDevice();
	afx_msg void OnCbnSelchangeComboVideoformat();

private:
	CComPtr<IGraphBuilder> m_pGraph;
	CComQIPtr<IMediaControl, &IID_IMediaControl> m_pControl;
	IBaseFilter* m_pVideoCapture;
	CComPtr<IBaseFilter> m_pSampleGrabber;
	CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> m_pISampleGrabber;
	CComPtr<IBaseFilter> m_pNullRenderer;
	CComPtr<IBaseFilter> m_pVideoRenderer;
	CComPtr<IBaseFilter> m_pSmartT;

	DWORD m_ROTRegister;

	CHistogramCtrl* m_pHistogramCtrl;	// custom histogram control
	CComboBox m_deviceCtrl;
	CComboBox m_videoFormatCtrl;
	CSGCallbackHandler* m_pSGCallback;

	HANDLE m_hThread;
	HANDLE m_hStopThreadEvent;
	DWORD m_pollPeriod;

	HRESULT BuildGraph(void);
	HRESULT DestroyGraph(void);

	HRESULT PopulateDeviceControl(const GUID* pCategory, CComboBox* pCtrl);
	HRESULT PopulateVideoControl(void);
	static DWORD WINAPI ThreadWrapper(LPVOID lpParameter);
	DWORD Thread(void);
};
