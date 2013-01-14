//-----------------------------------------------------------------------------
// DecklinkExportToTapeDlg.h
//
// Desc: DirectShow export to tape sample
//
// Copyright (c) Blackmagic Design 2006. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include "BasePlayList.h"

//-----------------------------------------------------------------------------
// CDecklinkExportToTapeDlg 
//
class CDecklinkExportToTapeDlg : public CDialog
{
// Construction
public:
	CDecklinkExportToTapeDlg(CWnd* pParent = NULL);	// standard constructor
	DWORD Thread(void);

// Dialog Data
	enum { IDD = IDD_DECKLINKEXPORTTOTAPE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	afx_msg void OnBnClickedButtonAddfile();
	afx_msg void OnBnClickedButtonRemovefile();
	afx_msg void OnBnClickedButtonMovefileup();
	afx_msg void OnBnClickedButtonMovefiledown();
	afx_msg void OnBnClickedButtonSetinpoint();
	afx_msg void OnBnClickedButtonSetoutpoint();
	afx_msg void OnCbnSelchangeComboDevice();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonFrew();
	afx_msg void OnBnClickedButtonFfwd();
	afx_msg void OnBnClickedButtonEject();
	afx_msg void OnBnClickedButtonExport();
	afx_msg void OnBnClickedButtonAbort();
	afx_msg void OnLvnInsertitemListFiles(NMHDR* pNMHDR, LRESULT* pResult);

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
	CBasePlayList m_FileList;	// list of files to export/playout

	CComPtr<IGraphBuilder> m_pGraph;	// for building playback graphs
	CComQIPtr<IMediaEvent, &IID_IMediaEvent> m_pIMediaEvent;	// for monitoring graph events, e.g. EC_COMPLETE during playback
	CComQIPtr<IMediaControl, &IID_IMediaControl> m_pIMediaControl;	// for stopping, pausing and starting the graphs
	CComQIPtr<IMediaSeeking, &IID_IMediaSeeking> m_pIMediaSeeking;	// for monitoring current position during playback
	CComPtr<IBaseFilter> m_pVideoRenderer;	// currently selected DeckLink video renderer
	CComPtr<IBaseFilter> m_pAudioRenderer;	// currently selected DeckLink audio renderer
	CComQIPtr<IAMExtTransport, &IID_IAMExtTransport> m_pIExtTransport;	// for transport control and edit presets
	CComQIPtr<IAMTimecodeReader, &IID_IAMTimecodeReader> m_pITimecodeReader;	// for reading timecode from the deck
	CComQIPtr<IMediaFilter, &IID_IMediaFilter> m_pIMediaFilter;
	CComQIPtr<IReferenceClock, &IID_IReferenceClock> m_pIReferenceClock;

	DWORD m_ROTRegister;

	CBitmapButton m_BtnAddFile;
	CBitmapButton m_BtnRemoveFile;
	CBitmapButton m_BtnMoveFileUp;
	CBitmapButton m_BtnMoveFileDown;
	CBitmapButton m_BtnSetInpoint;
	CBitmapButton m_BtnSetOutpoint;
	CComboBox m_deviceCtrl;	// a list of external renderers
	CStatic m_TimecodeCtrl;	// current deck timecode
	CEdit m_InpointCtrl;	// clip inpoint
	CEdit m_OutpointCtrl;	// clip outpoint
	CStatic m_DurationCtrl;	// clip duration
	CEdit m_OffsetCtrl;
	CBitmapButton m_BtnFRew;
	CBitmapButton m_BtnPlay;
	CBitmapButton m_BtnFFwd;
	CBitmapButton m_BtnStop;
	CBitmapButton m_BtnEjct;

	CMediaType m_mtProject;	// project format
	VIDEOINFOHEADER* m_pvih;

	HANDLE m_hThread;
	HANDLE m_hExitEvent;

	bool m_bExport;

	void EnableControls(BOOL enable);
	HRESULT EnumerateDevices(void);

	HRESULT CreateThreads(void);
	HRESULT DestroyThreads(void);
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	HRESULT CreateBaseGraph(void);
	HRESULT CreateGraph(const TCHAR* pFilename);
	HRESULT DestroyGraph(bool bRemoveRenderers);

	HRESULT CheckTransport(void);
	HRESULT CreateEditPropertySet(CTimecode& Inpoint, CTimecode& Outpoint, int Preroll);

	void UpdateTimecodeCtrl(CTimecode& Timecode);
};
