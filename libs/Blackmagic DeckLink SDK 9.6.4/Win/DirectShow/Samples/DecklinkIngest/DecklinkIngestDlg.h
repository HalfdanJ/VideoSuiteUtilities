//-----------------------------------------------------------------------------
// DecklinkIngestDlg.h
//
// Desc: DirectShow ingest sample
//
// Copyright (c) Blackmagic Design 2007. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include "GMFBridge_h.h"
#include "StillGrabber.h"
#include "TimecodeGrabber.h"
#include "ClipList.h"

//-----------------------------------------------------------------------------
// CDecklinkIngestDlg 
//
class CDecklinkIngestDlg : public CDialog
{
// Construction
public:
	CDecklinkIngestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DECKLINKINGEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	afx_msg void OnFileImportEDL();
	afx_msg void OnEditPreferences();
	afx_msg void OnDeviceMenu(UINT nID);
	afx_msg void OnDeviceInfo();
	afx_msg void OnAboutBox();
	afx_msg void OnBnClickedButtonLogclip();
	afx_msg void OnBnClickedButtonRemoveclip();
	afx_msg void OnBnClickedButtonMoveclipup();
	afx_msg void OnBnClickedButtonMoveclipdown();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonFrew();
	afx_msg void OnBnClickedButtonFfwd();
	afx_msg void OnBnClickedButtonEject();
	afx_msg void OnBnClickedButtonCaptureStill();
	afx_msg void OnBnClickedButtonCaptureNow();
	afx_msg void OnBnClickedButtonBatchCapture();
	afx_msg void OnBnClickedButtonAbort();

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
	static const int MAXDEVICES;	// Maximum number of supported devices.

	CComPtr<IGraphBuilder> m_pSourceGraph;	// For building source side capture graphs.
	CComPtr<IGraphBuilder> m_pSinkGraph;	// For building sink side capture graphs.
	CComPtr<IGMFBridgeController> m_pIGMFBridgeController;	// GDCL Multigraph Framework object to enable continuous preview during capture to multiple files.

	// Filters and interfaces for the source (capture) side of the bridge.
	CComQIPtr<IMediaEvent, &IID_IMediaEvent> m_pIMediaEvent;	// for monitoring graph events, e.g. EC_STREAM_CONTROL_STARTED, EC_STREAM_CONTROL_STOPPED, etc. during file writing.
	CComPtr<IBaseFilter> m_pSmartT;
	CComPtr<IBaseFilter> m_pVideoRenderer;	// Desktop preview.
	CComPtr<IBaseFilter> m_pBridgeSink;	// Source side filter for the GMF bridge.
	CComPtr<IBaseFilter> m_pVideoDevice;	// Currently selected DeckLink video capture filter.
	CComPtr<IBaseFilter> m_pAudioDevice;	// Currently selected DeckLink audio capture filter.
	CComPtr<IBaseFilter> m_pSampleGrabberStills;	// Sample grabber filter for capturing stills.
	CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> m_pISampleGrabberStills;	// Sample grabber interface for capturing stills.
	CStillGrabber* m_pStillGrabber;	// Sample grabber callback for capturing stills.
	CComPtr<IBaseFilter> m_pSampleGrabberTimecode;	// Sample grabber filter for capturing Timecode.
	CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> m_pISampleGrabberTimecode;	// Sample grabber interface for capturing Timecode.
	CTimecodeGrabber* m_pTimecodeGrabber;	// Sample grabber callback for capturing Timecode.
	CComPtr<IBaseFilter> m_pNullRenderer;	// Stream sink for the timecode stream.

	// Filters and interfaces for the sink (file writing) side of the bridge.
	CComPtr<IBaseFilter> m_pBridgeSource;	// Sink side filter for the GMF bridge.
	CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> m_pIFileSinkFilter;	// Interface to set the capture file name.

	CComQIPtr<IAMExtTransport, &IID_IAMExtTransport> m_pIExtTransport;	// For transport control.

	LPWSTR m_pDeviceNameVideo;	// The filter name of the currently selected device.
#ifdef _DEBUG
	DWORD m_ROTRegisterSource;
	DWORD m_ROTRegisterSink;
#endif
	CClipList m_ClipList;	// A list of clips to ingest.
	basic_string<TCHAR> m_CaptureFilePath;	// The folder location for captured clips.
	basic_string<TCHAR> m_CaptureFilename;	// The base filename for 'capture now' clips.
	basic_string<TCHAR> m_CaptureFileExtension;	// File extension which is set according to the compression type.
	basic_string<TCHAR> m_CaptureTapename;
	CMediaType m_VideoFormat;	// The format of the captured video.
	CMediaType m_AudioFormat;	// The format of the captured audio.
	bool m_bMuteAudio;	// Specify whether audio is to be captured.
	int m_Compression;	// The selected video compression; uncompressed, DV, MJPEG and WM.
	int m_Device;	// The index of the currently selected capture device.
	
	int m_FileIndex;

	CBitmapButton m_BtnAddClip;
	CBitmapButton m_BtnRemoveClip;
	CBitmapButton m_BtnMoveClipUp;
	CBitmapButton m_BtnMoveClipDown;
	CStatic m_TimecodeCtrl;	// current deck timecode
	CEdit m_InpointCtrl;	// clip inpoint
	CEdit m_OutpointCtrl;	// clip outpoint
	CStatic m_DurationCtrl;	// clip duration
	CBitmapButton m_BtnFRew;
	CBitmapButton m_BtnPlay;
	CBitmapButton m_BtnFFwd;
	CBitmapButton m_BtnStop;
	CBitmapButton m_BtnEjct;
	CEdit m_TapenameCtrl;	// clip tape name
	CEdit m_ClipnameCtrl;	// clip clip name

	CStatusBarCtrl m_StatusBarCtrl;
	CProgressCtrl m_ProgressCtrl;

	HANDLE m_hThread;
	HANDLE m_hExitEvent;

	bool m_bBatchCapture;

	CRegUtils m_RegUtils;

	HRESULT CreateDeviceMenu(void);
	int FindMenuItem(CMenu* pMenu, LPCTSTR MenuString);

	HRESULT CreateThreads(void);
	HRESULT DestroyThreads(void);
	DWORD Thread(void);
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	HRESULT CheckTransport(void);
	void UpdateTimecodeCtrl(CTimecode& Timecode);

	HRESULT CreateCaptureSourceGraph(void);
	HRESULT ConfigureSourceGraph(REFERENCE_TIME* prtStart, REFERENCE_TIME* prtStop);
	HRESULT CreateCaptureSinkGraph(LPCTSTR pszFilename);
	HRESULT CreateUncompressedCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter);
	HRESULT CreateDVCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter);
	HRESULT CreateWMCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter);
	HRESULT ConfigureWMEncoder(IBaseFilter* pASFWriter);
	HRESULT CreateMJPEGCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter);
	HRESULT CreateStillSequenceCaptureSinkGraph(IBaseFilter* pAVIMux, IBaseFilter* pFileWriter);
	HRESULT DestroySourceGraph(void);
	HRESULT DestroySinkGraph(void);
	HRESULT StartCapture(void);
	HRESULT StopCapture(void);
	HRESULT EnableSampleGrabberCallback(bool bEnable);

	void EnableWindows(const int* pCtrlIDs, int cCtrlIDs, BOOL bEnable);

	void QueryRegistry(void);
	void SavePreferencesToRegistry(void);
	
	void SetFileExtension(basic_string<TCHAR>& Filename);
	void ResetFileIndex(void) { m_FileIndex = 1; }

	void CreateStatusBarControl(void);
};
