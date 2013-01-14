#pragma once

#include "afxcmn.h"
#include "afxwin.h"

// CPreferencesDlg dialog

class CPreferencesDlg : public CDialog
{
	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CPreferencesDlg(basic_string<TCHAR>& CaptureLocation, basic_string<TCHAR>& CaptureFilename, IBaseFilter* pVideoCapture, CMediaType& VideoFormat, IBaseFilter* pAudioCapture, CMediaType& AudioFormat, bool bMuteAudio, int Compression, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPreferencesDlg();

// Dialog Data
	enum { IDD = IDD_PREFERENCES_DIALOG };

	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnEnChangeEditCapturelocation();
	afx_msg void OnEnChangeEditCapturefilename();
	afx_msg void OnCbnSelchangeComboVideoformat();
	afx_msg void OnCbnSelchangeComboAudioformat();
	afx_msg void OnBnClickedCheckMuteaudio();
	afx_msg void OnCbnSelchangeComboCompression();

	LPCTSTR GetCaptureLocation(void) { return m_CaptureLocation.c_str(); }
	LPCTSTR GetCaptureFilename(void) { return m_CaptureFilename.c_str(); }
	CMediaType& GetVideoFormat(void) { return m_VideoFormat; }
	CMediaType& GetAudioFormat(void) { return m_AudioFormat; }
	bool GetMuteAudio(void) { return m_bMuteAudio; }
	int GetCompression(void) { return m_Compression; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	DECLARE_MESSAGE_MAP()

private:
	CEdit m_CaptureLocationCtrl;
	CEdit m_CaptureFilenameCtrl;
	CComboBox m_VideoFormatCtrl;
	CComboBox m_AudioFormatCtrl;
	CButton m_MuteAudioCtrl;
	CComboBox m_CompressionCtrl;

	basic_string<TCHAR> m_CaptureLocation;
	basic_string<TCHAR> m_CaptureFilename;
	CComPtr<IBaseFilter> m_pVideoCapture;
	CMediaType m_VideoFormat;
	CComPtr<IBaseFilter> m_pAudioCapture;
	CMediaType m_AudioFormat;
	bool m_bMuteAudio;
	int m_Compression;

	HRESULT PopulateVideoFormatControl(void);
	HRESULT PopulateAudioFormatControl(void);
	HRESULT PopulateCompressionControl(void);

	int GetIndexForCompression(void);
};
