// Based upon the Hyperlink control by Chris Maunder, www.codeproject.com.
//


#pragma once


// CStaticURL

class CStaticURL : public CStatic
{
	DECLARE_DYNAMIC(CStaticURL)

public:
	CStaticURL();
	virtual ~CStaticURL();

protected:
	COLORREF m_crLinkColour;
	COLORREF m_crHoverColour;
	BOOL m_bOverControl;
	CFont m_Font;
	HCURSOR m_hCursor;

    virtual void PreSubclassWindow();

	DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


