//-----------------------------------------------------------------------------
// HistogramCtrl.h
//
// Desc: DirectShow histogram sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------------------
// CHistogramCtrl class
//
class CHistogramCtrl : public CStatic
{
public:
	CHistogramCtrl();
	virtual ~CHistogramCtrl();

	// base class override
	BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL, CRgn* prgnUpdate = NULL, UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

	HRESULT SetType(const BITMAPINFOHEADER* pbmih);
	HRESULT SetBuffer(const unsigned char* pBuffer, unsigned long cbBuffer);

protected:
	//{{AFX_MSG(CHistogramCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	
private:
	enum _tagHistogramType { HISTOGRAM_TYPE_RGB, HISTOGRAM_TYPE_LUMA };

	CRITICAL_SECTION m_critSec;	// thread synchronisation

	BITMAPINFOHEADER m_bmih;
	unsigned long m_maxVal;	// max pixel value of the current format
	enum _tagHistogramType m_type;	// histogram type, RGB or luma

	unsigned long* m_pHistogram;	// histogram data array

	CBrush m_brushBkgnd;	// brush for erasing the background for every redraw
	CPen m_penBlue;
	CPen m_penGreen;
	CPen m_penRed;
	CPen m_penGrey;	// grey pen for luminosity histogram

	BOOL m_bRedraw;
};
