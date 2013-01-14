//-----------------------------------------------------------------------------
// DecklinkKeyDlg.h
//
// Desc: DirectShow keying sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CDecklinkKeyDlg dialog
class CDecklinkKeyDlg : public CDialog
{
// Construction
public:
	CDecklinkKeyDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DECKLINKKEY_DIALOG };

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
	afx_msg void OnCbnSelchangeComboDevice();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedRadioKey();
	afx_msg void OnBnClickedButtonBrowse();
	DECLARE_MESSAGE_MAP()

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void HandleGraphEvent();

private:
	CComPtr<IGraphBuilder> m_pGraph;	// the graph
	CComQIPtr<IMediaControl, &IID_IMediaControl> m_pControl;	// graph control interface
	CComQIPtr<IMediaEventEx, &IID_IMediaEventEx> m_pMediaEvent;	// for the preview window
	CComQIPtr<IMediaSeeking, &IID_IMediaSeeking> m_pIMediaSeeking;	// Media seeking interface
	CComQIPtr<IMediaFilter, &IID_IMediaFilter> m_pIMediaFilter;

	CComPtr<IBaseFilter> m_pCheckerSource;	// a source that provides a checker background to preview the keyed source
	CComPtr<IAMStreamControl> m_pIStreamControl;	// Stream control interface to turn off checker source so AVI movies can loop
	CComPtr<IBaseFilter> m_pInfiniteTee;	// infinite tee filter
	CComPtr<IBaseFilter> m_pVideoRenderer;	// screen renderer to preview keyed image
	CComQIPtr<IVMRMixerControl, &IID_IVMRMixerControl> m_pIVMRMixerControl;	// screen renderer mixer control to blend checker image with key in the preview window
	CComQIPtr<IDecklinkKeyer, &IID_IDecklinkKeyer> m_pIDecklinkKeyer;	// Decklink keyer control interface
	CComQIPtr<IReferenceClock, &IID_IReferenceClock> m_pIReferenceClock;

	DWORD m_ROTRegister;

	CStatic m_preview;
	CComboBox m_deviceCtrl;
	CComboBox m_frameRateCtrl;
	CSliderCtrl m_alphaCtrl;
	CStatic m_alphaDisplayCtrl;
	CEdit m_filenameCtrl;
	CString m_filename;

	typedef enum tagKeyControl { KEY_OFF, KEY_INT, KEY_EXT }KEYMODE;
	KEYMODE m_keyMode;
	int m_keyLevel;

	CRegUtils m_regUtils;

	HRESULT BuildGraph(void);
	HRESULT DestroyGraph();
	void QueryRegistry(void);
	void UpdateUI(void);
	HRESULT EnumerateDevices(void);
	HRESULT PopulateFrameRateCtrl(CMediaType& mediaType);
public:
	afx_msg void OnCbnSelchangeComboFramerate();
};
