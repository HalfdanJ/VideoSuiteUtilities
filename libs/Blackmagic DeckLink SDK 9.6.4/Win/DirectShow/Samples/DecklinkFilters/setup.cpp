//------------------------------------------------------------------------------
// setup.cpp
//
// Desc: DirectShow sample code - implementation of Decklink sample filters
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"

#include "DecklinkPushSource\DecklinkPushSource.h"
#include "DecklinkPushSource\CustomAllocator.h"
#include "DecklinkPushSource\DecklinkVideoSource.h"
#include "DecklinkPushSource\DecklinkAudioSource.h"
#include "DecklinkFieldSwap\DecklinkFieldSwap.h"
#include "DecklinkStillSource\DecklinkStillSource.h"
#include "DecklinkComposite\DecklinkComposite.h"
#include "DecklinkComposite\DecklinkCompositeProp.h"
#include "WAVDest\wavdest.h"
#include "DecklinkToneSource\DecklinkToneSource.h"

// Note: It is better to register no media types than to register a partial 
// media type (subtype == GUID_NULL) because that can slow down intelligent connect 
// for everyone else.

// For a specialized source filter like this, it is best to leave out the 
// AMOVIESETUP_FILTER altogether, so that the filter is not available for 
// intelligent connect. Instead, use the CLSID to create the filter or just 
// use 'new' in your application.


//------------------------------------------------------------------------------
// DecklinkPushSource filter
//------------------------------------------------------------------------------
// Filter setup data
const AMOVIESETUP_MEDIATYPE sudPinTypesDPS[] =
{
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UYVY },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_HDYC },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2 },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_v210a },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_r210 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB24 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB32 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ARGB32 },
};


const AMOVIESETUP_PIN sudOutputPinDPS = 
{
    L"Video",       // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    SIZEOF_ARRAY(sudPinTypesDPS),// Number of media types.
    sudPinTypesDPS  // Pointer to media types.
};

const AMOVIESETUP_FILTER sudDPS =
{
    &CLSID_DecklinkPushSource,		// Filter CLSID
    L"Decklink Push Source Filter",	// String name
    MERIT_DO_NOT_USE,				// Filter merit
    1,								// Number pins
    &sudOutputPinDPS				// Pin details
};

//------------------------------------------------------------------------------
// DecklinkVideoSource filter
//------------------------------------------------------------------------------
// Filter setup data
const AMOVIESETUP_MEDIATYPE sudPinTypesDVS[] =
{
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UYVY },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_HDYC },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2 },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_v210a },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_r210 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB24 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB32 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ARGB32 },
};


const AMOVIESETUP_PIN sudOutputPinDVS = 
{
    L"Video",       // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    SIZEOF_ARRAY(sudPinTypesDVS),// Number of media types.
    sudPinTypesDVS  // Pointer to media types.
};

const AMOVIESETUP_FILTER sudDVS =
{
    &CLSID_DecklinkVideoSource,		// Filter CLSID
    L"Decklink Video Source Filter",// String name
    MERIT_DO_NOT_USE,				// Filter merit
    1,								// Number pins
    &sudOutputPinDVS				// Pin details
};

//------------------------------------------------------------------------------
// DecklinkAudioSource filter
//------------------------------------------------------------------------------
// Filter setup data
const AMOVIESETUP_MEDIATYPE sudPinTypesDAS =
{
    &MEDIATYPE_Audio,       // Major type
    &MEDIASUBTYPE_PCM      // Minor type
};


const AMOVIESETUP_PIN sudOutputPinDAS = 
{
    L"Audio",      // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    &sudPinTypesDAS // Pointer to media types.
};

const AMOVIESETUP_FILTER sudDAS =
{
    &CLSID_DecklinkAudioSource,		// Filter CLSID
    L"Decklink Audio Source Filter",// String name
    MERIT_DO_NOT_USE,				// Filter merit
    1,								// Number pins
    &sudOutputPinDAS				// Pin details
};

//------------------------------------------------------------------------------
// DecklinkFieldSwap filter
//------------------------------------------------------------------------------
const AMOVIESETUP_MEDIATYPE sudPinTypesFS[] =
{
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UYVY },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_HDYC },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2 },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_v210a },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_r210 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB24 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB32 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ARGB32 },
};

const AMOVIESETUP_PIN psudPinsFS[] =
{
	{
		L"Input",            // strName
		FALSE,               // bRendered
		FALSE,               // bOutput
		FALSE,               // bZero
		FALSE,               // bMany
		&CLSID_NULL,         // clsConnectsToFilter
		L"",                 // strConnectsToPin
		SIZEOF_ARRAY(sudPinTypesFS),// nTypes
		sudPinTypesFS,        // lpTypes
	},
	{
		L"Output",           // strName
		FALSE,               // bRendered
		TRUE,                // bOutput
		FALSE,               // bZero
		FALSE,               // bMany
		&CLSID_NULL,         // clsConnectsToFilter
		L"",                 // strConnectsToPin
		SIZEOF_ARRAY(sudPinTypesFS),// nTypes
		sudPinTypesFS,        // lpTypes
	},
};

const AMOVIESETUP_FILTER sudDecklinkFieldSwap =
{ 
	&CLSID_DecklinkFieldSwap,        // clsID
	L"Decklink PAL Field Swap",      // strName
	MERIT_DO_NOT_USE,                // dwMerit
	2,                               // nPins
	psudPinsFS,                        // lpPin
};

//------------------------------------------------------------------------------
// DecklinkStillSource filter
//------------------------------------------------------------------------------
// Filter setup data
const AMOVIESETUP_MEDIATYPE sudPinTypesDSS =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_ARGB32    // Minor type
};


const AMOVIESETUP_PIN sudOutputPinDSS = 
{
    L"Video",       // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    &sudPinTypesDSS // Pointer to media types.
};

const AMOVIESETUP_FILTER sudDSS =
{
    &CLSID_DecklinkStillSource,		// Filter CLSID
    L"Decklink Still Source Filter",// String name
    MERIT_DO_NOT_USE,				// Filter merit
    1,								// Number pins
    &sudOutputPinDSS				// Pin details
};

//------------------------------------------------------------------------------
// DecklinkComposite filter
//------------------------------------------------------------------------------
const AMOVIESETUP_MEDIATYPE sudPinTypesC[] =
{
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UYVY },
	{ &MEDIATYPE_Video, &IID_MEDIASUBTYPE_HDYC },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB24 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RGB32 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ARGB32 },
};

const AMOVIESETUP_PIN psudPinsC[] =
{
	{
		L"Input",            // strName
		FALSE,               // bRendered
		FALSE,               // bOutput
		FALSE,               // bZero
		FALSE,               // bMany
		&CLSID_NULL,         // clsConnectsToFilter
		L"",                 // strConnectsToPin
		SIZEOF_ARRAY(sudPinTypesC),// nTypes
		sudPinTypesC,        // lpTypes
	},
	{
		L"Output",           // strName
		FALSE,               // bRendered
		TRUE,                // bOutput
		FALSE,               // bZero
		FALSE,               // bMany
		&CLSID_NULL,         // clsConnectsToFilter
		L"",                 // strConnectsToPin
		SIZEOF_ARRAY(sudPinTypesC),// nTypes
		sudPinTypesC,        // lpTypes
	},
};
const AMOVIESETUP_FILTER sudDCF =
{
    &CLSID_DecklinkComposite,		// Filter CLSID
    L"Decklink Composite Filter",   // String name
    MERIT_DO_NOT_USE,				// Filter merit
    2,								// Number pins
    psudPinsC						// Pin details
};

//------------------------------------------------------------------------------
// Microsoft DirectShow SDK Sample WAV Dest filter
//------------------------------------------------------------------------------
const AMOVIESETUP_FILTER sudWavDest =
{
    &CLSID_WavDest,           // clsID
    L"WAV Dest",              // strName
    MERIT_DO_NOT_USE,         // dwMerit
    0,                        // nPins
    0                         // lpPin
};

//------------------------------------------------------------------------------
// DecklinkToneSource filter
//------------------------------------------------------------------------------
// Filter setup data
const AMOVIESETUP_MEDIATYPE sudPinTypesDTS =
{
    &MEDIATYPE_Audio,       // Major type
    &MEDIASUBTYPE_PCM      // Minor type
};


const AMOVIESETUP_PIN sudOutputPinDTS = 
{
    L"Audio",       // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    &sudPinTypesDTS // Pointer to media types.
};

const AMOVIESETUP_FILTER sudDTS =
{
    &CLSID_DecklinkToneSource,		// Filter CLSID
    L"Decklink Tone Source Filter", // String name
    MERIT_DO_NOT_USE,				// Filter merit
    1,								// Number pins
    &sudOutputPinDTS				// Pin details
};

//------------------------------------------------------------------------------
// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance.
// We provide a set of filters in this one DLL.

CFactoryTemplate g_Templates[] = 
{
	{ 
		L"Decklink Push Source Filter",				// Name
		&CLSID_DecklinkPushSource,					// CLSID
		CDecklinkPushSource::CreateInstance,		// Method to create an instance of MyComponent
		NULL,										// Initialization function
		&sudDPS										// Set-up information (for filters)
	},
	{ 
		L"Decklink Push Source Custom Allocator",	// Name
		&CLSID_CustomMemAllocator,					// CLSID
		CCustomAllocator::CreateInstance,			// Method to create an instance of MyComponent
		NULL,										// Initialization function
		NULL										// Set-up information (for filters)
	},
	{ 
		L"Decklink Video Source Filter",			// Name
		&CLSID_DecklinkVideoSource,					// CLSID
		CDecklinkVideoSource::CreateInstance,		// Method to create an instance of MyComponent
		NULL,										// Initialization function
		&sudDVS										// Set-up information (for filters)
	},
	{ 
		L"Decklink Audio Source Filter",			// Name
		&CLSID_DecklinkAudioSource,					// CLSID
		CDecklinkAudioSource::CreateInstance,		// Method to create an instance of MyComponent
		NULL,										// Initialization function
		&sudDAS										// Set-up information (for filters)
	},
	{
		L"Decklink PAL Field Swap",					// Name
		&CLSID_DecklinkFieldSwap,					// CLSDID
		CDecklinkFieldSwap::CreateInstance,			// Method to create an instance of MyComponent
		NULL,										// Initialization function
		&sudDecklinkFieldSwap						// Set-up information (for filters)
	},
	{ 
		L"Decklink Still Source Filter",			// Name
		&CLSID_DecklinkStillSource,					// CLSID
		CDecklinkStillSource::CreateInstance,		// Method to create an instance of MyComponent
		NULL,										// Initialization function
		&sudDSS										// Set-up information (for filters)
	},
	{ 
		L"Decklink Composite Filter",				// Name
		&CLSID_DecklinkComposite,					// CLSID
		CDecklinkComposite::CreateInstance,			// Method to create an instance of MyComponent
		NULL,										// Initialization function
		&sudDCF										// Set-up information (for filters)
	},
	{ 
		L"Decklink Composite Filter Properties",	// Name
		&CLSID_DecklinkCompositeProperties,			// CLSID
		CDecklinkCompositeProperties::CreateInstance,// Method to create an instance of MyComponent
		NULL,										// Initialization function
		NULL										// Set-up information (for filters)
	},
    {
		L"WAV Dest",
		&CLSID_WavDest,
		CWavDestFilter::CreateInstance,
		NULL,
		&sudWavDest
	},
	{ 
		L"Decklink Tone Source Filter",				// Name
		&CLSID_DecklinkToneSource,					// CLSID
		CDecklinkToneSource::CreateInstance,		// Method to create an instance of MyComponent
		NULL,										// Initialization function
		&sudDTS										// Set-up information (for filters)
	},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);    

////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// DllRegisterServer
//
STDAPI DllRegisterServer()
{
	HRESULT hr = AMovieDllRegisterServer2(TRUE);

	// register type library
	WCHAR achFileName[MAX_PATH];
	GetModuleFileNameW(g_hInst, achFileName, MAX_PATH);
	ITypeLib* pTypeLib;
	LoadTypeLib(achFileName, &pTypeLib);
	if (pTypeLib)
	{
		RegisterTypeLib(pTypeLib, achFileName, NULL);
		pTypeLib->Release();
	}

	return hr;
}

//------------------------------------------------------------------------------
// AMovieDllRegisterServer2
//
STDAPI DllUnregisterServer()
{
	HRESULT hr = AMovieDllRegisterServer2(FALSE);

	// unregister type library
	WCHAR achFileName[MAX_PATH];
	GetModuleFileNameW(g_hInst, achFileName, MAX_PATH);
	ITypeLib* pTypeLib;
	LoadTypeLib(achFileName, &pTypeLib);

	if (pTypeLib)
	{
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

//------------------------------------------------------------------------------
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

//------------------------------------------------------------------------------
// DllMain
//
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

