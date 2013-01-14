// DeviceInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DeviceInfoDlg.h"


//-----------------------------------------------------------------------------
// CDeviceInfoDlg dialog
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CDeviceInfoDlg, CDialog)
//-----------------------------------------------------------------------------
// Construction
//
CDeviceInfoDlg::CDeviceInfoDlg(LPCWSTR pDeviceName, const GUID* pCategory, CWnd* pParent /*=NULL*/)
	: CDialog(CDeviceInfoDlg::IDD, pParent)
	, m_DeviceName(pDeviceName)
	, m_pCategory(pCategory)
{
}

//-----------------------------------------------------------------------------
// Destruction
//
CDeviceInfoDlg::~CDeviceInfoDlg()
{
}

//-----------------------------------------------------------------------------
// DoDataExchange
//
void CDeviceInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DEVICEINFO, m_DeviceInfoCtrl);
}

//-----------------------------------------------------------------------------
// Message map
//
BEGIN_MESSAGE_MAP(CDeviceInfoDlg, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// CDeviceInfoDlg message handlers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// OnInitDialog
// Simply queries the specified device for a set of capabilities and uses a list
// control to format the information.
BOOL CDeviceInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CComPtr<IGraphBuilder> pGraph;
	if (SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&pGraph))))
	{
		CComPtr<IBaseFilter> pFilter;
		if (SUCCEEDED(CDSUtils::AddFilter2(pGraph, *m_pCategory, m_DeviceName.c_str(), &pFilter)))
		{
			// Determine the display name of the device.
			basic_string<WCHAR> DeviceName;
			LPWSTR pVendorInfo = NULL;
			pFilter->QueryVendorInfo(&pVendorInfo);
			if (pVendorInfo)
			{
				DeviceName = pVendorInfo;
				CoTaskMemFree(reinterpret_cast<LPVOID>(pVendorInfo));
			}
			else
			{
				DeviceName = m_DeviceName;
			}
			DeviceName += L" Capabilities";
			
			CComQIPtr<IDecklinkIOControl, &IID_IDecklinkIOControl> pIDecklinkIOControl = pFilter;
			if (pIDecklinkIOControl)
			{
				// Query the device for its supported features.
				unsigned long Features = 0;
				pIDecklinkIOControl->GetIOFeatures(&Features);
				if (Features)
				{
					m_DeviceInfoCtrl.InsertColumn(0, TEXT("Property"), LVCFMT_LEFT, 200);
					m_DeviceInfoCtrl.InsertColumn(1, TEXT("Attribute"), LVCFMT_CENTER, 150);

					// Set the dialog window text to the product name of the device.
					SetWindowText(CW2T(DeviceName.c_str()));

					// Fill out the properties of the device.
					int ItemIndex = 0;
					bool bHDCapable = (Features & DECKLINK_IOFEATURES_SUPPORTSHD) ? true : false;

					// Video input.
  					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT(""));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, TEXT(""));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Composite Video Input"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASCOMPOSITEVIDEOINPUT) ? TEXT("Supported") : TEXT("X"));
					
					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Component Video Input"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASCOMPONENTVIDEOINPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    S-Video Input"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASSVIDEOINPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    SDI Video Input"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASDIGITALVIDEOINPUT) ? (bHDCapable ? TEXT("HD/SD-SDI") : TEXT("SD-SDI")) : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Dual Link Input"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASDUALLINKINPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Multicamera Input"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_SUPPORTSMULTICAMERAINPUT) ? TEXT("Supported") : TEXT("X"));

					// Video output.
					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT(""));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, TEXT(""));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Composite Video Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASCOMPONENTVIDEOOUTPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Component Video Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASCOMPOSITEVIDEOOUTPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    S-Video Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASSVIDEOOUTPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    SDI Video Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASDIGITALVIDEOOUTPUT) ? (bHDCapable ? TEXT("HD/SD-SDI") : TEXT("SD-SDI")) : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    DVI Video Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASDVIVIDEOOUTPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Dual Link Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASDUALLINKOUTPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Video Output HD Downconversion"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_SUPPORTSHDDOWNCONVERSION) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    2K Video Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_SUPPORTS2KOUTPUT) ? TEXT("Supported") : TEXT("X"));

					// Audio input.
  					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT(""));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, TEXT(""));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Analogue Audio Input"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASANALOGUEAUDIOINPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    AES-S/PDIF Audio Input"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASAESAUDIOINPUT) ? TEXT("Supported") : TEXT("X"));

					// Audio output.
					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT(""));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, TEXT(""));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Analogue Audio Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_HASANALOGUEAUDIOINPUT) ? TEXT("Supported") : TEXT("X"));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    AES-S/PDIF Audio Output"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (basic_string<WCHAR>::npos != DeviceName.find(L"DeckLink")) ? TEXT("Supported") : TEXT("X"));	// NOTE: DeckLink Pro has 4 outputs, all other DeckLinks have 1

					// Misc.
					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT(""));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, TEXT(""));

					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    Internal Keying"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_SUPPORTSINTERNALKEY) ? TEXT("Supported") : TEXT("X"));
					
					m_DeviceInfoCtrl.InsertItem(ItemIndex, TEXT("    External Keying"));
					m_DeviceInfoCtrl.SetItemText(ItemIndex++, 1, (Features & DECKLINK_IOFEATURES_SUPPORTSEXTERNALKEY) ? TEXT("Supported") : TEXT("X"));
				}
			}
		}
	}
	
	return TRUE;
}
