// 
// GDCL Multigraph Framework
//
// GMFPlay Demo Application: Main window class declaration
//
// Copyright (c) GDCL 2004. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk


#pragma once

#include "player.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	CMainDlg()
	: m_bTracking(false)
	{
	}
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	enum
	{
		MSG_ENDOFSEGMENT	=	WM_USER+100,
		MSG_DSEVENT			=	WM_USER+101,
	};

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_ADD, BN_CLICKED, OnBnClickedAdd)
		COMMAND_HANDLER(IDC_PLAY, BN_CLICKED, OnBnClickedPlay)
		COMMAND_HANDLER(IDC_PAUSE, BN_CLICKED, OnBnClickedPause)
		COMMAND_HANDLER(IDC_STOP, BN_CLICKED, OnBnClickedStop)
		COMMAND_HANDLER(IDC_LOOP, BN_CLICKED, OnBnClickedLoop)
        COMMAND_HANDLER(IDC_LIST1, LBN_SELCHANGE, OnListSelChange)
        COMMAND_HANDLER(IDC_LIST1, LBN_SELCANCEL, OnListSelChange)
        COMMAND_HANDLER(IDC_LIMITS, BN_CLICKED, OnBnClickedLimits)
        
		// private message for end of segment
		MESSAGE_HANDLER(MSG_ENDOFSEGMENT, OnEndOfSegment)
		MESSAGE_HANDLER(MSG_DSEVENT, OnDSEvent)

		// update current position
		MESSAGE_HANDLER(WM_TIMER, OnTimer);
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
            COMMAND_HANDLER(IDC_PLAYNEXT, BN_CLICKED, OnBnClickedPlaynext)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);
	LRESULT OnBnClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPlay(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedLoop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEndOfSegment(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDSEvent(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnListSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	smart_ptr<ClipPlayer> m_pPlayer;

	// slider-dragging support
	bool m_bTracking;
	FILTER_STATE m_State;
public:
    LRESULT OnBnClickedLimits(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedPlaynext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
