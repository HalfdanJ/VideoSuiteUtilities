// 
// GDCL Multigraph Framework
//
// GMFPreview Demo Application: PreviewController.cpp
// implementation of preview controller class, 
// manages capture device using GMFBridge tools
//
// Copyright (c) GDCL 2004. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk



#include "StdAfx.h"
#include ".\previewcontroller.h"
	
PreviewController::~PreviewController()
{
	IVideoWindowPtr pVW = m_pSourceGraph;
	if (pVW != NULL)
	{
		pVW->put_Visible(OAFALSE);
		pVW->put_Owner(NULL);
	}
	StopCapture();
	m_pCaptureGraph = NULL;
	IMediaControlPtr pMC = m_pSourceGraph;
	if (pMC != NULL)
	{
		pMC->Stop();
		pMC = NULL;
	}
	m_pSourceGraph = NULL;
}

HRESULT 
PreviewController::SelectDevice(const TCHAR* pName, HWND hwnd, RECT* prc)
{
	// create a new device
	m_pSourceGraph = NULL;
	m_pBridge = NULL;
	m_pCapOutput = NULL;

	HRESULT hr = m_pBridge.CreateInstance(__uuidof(GMFBridgeController));

	if (FAILED(hr))
	{
		return hr;
	}

	// init to video-only, in discard mode (ie when source graph
	// is running but not connected, buffers are discarded at the bridge)
	hr = m_pBridge->AddStream(true, eMuxInputs, true);
	if (FAILED(hr))
	{
		return hr;
	}

	// locate the requested filter
	IBaseFilterPtr pfDevice;
	hr = LocateFilter(pName, CLSID_VideoInputDeviceCategory, &pfDevice);
	if (FAILED(hr))
	{
		return hr;
	}

	// create source graph and add sink filter
	hr = m_pSourceGraph.CreateInstance(CLSID_FilterGraph);
	if (FAILED(hr))
	{
		return hr;
	}
	_bstr_t strName = pName;
	m_pSourceGraph->AddFilter(pfDevice, strName);

	IUnknownPtr punkSink;
	hr = m_pBridge->InsertSinkFilter(m_pSourceGraph, &punkSink);
	if (FAILED(hr))
	{
		return hr;
	}
	IBaseFilterPtr pfSink = punkSink;

	// use capture graph builder to render preview
	ICaptureGraphBuilder2Ptr pBuilder;
	hr = pBuilder.CreateInstance(CLSID_CaptureGraphBuilder2);
	if (SUCCEEDED(hr))
	{
		pBuilder->SetFiltergraph(m_pSourceGraph);
		hr = pBuilder->RenderStream(
                                    &PIN_CATEGORY_PREVIEW, 
                                    &MEDIATYPE_Video,
                                    pfDevice,
                                    NULL,
                                    NULL);
							
		if (SUCCEEDED(hr))
		{
			// connect capture output to the pseudo-sink filter,
			// where it will be discarded until required
			hr = pBuilder->RenderStream(
                                &PIN_CATEGORY_CAPTURE,
                                &MEDIATYPE_Video,
                                pfDevice,
                                NULL,
                                pfSink);

			if (SUCCEEDED(hr))
			{
				// turn off capture stream if possible except when capturing
				HRESULT hrCtl = pBuilder->FindPin(
											pfDevice,
											PINDIR_OUTPUT,
											&PIN_CATEGORY_CAPTURE,
											&MEDIATYPE_Video,
											false,
											0,
											&m_pCapOutput);
				if (SUCCEEDED(hrCtl))
				{
					IAMStreamControlPtr pSC = m_pCapOutput;
					if (pSC != NULL)
					{
						REFERENCE_TIME tNever = MAXLONGLONG;
						pSC->StartAt(&tNever, 0);
					}
				}
			}
		}
		pBuilder->SetFiltergraph(NULL);
	}
	if (SUCCEEDED(hr) && IsWindow(hwnd))
	{
		// adjust rect to maintain something like aspect ratio
		IBasicVideoPtr pBV = m_pSourceGraph;
		long cx, cy;
		pBV->GetVideoSize(&cx, &cy);
		RECT rc = *prc;
		long cxRect = rc.right - rc.left;
		long cyRect = rc.bottom - rc.top;
		if ((cx > cxRect) ||
			(cy > cyRect))
		{
			if ((cxRect * cy/cx) <= cyRect)
			{
				cy = cxRect * cy / cx;
				cx = cxRect;
			} else {
				cx = cyRect * cx / cy;
				cy = cyRect;
			}
		}

		// centre video inside requested rect
		rc.left = rc.left + ((cxRect - cx) / 2);
		rc.right = rc.left + cx;
		rc.top = rc.top + ((cyRect - cy)/2);
		rc.bottom = rc.top + cy;

		// reparent playback window
		IVideoWindowPtr pVW = m_pSourceGraph;
		if (pVW != NULL)
		{
			long style;
			pVW->get_WindowStyle(&style);
			style &= ~WS_OVERLAPPEDWINDOW;
			style |= WS_CHILD;
			pVW->put_WindowStyle(style);
			pVW->put_Owner((OAHWND)hwnd);
			pVW->SetWindowPosition(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
		}
		IMediaControlPtr pMC = m_pSourceGraph;
		hr = pMC->Run();
		if (hr == S_FALSE)
		{
			// this is just background state change
			hr = S_OK;
		}
	}
	m_pSourceGraphSinkFilter = punkSink;
	pfSink = NULL;
	return hr;
}

HRESULT 
PreviewController::SetNextFilename(const TCHAR* pFile)
{
	if (m_pSourceGraph == NULL)
	{
		return VFW_E_WRONG_STATE;
	}

	HRESULT hr = m_pCaptureGraph.CreateInstance(CLSID_FilterGraph);
	if (FAILED(hr))
	{
		return hr;
	}
    IUnknownPtr punkSource;
	hr = m_pBridge->InsertSourceFilter(m_pSourceGraphSinkFilter, m_pCaptureGraph, &punkSource);
	if (FAILED(hr))
	{
		return hr;
	}
	IBaseFilterPtr pfSource = punkSource;

	// use capture graph builder to create mux/writer stage
	ICaptureGraphBuilder2Ptr pBuilder;
	hr = pBuilder.CreateInstance(CLSID_CaptureGraphBuilder2);
	if (FAILED(hr))
	{
		return hr;
	}
	pBuilder->SetFiltergraph(m_pCaptureGraph);

	// create the mux/writer
	_bstr_t bstrFile = pFile;
	IBaseFilterPtr pfMux;
	hr = pBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi, bstrFile, &pfMux, NULL);
	if (SUCCEEDED(hr))
	{
		// render source output to mux
		hr = pBuilder->RenderStream(NULL, NULL, punkSource, NULL, pfMux);
	}

	if (SUCCEEDED(hr))
	{
		m_strFile = pFile;
	}
	m_pCaptureGraphSourceFilter = punkSource;
	return hr;
}

HRESULT 
PreviewController::StartCapture()
{
	// re-enable capture stream
	IAMStreamControlPtr pSC = m_pCapOutput;
	if (pSC != NULL)
	{
		// immediately!
		pSC->StartAt(NULL, 0);
	}

	// start capture graph
	IMediaControlPtr pMC = m_pCaptureGraph;
	if (pMC == NULL)
	{
		return E_INVALIDARG;
	}
	pMC->Run();

	// connect the two segments
	return m_pBridge->BridgeGraphs(m_pSourceGraphSinkFilter, m_pCaptureGraphSourceFilter);
}

HRESULT 
PreviewController::StopCapture()
{
	HRESULT hr = S_OK;
	if (m_pBridge)
	{
		// disconnect segments
		hr = m_pBridge->BridgeGraphs(NULL, NULL);

		// stop capture graph
		IMediaControlPtr pMC = m_pCaptureGraph;
		if (pMC)
		{
			pMC->Stop();
		}
	}
	// disable capture stream (to save resources)
	IAMStreamControlPtr pSC = m_pCapOutput;
	if (pSC != NULL)
	{
		REFERENCE_TIME tNever = MAXLONGLONG;
		pSC->StartAt(&tNever, 0);
	}
	return hr;
}

HRESULT
PreviewController::LocateFilter(const TCHAR* pName, REFIID catid, IBaseFilter** ppFilter)
{
	_bstr_t strRequested = pName;
    ICreateDevEnumPtr pCreate(CLSID_SystemDeviceEnum);
	IEnumMonikerPtr pEnum;
    HRESULT hr = pCreate->CreateClassEnumerator(catid, &pEnum, 0);
    if (hr != S_OK) {
	    // hr == S_FALSE means pEnum == NULL because category empty
        return E_INVALIDARG;
    }

	if (pEnum != NULL) {
        IMonikerPtr pMoniker;
        while(pEnum->Next(1, &pMoniker, NULL) == S_OK) {
			IPropertyBagPtr pBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);

			if (SUCCEEDED(hr)) {
				_variant_t var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL);
				if (SUCCEEDED(hr)) {
					_bstr_t str = var;
					if (str == strRequested)
					{
				        hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)ppFilter);
						if (FAILED(hr))
						{
							return hr;
						}
						break;
					}
				}
			}
		}
	}
	if (*ppFilter == NULL)
	{
		return E_INVALIDARG;
	}
	return S_OK;
}