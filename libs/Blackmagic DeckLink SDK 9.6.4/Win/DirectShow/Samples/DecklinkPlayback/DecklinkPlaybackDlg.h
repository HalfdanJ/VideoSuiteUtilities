//-----------------------------------------------------------------------------
// DecklinkPlaybackDlg.h
//
// Desc: DirectShow playback sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "SGCallbackHandler.h"

// CDecklinkPlaybackDlg dialog
class CDecklinkPlaybackDlg : public CDialog
{
// Construction
public:
	CDecklinkPlaybackDlg(CWnd* pParent = NULL);	// standard constructor
	DWORD Thread(void);

// Dialog Data
	enum { IDD = IDD_DECKLINKPLAYBACK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonBrowse2();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckLoopPlayback();
	afx_msg void OnCbnSelchangeComboDevice();

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

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void HandleGraphEvent();

private:
	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<IMediaDet> m_pIMediaDetector;
	CComQIPtr<IMediaEventEx, &IID_IMediaEventEx> m_pMediaEvent;	// for EC_COMPLETE notification
	CComQIPtr<IMediaControl, &IID_IMediaControl> m_pControl;
	CComQIPtr<IMediaSeeking, &IID_IMediaSeeking> m_pMediaSeeking;

	CComPtr<IBaseFilter> m_pVideoRenderer;
	CComPtr<IBaseFilter> m_pSampleGrabber;
	CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> m_pISampleGrabber;
	CSGCallbackHandler* m_pSGCallback;

	CComQIPtr<IMediaFilter, &IID_IMediaFilter> m_pIMediaFilter;
	CComQIPtr<IReferenceClock, &IID_IReferenceClock> m_pIReferenceClock;

	DWORD m_ROTRegister;

	CString m_renderFile;
	CString m_blendFile;
	CEdit m_renderFileCtrl;
	CEdit m_blendFileCtrl;
	CSliderCtrl m_shuttleCtrl;
	CComboBox m_deviceCtrl;

	HANDLE m_hThread;
	HANDLE m_hExitEvent;

	BOOL m_bLoopPlayback;
	BOOL m_bCanSeek;

	CRegUtils m_regUtils;

	HRESULT CreateGraph(void);
	HRESULT CreateGraphGeneric(void);
	HRESULT CreateGraphWM(void);
	HRESULT DestroyGraph();

	void EnableControls(void);
	void DisableControls(void);
	
	HRESULT CreateThreads(void);
	HRESULT DestroyThreads(void);
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	void UpdateDisplay(int ctrlID, REFERENCE_TIME* pTime);
	void QueryRegistry(void);
	HRESULT EnumerateDevices(void);
};
