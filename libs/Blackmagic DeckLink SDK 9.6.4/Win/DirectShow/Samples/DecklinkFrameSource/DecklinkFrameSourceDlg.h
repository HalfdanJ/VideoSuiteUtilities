//------------------------------------------------------------------------------
// DecklinkFrameSourceDlg.h
//
// Desc: DirectShow sample code - Application to deliver frames
//		 to a DirectShow graph with the Decklink video render filter
//		 via a custom interface on a push source filter.
//
// Copyright (c) Blackmagic Design 2005.  All rights reserved.
//------------------------------------------------------------------------------

#pragma once

#include "DecklinkFilters_h.h"
#include "afxwin.h"

// CDecklinkFrameSourceDlg dialog
class CDecklinkFrameSourceDlg : public CDialog
{
// Construction
public:
	CDecklinkFrameSourceDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DECKLINKFRAMESOURCE_DIALOG };

	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnCbnSelchangeComboDevice();
	afx_msg void OnCbnSelchangeComboVideoformat();
	afx_msg void OnCbnSelchangeComboAudioformat();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	BOOL DestroyWindow();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<IMediaControl> m_pControl;
	CComPtr<IBaseFilter> m_pDecklinkVideoSource;
	CComPtr<IBaseFilter> m_pDecklinkAudioSource;
	CComPtr<IBaseFilter> m_pInfinitePinT;
	CComPtr<IBaseFilter> m_pVideoRenderer;
	CComPtr<IDecklinkPushSource2> m_pIPushSourceVideo;
	CComPtr<IDecklinkPushSource2> m_pIPushSourceAudio;

	DWORD m_ROTRegister;

	HANDLE m_hThread;
	HANDLE m_hThreadEvent;

	CStatic m_Preview;
	CStatic m_FrameCountCtrl;
	unsigned long m_FrameCount;
	unsigned long m_SampleCount;
	CComboBox m_videoFormatCtrl;
	CComboBox m_audioFormatCtrl;
	CComboBox m_deviceCtrl;

    ULONG_PTR m_gdiplusToken; // GDI+ initialization token

	HRESULT CreateGraph(void);
	HRESULT DestroyGraph(void);
	HRESULT PopulateDeviceControl(void);
	HRESULT PopulateVideoControl(void);
	HRESULT PopulateAudioControl(void);

	static DWORD WINAPI ThreadFn(LPVOID lpParam);
	DWORD Thread();

	HRESULT WriteToBuffer(int xpos, int ypos, LPCTSTR pszText, BYTE* pData, BITMAPINFOHEADER* pbmih);
	void GetVideoInfoParameters(const VIDEOINFOHEADER* pvih, BYTE* const pbData, DWORD* pdwWidth, DWORD* pdwHeight, LONG* plStrideInBytes,  BYTE** ppbTop, bool bYuv);
	HRESULT FillAudioBuffer(WAVEFORMATEX* pwfex, REFERENCE_TIME* prtAvgTimePerFrame, BYTE* pbData, DWORD cbData);
};
