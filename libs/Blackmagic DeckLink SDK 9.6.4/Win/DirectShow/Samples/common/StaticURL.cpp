// StaticURL.cpp : implementation file
//
// Based upon the Hyperlink control by Chris Maunder, www.codeproject.com.
//

#include "stdafx.h"
#include "StaticURL.h"


// CStaticURL

IMPLEMENT_DYNAMIC(CStaticURL, CStatic)
CStaticURL::CStaticURL()
	: m_crLinkColour(RGB(0, 0, 255))
	, m_crHoverColour(RGB(0, 127, 255))
	, m_bOverControl(FALSE)
	, m_hCursor(NULL)
{
}

CStaticURL::~CStaticURL()
{
}


BEGIN_MESSAGE_MAP(CStaticURL, CStatic)
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_SETCURSOR()
    ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CStaticURL message handlers

void CStaticURL::PreSubclassWindow() 
{
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, GetStyle() | SS_NOTIFY);

	// Create the font
	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	lf.lfUnderline = TRUE;
	m_Font.CreateFontIndirect(&lf);
	SetFont(&m_Font);

	CStatic::PreSubclassWindow();
}

HBRUSH CStaticURL::CtlColor(CDC* pDC, UINT nCtlColor)
{
	UNREFERENCED_PARAMETER(nCtlColor);
	ASSERT(nCtlColor == CTLCOLOR_STATIC);
	
	if (m_bOverControl)
	{
		pDC->SetTextColor(m_crHoverColour);
	}
	else
	{
		pDC->SetTextColor(m_crLinkColour);
	}

	// transparent text.
	pDC->SetBkMode(TRANSPARENT);
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

BOOL CStaticURL::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (NULL == m_hCursor)
	{
		m_hCursor = ::LoadCursor(NULL, IDC_HAND);
	}
	
	if (m_hCursor)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}
	return FALSE;
}

void CStaticURL::OnMouseMove(UINT nFlags, CPoint point)
{
	CStatic::OnMouseMove(nFlags, point);

	if (m_bOverControl)        // Cursor is currently over control
	{
		CRect rect;
		GetClientRect(rect);

		if (!rect.PtInRect(point))
		{
			m_bOverControl = FALSE;
			ReleaseCapture();
			RedrawWindow();
			return;
		}
	}
	else                      // Cursor has just moved over control
	{
		m_bOverControl = TRUE;
		RedrawWindow();
		SetCapture();
	}
}

void CStaticURL::OnLButtonUp(UINT nFlags, CPoint point)
{
	CString URL;
	GetWindowText(URL);
	if (!URL.IsEmpty())
	{
		if (HINSTANCE_ERROR < (int)ShellExecute(0, TEXT("open"), URL, 0, 0, SW_SHOWNORMAL))
		{
			m_bOverControl = FALSE;
		    if (::IsWindow(GetSafeHwnd()))
		    {
				Invalidate(); 
			}
		}
	}
}
