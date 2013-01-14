//-----------------------------------------------------------------------------
// HistogramCtrl.cpp
//
// Desc: DirectShow histogram sample
//
// Copyright (c) Blackmagic Design 2005. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "HistogramCtrl.h"
#include <dvdmedia.h>

//-----------------------------------------------------------------------------
// Constructor
//
CHistogramCtrl::CHistogramCtrl()
	: m_maxVal(0)
	, m_type(HISTOGRAM_TYPE_RGB)
	, m_pHistogram(NULL)
	, m_brushBkgnd(GetSysColor(COLOR_BTNFACE))
	, m_penBlue(PS_SOLID, 1, RGB(0, 0, 255))
	, m_penGreen(PS_SOLID, 1, RGB(0, 255, 0))
	, m_penRed(PS_SOLID, 1, RGB(255, 0, 0))
	, m_penGrey(PS_SOLID, 1, RGB(200, 200, 200))
	, m_bRedraw(FALSE)
{
	ZeroMemory(&m_bmih, sizeof(m_bmih));
	InitializeCriticalSection(&m_critSec);
}

//-----------------------------------------------------------------------------
// Deconstructor
//
CHistogramCtrl::~CHistogramCtrl()
{
	SAFE_DELETE_ARRAY(m_pHistogram);
	DeleteCriticalSection(&m_critSec);
}

//-----------------------------------------------------------------------------
// Message map
//
BEGIN_MESSAGE_MAP(CHistogramCtrl, CStatic)
	//{{AFX_MSG_MAP(CHistogramCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// CHistogramCtrl message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// OnPaint
// Overridden to paint the histogram bars on the display
void CHistogramCtrl::OnPaint() 
{
	if (m_pHistogram && TryEnterCriticalSection(&m_critSec))	// minimal blocking of Windows, call it paranoia
	{
		CPaintDC dc(this);	// dc for painting histogram
		CRect rcControl;	// bounds of this control
		GetClientRect(&rcControl);
		dc.FillRect(rcControl, &m_brushBkgnd);	// erase previous drawing
		
		CRect rcLine = rcControl;
		rcLine.top += 2;
		rcLine.bottom -= 1;

		LONG lineMaxHeight = rcLine.bottom - rcLine.top;	// max line height within the bounds of the control
		float lineAcc = (float)(rcLine.right - rcLine.left) / m_maxVal;	// horizontal increment between histogram elements
		float lineRight = 0;

		unsigned long* pHistogramChannel = m_pHistogram;
		unsigned long cChannels = (HISTOGRAM_TYPE_RGB == m_type) ? 3 : 1;

		CPen* pOldPen;	// used to restore the previous pen once drawing is complete

		for (unsigned long channel=0; channel<cChannels; ++channel, pHistogramChannel+=m_maxVal)
		{
			// setup drawing for this iteration
			if (0 == channel)
			{
				if (HISTOGRAM_TYPE_RGB == m_type)
				{
					pOldPen = dc.SelectObject(&m_penBlue);
				}
				else
				{
					pOldPen = dc.SelectObject(&m_penGrey);
				}
			}
			else if (1 == channel)
			{
				dc.SelectObject(&m_penGreen);
			}
			else
			{
				dc.SelectObject(&m_penRed);
			}

			// loop through all the possible pixel values and find the highest count
			unsigned long maxCount = 0;
			for (unsigned long val=0; val<m_maxVal; ++val)
			{
				if (maxCount < pHistogramChannel[val])
				{
					maxCount = pHistogramChannel[val];
				}
			}

			if (0 == maxCount)
			{
				// prevent div zero
				maxCount = 1;
			}

			// reset line drawing to the left side of the control
			rcLine.left = rcControl.left;
			lineRight = rcLine.left + lineAcc;
			rcLine.right = (LONG)lineRight;
			// the vertical scale is governed by the total number of pixels which can have the same value
			unsigned long lineHeight = lineMaxHeight - (lineMaxHeight * pHistogramChannel[0] / maxCount);
			dc.MoveTo(rcLine.left, lineHeight);

			// loop through all the possible pixel values and plot a line graph
			for (unsigned long val=1; val<m_maxVal; ++val)
			{
				// draw consectutive lines for each histogram element
				// the vertical scale is governed by the total number of pixels which can have the same value
				lineHeight = lineMaxHeight - (lineMaxHeight * pHistogramChannel[val] / maxCount);
				dc.LineTo(rcLine.right, lineHeight);
				
				rcLine.left = (LONG)lineRight;	// cast away the mantissa
				lineRight += lineAcc;
				rcLine.right = (LONG)lineRight;	// cast away the mantissa
			}
		}

		dc.SelectObject(pOldPen);

		LeaveCriticalSection(&m_critSec);
	}
}

//-----------------------------------------------------------------------------
// RedrawWindow
// Repaint the control if new histogram data has been flagged
BOOL CHistogramCtrl::RedrawWindow(LPCRECT lpRectUpdate, CRgn* prgnUpdate, UINT flags)
{
	BOOL ret = TRUE;

	if (m_bRedraw)
	{
		m_bRedraw = FALSE;
		ret = CStatic::RedrawWindow(lpRectUpdate, prgnUpdate, flags);
	}

	return ret;
}

//-----------------------------------------------------------------------------
// SetType
// Inform the class of the bitmap to determine which type of histogram to display
HRESULT CHistogramCtrl::SetType(const BITMAPINFOHEADER* pbmih)
{
	HRESULT hr = S_OK;

	if (pbmih)
	{
		m_bmih = *pbmih;
		// extract bit depth and compression
		if ((BI_RGB == m_bmih.biCompression) && ((32 == m_bmih.biBitCount) || (24 == m_bmih.biBitCount)))
		{
			// 8-bit RGB format
			m_maxVal = 255;
			m_type = HISTOGRAM_TYPE_RGB;

			SAFE_DELETE_ARRAY(m_pHistogram);
			m_pHistogram = new unsigned long [3 * (m_maxVal + 1)];	// histograms for all three channels
			if (NULL == m_pHistogram)
			{
				hr = E_OUTOFMEMORY;
			}
		}
		else if (('YVYU' == m_bmih.biCompression) || ('2YUY' == m_bmih.biCompression) ||
					('yuv2' == m_bmih.biCompression) || ('yuV2' == m_bmih.biCompression))	// QT FCCs
		{
			// 8-bit YUV format
			m_maxVal = 235;
			m_type = HISTOGRAM_TYPE_LUMA;

			SAFE_DELETE_ARRAY(m_pHistogram);
			m_pHistogram = new unsigned long [m_maxVal + 1];	// histogram for luma channel
			if (NULL == m_pHistogram)
			{
				hr = E_OUTOFMEMORY;
			}
		}
		else
		{
			// unsupported compression or bit depth
			hr = VFW_E_INVALIDMEDIATYPE;
		}
	}
	else
	{
		hr = E_POINTER;
	}
	
	return hr;
}

//-----------------------------------------------------------------------------
// SetType
// Inform the class of the bitmap buffer to analyse
HRESULT CHistogramCtrl::SetBuffer(const unsigned char* pBuffer, unsigned long cbBuffer)
{
	HRESULT hr = S_OK;

	if (pBuffer && cbBuffer && m_pHistogram)
	{
		if (TryEnterCriticalSection(&m_critSec))	// attempt to hold the lock, this way this there is minimum inteference to the stream being sampled
		{
			unsigned long maxBins = m_maxVal + 1;
			if (HISTOGRAM_TYPE_RGB == m_type)
			{
				ZeroMemory(m_pHistogram, 3 * maxBins * sizeof(unsigned long));	// reset the histogram data
				unsigned long* pBlueHistogram = m_pHistogram;
				unsigned long* pGreenHistogram = m_pHistogram + maxBins;
				unsigned long* pRedHistogram = m_pHistogram + (2 * maxBins);
				for (unsigned long i=0; i<cbBuffer; i+=3)
				{
					ASSERT(*pBuffer <= m_maxVal);
					pBlueHistogram[*pBuffer++] += 1;
					ASSERT(*pBuffer <= m_maxVal);
					pGreenHistogram[*pBuffer++] += 1;
					ASSERT(*pBuffer <= m_maxVal);
					pRedHistogram[*pBuffer++] += 1;
					if (32 == m_bmih.biBitCount)
					{
						// skip alpha channel
						++pBuffer;
						++i;
					}
				}
			}
			else if (HISTOGRAM_TYPE_LUMA == m_type)
			{
				ZeroMemory(m_pHistogram, maxBins * sizeof(unsigned long));	// reset the histogram data
				if ('YVYU' == m_bmih.biCompression)
				{
					++pBuffer;	// start on the luma data
				}
				for (unsigned long i=0; i<cbBuffer; i+=2, pBuffer+=2)
				{
					m_pHistogram[*pBuffer] += 1;
				}
			}
			else
			{
				// unknown histogram type
				hr = E_FAIL;
			}

			m_bRedraw = TRUE;
			LeaveCriticalSection(&m_critSec);
		}
		else
		{
			hr = S_FALSE;
		}
	}
	else
	{
		hr = E_POINTER;
	}
	
	return hr;
}
