//
//  DecklinkCallback.h
//  ViljensTriumf
//
//  Created by Jonas on 10/3/12.
//
//
#include <vector>

#include "DeckLinkAPI.h"
#import <QuartzCore/QuartzCore.h>
#import <QTKit/QTKit.h>

class DecklinkCallback : public IDeckLinkInputCallback{
public:
    DecklinkCallback();
    
    unsigned char * bytes;
    NSBitmapImageRep * imageRep;
    
    int w;
    int h;
    int size;
    bool newFrame;
    
    unsigned char                                red[256][256];
    unsigned char                                blue[256][256];
    unsigned char                                green[256][256][256];
    
    unsigned char * rgb;

        bool cameraActive;
    // IUnknown needs only a dummy implementation
	virtual HRESULT		QueryInterface (REFIID iid, LPVOID *ppv)	{return E_NOINTERFACE;}
	virtual ULONG		AddRef ()									{return 1;}
	virtual ULONG		Release ()									{return 1;}
    
    
    
    virtual HRESULT		VideoInputFormatChanged (/* in */ BMDVideoInputFormatChangedEvents notificationEvents, /* in */ IDeckLinkDisplayMode *newDisplayMode, /* in */ BMDDetectedVideoInputFormatFlags detectedSignalFlags);
	virtual HRESULT		VideoInputFrameArrived (/* in */ IDeckLinkVideoInputFrame* videoFrame, /* in */ IDeckLinkAudioInputPacket* audioPacket);
    
    IDeckLinkOutput * decklinkOutput;
    
    unsigned char Clamp(int value);
    void CreateLookupTables();
    void YuvToRgbChunk(unsigned char *yuv, unsigned char * rgb, unsigned int offset, unsigned int chunk_size, unsigned int rowBytes);
    unsigned char * YuvToRgb(IDeckLinkVideoInputFrame* pArrivedFrame);
    
    NSLock * lock;
    
    id delegate;
    bool delegateBusy;
    
        CVPixelBufferRef buffer;
    
    int counter;
    
    IDeckLinkVideoInputFrame* _videoFrame;
    
    bool even;

};

