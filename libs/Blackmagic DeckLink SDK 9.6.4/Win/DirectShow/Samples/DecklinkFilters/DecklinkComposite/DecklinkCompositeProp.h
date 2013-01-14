//------------------------------------------------------------------------------
// DecklinkCompositeProp.h
//
//
//
//
//

#pragma once

class CDecklinkCompositeProperties : public CBasePropertyPage
{
public:

    CDecklinkCompositeProperties(LPUNKNOWN lpUnk, HRESULT *phr);
    static CUnknown *WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

private:
	IAMStreamConfig* m_pISC;
	CMediaType m_mediaType;
	BITMAPINFOHEADER* m_pbmih;
	RECT* m_prcSrc;
	RECT* m_prcDst;

    static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void EnableControls(HWND hwndParent, BOOL Enable);
    void SetDirty(void);
};
