// GdiExample.cpp : Defines the entry point for the console application.
//
#include <conio.h>
#include <objbase.h>		// Necessary for COM
#include <comutil.h>
#include "DeckLinkAPI_h.h"
#include "stdafx.h"
#include <stdio.h>

void OutputGraphic (IDeckLink* deckLink);
void FillColourBars (IDeckLinkVideoFrame* theFrame);
void GdiDraw (IDeckLinkVideoFrame* theFrame);

int		_tmain(int argc, _TCHAR* argv[])
{
	IDeckLinkIterator*		deckLinkIterator = NULL; 
	IDeckLink*				deckLink = NULL;
	int						numDevices = 0; 
	HRESULT					result; 

	printf("GDI Sample Application\n\n"); 
	// Initialize COM on this thread
	result = CoInitialize(NULL);
	if (FAILED(result))
	{
		fprintf(stderr, "Initialization of COM failed - result = %08x.\n", result);
		return 1;
	}
	
	// Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
	result = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&deckLinkIterator);
	if (FAILED(result))
	{
		fprintf(stderr, "A DeckLink iterator could not be created.  The DeckLink drivers may not be installed.\n");
		goto bail;
	}

	// Enumerate all cards in this system 
	while (deckLinkIterator->Next(&deckLink) == S_OK) 
	{ 
		BSTR	deviceNameBSTR = NULL; 
		 
		// Increment the total number of DeckLink cards found 
		numDevices++; 
		if (numDevices > 1) 
			printf("\n\n");	 
		 
		// Print the model name of the DeckLink card 
		result = deckLink->GetModelName(&deviceNameBSTR); 
		if (result == S_OK) 
		{
			_bstr_t deviceName(deviceNameBSTR);		
			printf("Found Blackmagic device: %s\n", (char*)deviceName); 
			OutputGraphic(deckLink);
 		}		 
		deckLink->Release(); 
 	} 

bail:
	if (deckLinkIterator != NULL)
		deckLinkIterator->Release();

	// If no DeckLink cards were found in the system, inform the user
	if (numDevices == 0) 
		printf("No Blackmagic Design devices were found.\n"); 
 	printf("\n");

	// Uninitalize COM on this thread
	CoUninitialize();

	return 0;
}

// Prepare video output
void	OutputGraphic (IDeckLink* deckLink)
{
	IDeckLinkOutput*			m_deckLinkOutput = NULL;
	IDeckLinkMutableVideoFrame*	m_videoFrameGdi = NULL;

	// Obtain the audio/video output interface (IDeckLinkOutput)
	if (deckLink->QueryInterface(IID_IDeckLinkOutput, (void**)&m_deckLinkOutput) != S_OK)
	{
		fprintf(stderr, "Could not obtain the IDeckLinkOutput interface\n");
	}
	else
	{	
		IDeckLinkDisplayModeIterator* displayModeIterator = NULL;
		IDeckLinkDisplayMode* deckLinkDisplayMode = NULL;
		
		// Obtain a display mode iterator
		if (m_deckLinkOutput->GetDisplayModeIterator(&displayModeIterator) != S_OK)
		{
			fprintf(stderr, "Could not obtain the display mode iterator\n");
		}
		else
		{
			// Get the first display mode
			if (displayModeIterator->Next(&deckLinkDisplayMode) == S_OK)
			{
				if (deckLinkDisplayMode != NULL)
				{
					BSTR modeNameBSTR;
				
					if (deckLinkDisplayMode->GetName(&modeNameBSTR) == S_OK)
					{
						unsigned long m_frameWidth = 0;
						unsigned long m_frameHeight = 0;
						_bstr_t modeName(modeNameBSTR);

						// prints information about the display mode
						m_frameWidth = deckLinkDisplayMode->GetWidth();
						m_frameHeight = deckLinkDisplayMode->GetHeight();
						printf("Using video mode: %s, width: %lu, height: %lu\n", (char*)modeName, m_frameWidth, m_frameHeight);

						// enables video output on the DeckLink card
						if (m_deckLinkOutput->EnableVideoOutput(deckLinkDisplayMode->GetDisplayMode(), bmdVideoOutputFlagDefault) != S_OK)
						{
							fprintf(stderr, "Could not enable Video output\n");
						}
						else
						{
							// Get a BGRA frame 
							if (m_deckLinkOutput->CreateVideoFrame(m_frameWidth, m_frameHeight, m_frameWidth*4, bmdFormat8BitBGRA, bmdFrameFlagFlipVertical, &m_videoFrameGdi) != S_OK)
							{
								fprintf(stderr, "Could not obtain the IDeckLinkOutput CreateVideoFrame interface\n");
							}
							else
							{
								// draw on the frame
								GdiDraw (m_videoFrameGdi);

								// display the frame
								m_deckLinkOutput->DisplayVideoFrameSync(m_videoFrameGdi);
								Sleep(10000);

								// release the frame
								m_videoFrameGdi->Release();
							}

							// disable the video output
							m_deckLinkOutput->DisableVideoOutput();
						}
					}
				}

				// release the display mode
				deckLinkDisplayMode->Release();
			}
			else
			{
				printf("Could not obtain a display mode\n");
			}

			// release the display mode iterator
			displayModeIterator->Release();
		}

		// release the DeckLink Output interface
		m_deckLinkOutput->Release();
	}
}

// Use GDI to draw in video frame
void	GdiDraw (IDeckLinkVideoFrame* theFrame)
{
	// This function create a GDI bitmap, draws basic shapes in it,
	// and copies its content to the buffer of the DeckLink frame given in argument.


	BITMAPINFOHEADER bmi;
	
	// setup the bitmap info header members
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.biSize = sizeof(bmi);
	bmi.biWidth = theFrame->GetWidth();
	bmi.biHeight = theFrame->GetHeight();
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = (bmi.biWidth * bmi.biHeight * 4);
	
	// create a device context
	HDC hdc = CreateCompatibleDC(NULL);

	// create background and foreground rectangles
	RECT fillRect = {0, 0, bmi.biWidth, bmi.biHeight};					
	RECT fillRect2 = {50, 50, 100, 100};
	RECT fillRect3 = {100, 100, 150, 150};
	RECT fillRect4 = {150, 150, 200, 200};

	// pointer to the bitmap buffer
	BYTE* pbData = NULL;

	// create the bitmap and attach it to our device context
	HBITMAP hbm = CreateDIBSection(hdc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&pbData, NULL, 0);
	SelectObject(hdc, hbm);

	// create a brush to draw objects with
	HBRUSH	backBrush = (HBRUSH)GetStockObject(DC_BRUSH);
	SetDCBrushColor(hdc, RGB(50, 50, 110));

	// draw background rectangle
	FillRect(hdc, &fillRect, backBrush);

	// draw string in centre of screen
	TextOut(hdc, bmi.biWidth/2, bmi.biHeight/2, _T("Hello, world!"), 13);	

	// draw red, green and blue foreground rectangles
	SetDCBrushColor(hdc, RGB(222, 20, 0));								
	FillRect(hdc, &fillRect2, backBrush);
	SetDCBrushColor(hdc, RGB(0, 222, 0));
	FillRect(hdc, &fillRect3, backBrush);
	SetDCBrushColor(hdc, RGB(0, 0, 222));
	FillRect(hdc, &fillRect4, backBrush);

	// draw ellipse
	Ellipse(hdc, 200, 200,300,300);
	
	// get a pointer to our DeckLink's frame buffer
	BYTE* pbDestData = NULL;
	theFrame->GetBytes((void**)&pbDestData);

	// copy the bitmap buffer content to the DeckLink frame buffer
	memcpy(pbDestData, pbData, bmi.biSizeImage);

	// delete attached GDI object and free bitmap memory
	DeleteObject(SelectObject(hdc, hbm));
}
