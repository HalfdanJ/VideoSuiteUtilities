// 
// GDCL Multigraph Framework
//
// GMFPreview Demo Application: PreviewController.h
// Declaration of class to manage capture device preview and capture
// using GMFBridge tools
//
// Copyright (c) GDCL 2004. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk

#pragma once

#include <string>
using namespace std;

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#include "..\GMFBridge_h.h"
_COM_SMARTPTR_TYPEDEF(ICreateDevEnum, IID_ICreateDevEnum);
_COM_SMARTPTR_TYPEDEF(IGMFBridgeController, __uuidof(IGMFBridgeController));
_COM_SMARTPTR_TYPEDEF(IGraphBuilder, IID_IGraphBuilder);
_COM_SMARTPTR_TYPEDEF(IMediaControl, IID_IMediaControl);
_COM_SMARTPTR_TYPEDEF(ICaptureGraphBuilder2, IID_ICaptureGraphBuilder2);
_COM_SMARTPTR_TYPEDEF(IVideoWindow, IID_IVideoWindow);
_COM_SMARTPTR_TYPEDEF(IBasicVideo, IID_IBasicVideo);
_COM_SMARTPTR_TYPEDEF(IBaseFilter, IID_IBaseFilter);
_COM_SMARTPTR_TYPEDEF(IAMStreamControl, IID_IAMStreamControl);
_COM_SMARTPTR_TYPEDEF(IPin, IID_IPin);

class PreviewController
{
public:
	~PreviewController();
	HRESULT SelectDevice(const TCHAR* pName, HWND hwnd, RECT* prc);
	HRESULT SetNextFilename(const TCHAR* pFile);
	HRESULT StartCapture();
	HRESULT StopCapture();
	HRESULT PauseCapture();
	tstring GetFilename()
	{
		return m_strFile;
	}
    bool HasDevice()
    {
        return (m_pSourceGraph != NULL) ? true : false;
    }
private:
	HRESULT LocateFilter(const TCHAR* pName, REFIID catid, IBaseFilter** ppFilter);

private:
	IGMFBridgeControllerPtr m_pBridge;
	IGraphBuilderPtr m_pSourceGraph;
	tstring m_strFile;
	IGraphBuilderPtr m_pCaptureGraph;
	IPinPtr m_pCapOutput;
	IUnknownPtr m_pSourceGraphSinkFilter;
	IUnknownPtr m_pCaptureGraphSourceFilter;
};
