// 
// GDCL Multigraph Framework
//
// GMFPlay Demo Application
// MainDlg.cpp:	implementation of main dialog window class
//
// Copyright (c) GDCL 2004. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include ".\maindlg.h"

#include <sstream>

// dialog to set start/stop time on clip
class LimitsDlg : public CDialogImpl<LimitsDlg>
{
public:
	enum { IDD = IDD_LIMITS };

	BEGIN_MSG_MAP(LimitsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LimitsDlg(long Duration, long tStart = 0, long tStop = -1)
	: m_Duration(Duration),
      m_Start(tStart),
      m_Stop(tStop)
	{
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(GetParent());
		char ach[256];
		wsprintf(ach, "Duration: %d seconds", m_Duration);
		SetDlgItemText(IDC_DURATIONTEXT, ach);
		SetDlgItemInt(IDC_START, m_Start);
        if (m_Stop == -1)
        {
            SetDlgItemText(IDC_STOP, "");
        } else 
        {
            SetDlgItemInt(IDC_STOP, m_Stop);
        }
		return TRUE;
	}
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_Start = GetDlgItemInt(IDC_START, NULL, false);
		if (SendDlgItemMessage(IDC_STOP, WM_GETTEXTLENGTH, 0, 0) == 0)
		{
			m_Stop = -1;
		} else {
			m_Stop = GetDlgItemInt(IDC_STOP, NULL, false);
		}

		EndDialog(wID);
		return 0;
	}

	long Start()
	{
		return m_Start;
	}
	long Stop()
	{
		return m_Stop;
	}

private:
	long m_Duration;
	long m_Start;
	long m_Stop;
};



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

    try {
        m_pPlayer = new ClipPlayer(m_hWnd, MSG_ENDOFSEGMENT, MSG_DSEVENT);
    } catch(HRESULT)
    {
        MessageBox("Please register GMFBridge.DLL with regsvr32");
        CloseDialog(IDCANCEL);
    }

	// keep slider in range 0 to 1000 and map duration to that
    SendDlgItemMessage(IDC_SLIDER1, TBM_SETRANGE, 0, (LPARAM) MAKELONG(0, 1000));

	// update slider UI every 200ms
	SetTimer(1, 200);

	return TRUE;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	m_pPlayer = NULL;

	DestroyWindow();
	::PostQuitMessage(nVal);
}

const LONGLONG NANOSECONDS = (1000000000);       // 10 ^ 9
const LONGLONG UNITS = (NANOSECONDS / 100);      // 10 ^ 7

LRESULT CMainDlg::OnBnClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	char szFile[MAX_PATH];
	szFile[0] = '\0';
	ofn.lStructSize = sizeof(OPENFILENAME); 
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile; 
	ofn.nMaxFile = sizeof(szFile); 
	ofn.Flags = OFN_FILEMUSTEXIST; 

	if (GetOpenFileName(&ofn)) 
	{
		ClipEntry* pClip;
		HRESULT hr = m_pPlayer->AddClip(ofn.lpstrFile, &pClip);
		if (FAILED(hr))
		{
			MessageBox("Cannot add clip: format not support", "Incompatible Clip");
			return 0;
		}

		string sDesc = pClip->Name();

		// duration, limits dlg -- set clip limits (in seconds)
		LimitsDlg dlg(long((pClip->Duration() + UNITS - 1) / UNITS));
		if (dlg.DoModal() == IDOK)
		{
			REFERENCE_TIME tStart = dlg.Start() * UNITS;
			REFERENCE_TIME tStop = pClip->Duration();
			if (dlg.Stop() > 0)
			{
				tStop = dlg.Stop() * UNITS;
			}
            m_pPlayer->SetClipLimits(pClip, tStart, tStop);

			// append range to clip description text
			char ach[128];
			wsprintf(ach, " [%d..%d]", long(tStart/UNITS), long(tStop/UNITS));
			sDesc += ach;
		}

		SendDlgItemMessage(IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)sDesc.c_str());
	}
	return 0;
}

LRESULT CMainDlg::OnBnClickedPlay(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	
	m_pPlayer->Play();
	return 0;
}

LRESULT CMainDlg::OnBnClickedPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pPlayer->Pause();
	return 0;
}

LRESULT CMainDlg::OnBnClickedStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_pPlayer->Stop();
	return 0;
}

LRESULT CMainDlg::OnBnClickedLoop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool bLoop = IsDlgButtonChecked(IDC_LOOP) == BST_CHECKED? true : false;
	m_pPlayer->SetLoop(bLoop);

	return 0;
}
	
LRESULT 
CMainDlg::OnEndOfSegment(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_pPlayer->OnEndOfSegment();
	return 0;
}
	
	
LRESULT 
CMainDlg::OnDSEvent(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_pPlayer->OnEvent();
	return 0;
}

LRESULT 
CMainDlg::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    REFERENCE_TIME tDuration = m_pPlayer->TotalDuration();
	int iSliderPos = 0;
	if (tDuration > 0)
	{
		REFERENCE_TIME tNow = m_pPlayer->CurrentPosition();
		iSliderPos = int(tNow * 1000 / tDuration);
	}
	SendDlgItemMessage(IDC_SLIDER1, TBM_SETPOS, true, iSliderPos);

	return 0;
}

// drag slider bar to position movie
LRESULT 
CMainDlg::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int op = LOWORD(wParam);
    int position = HIWORD(wParam);
	
	if (op == TB_THUMBTRACK)
	{
		// on mouse-down, pause the graph
		if (!m_bTracking)
		{
			// stop/pause/run state of render graph
			m_State = m_pPlayer->GetState();
			if (m_State == State_Running)
			{
				m_pPlayer->Pause();
			}
			m_bTracking = true;
		} else {
			// mid track: don't issue a seek until previous one has finished
			if (!m_pPlayer->IsCued())
			{
				return 0;
			}
		}
	}

	// end-track is just mouse-up; other ops require positioning
	if (op != TB_ENDTRACK)
	{
		if ((op != TB_THUMBPOSITION) && (op != TB_THUMBTRACK))
		{
			position = SendDlgItemMessage(IDC_SLIDER1, TBM_GETPOS, 0, 0);
			switch(op)
			{
			case TB_PAGEUP:
			case TB_LINEUP:
				position -= 100;	// 10% up
				break;
			case TB_PAGEDOWN:
			case TB_LINEDOWN:
				position += 100;
				break;
			}
			if (position < 0)
			{
				position = 0;
			}
			if (position > 1000)
			{
				position = 1000;
			}
		}
		// scale 0..1000 track bar range into total duration
		REFERENCE_TIME tSeek = REFERENCE_TIME(position) * m_pPlayer->TotalDuration() / 1000;
		m_pPlayer->SetPosition(tSeek);
	}

	// on end-track, undo the pause if necessary
	if ((op == TB_ENDTRACK) || (op == TB_THUMBPOSITION))
	{
		if (m_bTracking)
		{
			if (m_State == State_Running)
			{
				m_pPlayer->Play();
			}
			m_bTracking = false;
		}
	}
	return 0;
}

	
LRESULT 
CMainDlg::OnListSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{   
    bool bEnable = false;
    long idx = SendDlgItemMessage(IDC_LIST1, LB_GETCURSEL, 0, 0);
    if ((idx != LB_ERR) && (m_pPlayer->GetClipByIndex(idx) != NULL))
    {
        bEnable = true;
    }
    ::EnableWindow(GetDlgItem(IDC_LIMITS), bEnable);
    ::EnableWindow(GetDlgItem(IDC_PLAYNEXT), bEnable);
    return 0;
}

LRESULT CMainDlg::OnBnClickedLimits(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    long idx = SendDlgItemMessage(IDC_LIST1, LB_GETCURSEL, 0, 0);
    ClipEntry* pClip = NULL;
    if (idx != LB_ERR)
    {
        pClip = m_pPlayer->GetClipByIndex(idx);
    }

    if (pClip != NULL)
    {
		string sDesc = pClip->Name();

		// duration, limits dlg -- set clip limits (in seconds)
        long lDur = long((pClip->NativeDuration() + UNITS - 1) / UNITS);
        REFERENCE_TIME tStart, tStop;
        pClip->GetLimits(&tStart, &tStop);
        long lStart = long((tStart + UNITS - 1) / UNITS);
        long lStop = -1;
        if (tStop != 0)
        {
            lStop = long((tStop + UNITS - 1) / UNITS);

        }
		LimitsDlg dlg(lDur, lStart, lStop);
		if (dlg.DoModal() == IDOK)
		{
			REFERENCE_TIME tStart = dlg.Start() * UNITS;
            REFERENCE_TIME tStop = pClip->NativeDuration();
			if (dlg.Stop() > 0)
			{
				tStop = dlg.Stop() * UNITS;
			}
            m_pPlayer->SetClipLimits(pClip, tStart, tStop);

			// append range to clip description text
            ostringstream strm;
            strm << sDesc << " [" << long(tStart/UNITS) << ".." << long(tStop/UNITS) << "]";
            sDesc = strm.str();

            SendDlgItemMessage(IDC_LIST1, LB_DELETESTRING, idx, 0);
    		SendDlgItemMessage(IDC_LIST1, LB_INSERTSTRING, idx, (LPARAM)sDesc.c_str());
		}
    }

    return 0;
}

LRESULT CMainDlg::OnBnClickedPlaynext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    long idx = SendDlgItemMessage(IDC_LIST1, LB_GETCURSEL, 0, 0);
    ClipEntry* pClip = NULL;
    if (idx != LB_ERR)
    {
        pClip = m_pPlayer->GetClipByIndex(idx);
    }

    if (pClip != NULL)
    {
        m_pPlayer->PlayNext(pClip);
    }
    return 0;
}
