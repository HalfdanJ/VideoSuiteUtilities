//-----------------------------------------------------------------------------
// DecklinkCaptureDlg.h
//
// Desc: DirectShow capture sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once
#include "afxwin.h"

// CDecklinkCaptureDlg dialog
class CDecklinkCaptureDlg : public CDialog
{
// Construction
public:
	CDecklinkCaptureDlg(CWnd* pParent = NULL);	// standard constructor

	DWORD InputStatusThread(void);

// Dialog Data
	enum { IDD = IDD_DECKLINKCAPTURE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	afx_msg void OnCbnSelchangeComboVideodevice();
	afx_msg void OnCbnSelchangeComboAudiodevice();
	afx_msg void OnCbnSelchangeComboVideoformats();
	afx_msg void OnCbnSelchangeComboAudioformats();
	afx_msg void OnBnClickedCheckAudiomute();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonCapture();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnCbnSelchangeComboCompression();

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

private:
	CComPtr<IGraphBuilder> m_pGraph;
	CComQIPtr<IMediaControl, &IID_IMediaControl> m_pControl;
	CComPtr<IBaseFilter> m_pVideoCapture;
	CComPtr<IBaseFilter> m_pAudioCapture;
	CComPtr<IBaseFilter> m_pVideoRenderer;
	CComPtr<IBaseFilter> m_pSmartT;

	DWORD m_ROTRegister;

	CComboBox m_videoDeviceCtrl;
	CComboBox m_audioDeviceCtrl;
	CComboBox m_videoFormatCtrl;
	CComboBox m_audioFormatCtrl;
	CComboBox m_compressionCtrl;
	BOOL m_bAudioMute;
	int m_compressor;
	BOOL m_bEnableCompressionCtrl;
	enum { ENC_NONE, ENC_DV, ENC_WM, ENC_MJ };

	CString m_captureFile;
	CEdit m_captureFileCtrl;

	VIDEOINFOHEADER m_vihDefault;
	WAVEFORMATEX m_wfexDefault;
	CRegUtils m_regUtils;

	CComQIPtr<IDecklinkStatus, &IID_IDecklinkStatus> m_pIDecklinkStatus;
	HANDLE m_hStopInputStatusThreadEvent;
	HANDLE m_hInputStatusThread;
	HANDLE m_hInputStatusChangeEvent;
	CCritSec m_csInputStatusLock;

	HRESULT CreatePreviewGraph();
	HRESULT CreateCaptureGraph();
	HRESULT CreateUncompressedCaptureGraph();
	HRESULT CreateDVCaptureGraph();
	HRESULT CreateWMCaptureGraph();
	HRESULT CreateMJCaptureGraph();
	HRESULT ConfigureWMEncoder(IBaseFilter* pASFWriter);
	HRESULT DestroyGraph();

	HRESULT PopulateDeviceControl(const GUID* pCategory, CComboBox* pCtrl);
	HRESULT PopulateVideoControl();
	HRESULT PopulateAudioControl();
	HRESULT PopulateCompressionControl();

	void EnableControls(void);
	void DisableControls(void);
	
	void QueryRegistry(void);

	static DWORD WINAPI InputStatusThreadWrapper(LPVOID lpParam);
};
