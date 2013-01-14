//
// GDCL Multigraph Framework
//
// GMFBridge.cpp : Registration and entrypoint code
//
// Copyright (c) GDCL 2004. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk


#include "stdafx.h"
#include "Bridge.h"

// DirectShow base class COM factory requires this table, 
// declaring all the COM objects in this DLL
CFactoryTemplate g_Templates[] = 
{

    // one entry for each CoCreate-able object
    {
        L"GDCL Multigraph Framework Bridge Controller",
        &__uuidof(GMFBridgeController),
        BridgeController::CreateInstance,
        NULL,
		NULL,		// not a filter, so no filter reg data
    },
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

// self-registration entrypoint
STDAPI DllRegisterServer()
{
    // base classes will handle registration using the factory template table
    HRESULT hr = AMovieDllRegisterServer2(true);

	// register type library
    TCHAR ach[MAX_PATH];
    GetModuleFileName(g_hInst, ach, MAX_PATH);
    _bstr_t strPath = ach;
    ITypeLib* pTypeLib;
    LoadTypeLib(strPath, &pTypeLib);
    if (pTypeLib)
    {
        RegisterTypeLib(pTypeLib, strPath, NULL);
        pTypeLib->Release();
    }

    return hr;
}

STDAPI DllUnregisterServer()
{
    // base classes will handle de-registration using the factory template table
    HRESULT hr = AMovieDllRegisterServer2(false);

	// de-register type library
    TCHAR ach[MAX_PATH];
    GetModuleFileName(g_hInst, ach, MAX_PATH);
    _bstr_t strPath = ach;
    ITypeLib* pTypeLib;
    LoadTypeLib(strPath, &pTypeLib);

    if (pTypeLib) {
        TLIBATTR* ptla;
        hr = pTypeLib->GetLibAttr(&ptla);
        if (SUCCEEDED(hr))
        {
            hr = UnRegisterTypeLib(ptla->guid, ptla->wMajorVerNum, ptla->wMinorVerNum, ptla->lcid, ptla->syskind);
            pTypeLib->ReleaseTLibAttr(ptla);
        }
        pTypeLib->Release();
    }

    return hr;
}

// if we declare the correct C runtime entrypoint and then forward it to the DShow base
// classes we will be sure that both the C/C++ runtimes and the base classes are initialized
// correctly
extern "C" BOOL WINAPI DllEntryPoint(HANDLE, ULONG, LPVOID);

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	return DllEntryPoint(hModule, ul_reason_for_call, lpReserved);
}

