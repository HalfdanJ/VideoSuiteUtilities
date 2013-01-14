//------------------------------------------------------------------------------
// DecklinkCompositeProp.cpp
//
//
//
//
//

#include "stdafx.h"
#include "DecklinkComposite.h"
#include "DecklinkCompositeProp.h"
#include "..\resource.h"

#pragma warning(disable:4127)   // C4127: conditional expression is constant

#define WM_PROPERTYPAGE_ENABLE  (WM_USER + 100)

#define DBG_DLG	5

// -------------------------------------------------------------------------
// CDecklinkCompositeProperties
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// CreateInstance
//
CUnknown * WINAPI CDecklinkCompositeProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT* phr)
{
    CUnknown *punk = new CDecklinkCompositeProperties(lpunk, phr);
    if (punk == NULL)
    {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}

// -------------------------------------------------------------------------
// Constructor
// Create a Property page object for the decklink
CDecklinkCompositeProperties::CDecklinkCompositeProperties(LPUNKNOWN lpunk, HRESULT* phr)
    : CBasePropertyPage(NAME("Decklink Bitmap Source Property Page"), lpunk, IDD_DECKLINK_COMPOSITE_PROPPAGE, IDS_DECKLINK_COMPOSITE_PROPNAME)
    , m_pISC(NULL)
	, m_pbmih(NULL)
	, m_prcSrc(NULL)
	, m_prcDst(NULL)
{
    m_mediaType.InitMediaType();
    ASSERT(phr);
}

// -------------------------------------------------------------------------
// OnConnect
// Give us the filter to communicate with
HRESULT CDecklinkCompositeProperties::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = S_OK;

	if (pUnknown)
	{
		CComQIPtr<IBaseFilter, &IID_IBaseFilter> pBaseFilter = pUnknown;
		if (pBaseFilter)
		{
			hr = pBaseFilter->QueryInterface(IID_IAMStreamConfig, reinterpret_cast<void**>(&m_pISC));
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

// -------------------------------------------------------------------------
// OnDisconnect
// Release the interface
HRESULT CDecklinkCompositeProperties::OnDisconnect()
{
	if (m_pISC)
	{
		m_pISC->Release();
		m_pISC = NULL;
	}

	return S_OK;
}

// -------------------------------------------------------------------------
// OnActivate
// Called on dialog creation
HRESULT CDecklinkCompositeProperties::OnActivate(void)
{
	HRESULT hr = S_OK;
	
	if (m_pISC)
	{
		AM_MEDIA_TYPE* pamt = NULL;
		hr = m_pISC->GetFormat(&pamt);
		if (SUCCEEDED(hr))
		{
			m_mediaType.Set(*pamt);

			if (FORMAT_VideoInfo == m_mediaType.formattype)
			{
				VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)m_mediaType.pbFormat;
				m_pbmih = &pvih->bmiHeader;
				m_prcSrc = &pvih->rcSource;
				m_prcDst = &pvih->rcTarget;
			}
			else if (FORMAT_VideoInfo2 == m_mediaType.formattype)
			{
				VIDEOINFOHEADER2* pvih = (VIDEOINFOHEADER2*)m_mediaType.pbFormat;
				m_pbmih = &pvih->bmiHeader;
				m_prcSrc = &pvih->rcSource;
				m_prcDst = &pvih->rcTarget;
			}

			TCHAR buf[16];
			if (m_pbmih)
			{
				StringCbPrintf(buf, sizeof(buf), TEXT("%ld"), m_pbmih->biWidth);
				SendDlgItemMessage(m_hwnd, IDC_EDIT_WIDTH, WM_SETTEXT, 0, (LPARAM)buf);
				StringCbPrintf(buf, sizeof(buf), TEXT("%ld"), m_pbmih->biHeight);
				SendDlgItemMessage(m_hwnd, IDC_EDIT_HEIGHT, WM_SETTEXT, 0, (LPARAM)buf);
			}
			else
			{
				StringCbPrintf(buf, sizeof(buf), TEXT("<Not Connected>"));
				SendDlgItemMessage(m_hwnd, IDC_EDIT_WIDTH, WM_SETTEXT, 0, (LPARAM)buf);
				StringCbPrintf(buf, sizeof(buf), TEXT("<Not Connected>"));
				SendDlgItemMessage(m_hwnd, IDC_EDIT_HEIGHT, WM_SETTEXT, 0, (LPARAM)buf);
			}

			DeleteMediaType(pamt);
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

// -------------------------------------------------------------------------
// OnDeactivate
// Called on dialog destruction
HRESULT CDecklinkCompositeProperties::OnDeactivate(void)
{
    return NOERROR;
}

// -------------------------------------------------------------------------
// OnApplyChanges
// User pressed the Apply button, remember the current settings
HRESULT CDecklinkCompositeProperties::OnApplyChanges(void)
{
    return m_pISC->SetFormat((AM_MEDIA_TYPE*)&m_mediaType);
}

// -------------------------------------------------------------------------
// OnReceiveMessages
// Handles the messages for our property window
INT_PTR CDecklinkCompositeProperties::OnReceiveMessage(HWND hwndDlg , UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[16];

	switch (uMsg)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_EDIT_WIDTH:
					if (EN_CHANGE == HIWORD(wParam))
					{
						if (m_pbmih && (0 < SendDlgItemMessage(m_hwnd, IDC_EDIT_WIDTH, WM_GETTEXT, 16, (LPARAM)buf)))
						{
							m_pbmih->biWidth = _tstoi(buf);
						    m_pbmih->biSizeImage = CUtils::GetImageSize(m_pbmih);
							SetDirty();
						}
					}
					break;

				case IDC_EDIT_HEIGHT:
					if (EN_CHANGE == HIWORD(wParam))
					{
						if (m_pbmih && (0 < SendDlgItemMessage(m_hwnd, IDC_EDIT_HEIGHT, WM_GETTEXT, 16, (LPARAM)buf)))
						{
							m_pbmih->biHeight = _tstoi(buf);
						    m_pbmih->biSizeImage = CUtils::GetImageSize(m_pbmih);
							SetDirty();
						}
					}
					break;

				default:
					break;
			}
			return TRUE;
	}

	return CBasePropertyPage::OnReceiveMessage(hwndDlg, uMsg, wParam, lParam);
}

// -------------------------------------------------------------------------
// SetDirty
// notifies the property page site of changes
void CDecklinkCompositeProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}



