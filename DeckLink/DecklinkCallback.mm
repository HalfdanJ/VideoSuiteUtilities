//
//  DecklinkCallback.m
//  ViljensTriumf
//
//  Created by Jonas on 10/3/12.
//
//

#import "DecklinkCallback.h"


// YUV format conforms to ITU.BT-601
//
// http://www.fourcc.org/yuv.php#UYVY
// http://www.martinreddy.net/gfx/faqs/colorconv.faq
// http://developer.apple.com/quicktime/icefloe/dispatch027.html
// http://www.fourcc.org/source/YUV420P-OpenGL-GLSLang.c
//
//
// Check out bit layout for "bmdFormat8BitYUV : UYVY 4:2:2 Representation"
// on page 204 of the Decklink SDK documentation
//
// Below formulas from Color Space Conversions on pg 227 of the Decklink SDK documentation
// R = 1.164(Y - 16) + 1.793(Cr - 128)
// G = 1.164(Y - 16) - 0.534(Cr - 128) - 0.213(Cb - 128)
// B = 1.164(Y - 16) + 2.115(Cb - 128)


#import "AppDelegate.h"


// clamp values between 0 and 255
unsigned char DecklinkCallback::Clamp(int value)
{
    if(value > 255) return 255;
    if(value < 0)   return 0;
    return value;
}


void DecklinkCallback::CreateLookupTables(){
    
    int yy, uu, vv, ug_plus_vg, ub, vr, val;
    
    // Red
    for (int y = 0; y < 256; y++) {
        for (int v = 0; v < 256; v++) {
            yy         = y << 8;
            vv         = v - 128;
            vr         = vv * 359;
            val        = (yy + vr) >>  8;
            red[y][v]  = Clamp(val);
        }
    }
    
    // Blue
    for (int y = 0; y < 256; y++) {
        for (int u = 0; u < 256; u++) {
            yy          = y << 8;
            uu          = u - 128;
            ub          = uu * 454;
            val         = (yy + ub) >> 8;
            blue[y][u]  = Clamp(val);
        }
    }
    
    // Green
    for (int y = 0; y < 256; y++) {
        for (int u = 0; u < 256; u++) {
            for (int v = 0; v < 256; v++) {
                yy              = y << 8;
                uu              = u - 128;
                vv              = v - 128;
                ug_plus_vg      = uu * 88 + vv * 183;
                val             = (yy - ug_plus_vg) >> 8;
                green[y][u][v]  = Clamp(val);
            }
        }
    }
}




void DecklinkCallback::YuvToRgbChunk(unsigned char *yuv, unsigned char * rgb, unsigned int offset, unsigned int chunk_size)
{

    // convert 4 YUV macropixels to 6 RGB pixels
	unsigned int i, j;
    unsigned int boundry = offset + chunk_size;
    int y, u, v;
    
    for(i=offset, j=(offset/4)*8; i<boundry; i+=4, j+=8){
        y = yuv[i+1];
        u = yuv[i];
        v = yuv[i+2];
        
        rgb[j+1]   =red[y][v];
        rgb[j+2] = green[y][u][v];
        rgb[j+3] = blue[y][u];
        
        y = yuv[i+3];
        
        rgb[j+5] =red[y][v];
        rgb[j+6] = green[y][u][v];
        rgb[j+7] = blue[y][u];
    }
    
    /*
     // fixed point math implementation - superceded by the lookup table method
     unsigned int i, j;
     unsigned int boundry = offset + chunk_size;
     int yy, uu, vv, ug_plus_vg, ub, vr;
     int r,g,b;
     for(i=offset, j=(offset/4)*6; i<boundry; i+=4, j+=6){
     yy = yuv[i+1] << 8;
     uu = yuv[i] - 128;
     vv = yuv[i+2] - 128;
     ug_plus_vg = uu * 88 + vv * 183;
     ub = uu * 454;
     vr = vv * 359;
     r = (yy + vr) >> 8;
     g = (yy - ug_plus_vg) >> 8;
     b = (yy + ub) >> 8;
     rgb->pixels[j]   = r < 0 ? 0 : (r > 255 ? 255 : (unsigned char)r);
     rgb->pixels[j+1] = g < 0 ? 0 : (g > 255 ? 255 : (unsigned char)g);
     rgb->pixels[j+2] = b < 0 ? 0 : (b > 255 ? 255 : (unsigned char)b);
     yy = yuv[i+3] << 8;
     r = (yy + vr) >> 8;
     g = (yy - ug_plus_vg) >> 8;
     b = (yy + ub) >> 8;
     rgb->pixels[j+3] = r < 0 ? 0 : (r > 255 ? 255 : (unsigned char)r);
     rgb->pixels[j+4] = g < 0 ? 0 : (g > 255 ? 255 : (unsigned char)g);
     rgb->pixels[j+5] = b < 0 ? 0 : (b > 255 ? 255 : (unsigned char)b);
     }
     */
}


unsigned char * DecklinkCallback::YuvToRgb(IDeckLinkVideoInputFrame* pArrivedFrame)
{
    unsigned char * yuv;
    pArrivedFrame->GetBytes((void**)&yuv);
    
    // allocate space for the rgb image
    if(rgb == nil){
        int size = pArrivedFrame->GetWidth() * pArrivedFrame->GetHeight()*4*sizeof(unsigned char);
        rgb = (unsigned char *) malloc(size);
        memset(rgb,255,size);
    }
    //    shared_ptr<DLFrame> rgb(new DLFrame(mCaptureWidth, mCaptureHeight, mRgbRowBytes, DLFrame::DL_RGB));
    
    int num_workers = 8;
    
    int a;
  // unsigned t0=clock(),t1;
    
    // split up the image into memory-aligned chunks so they take advantage of
    // the CPU cache
    int     mConversionChunkSize = pArrivedFrame->GetRowBytes() * (long)ceil(pArrivedFrame->GetHeight() /(float) num_workers);
    
    
    dispatch_queue_t queue = dispatch_queue_create("com.halfdanj.yuv", 0);
    dispatch_group_t group = dispatch_group_create();
    
    for(int i=0;i<num_workers;i++){
        dispatch_group_async(group,queue,^{
            YuvToRgbChunk(yuv,rgb, mConversionChunkSize*i, mConversionChunkSize);
        });
    }
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    
   // t1=clock()-t0;
    //printf("%i\n",t1);
    
    return rgb;
}

void bwFrames(unsigned char * bytes, int size){
    for(int i=0;i<size;i++){
        unsigned char * r = bytes + i*3;
        unsigned char * g = bytes + i*3+1;
        unsigned char * b = bytes + i*3+2;
        int c =  (*r +  *g + *b)/3.0;
        bytes[i] = c;
    }
}




DecklinkCallback::DecklinkCallback(){
    bytes = 0;
    CreateLookupTables();
    
    lock = [[NSLock alloc] init];
};



HRESULT		DecklinkCallback::VideoInputFormatChanged (/* in */ BMDVideoInputFormatChangedEvents notificationEvents, /* in */ IDeckLinkDisplayMode *newMode, /* in */ BMDDetectedVideoInputFormatFlags detectedSignalFlags)
{
	//UInt32				modeIndex = 0;
    printf("Video format changed");
    /*
     NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
     
     // Restart capture with the new video mode if told to
     if ([uiDelegate shouldRestartCaptureWithNewVideoMode] == YES)
     {
     // Stop the capture
     deckLinkInput->StopStreams();
     
     // Set the video input mode
     if (deckLinkInput->EnableVideoInput(newMode->GetDisplayMode(), bmdFormat8BitYUV, bmdVideoInputEnableFormatDetection) != S_OK)
     {
     [uiDelegate stopCapture];
     [uiDelegate showErrorMessage:@"This application was unable to select the new video mode." title:@"Error restarting the capture."];
     goto bail;
     }
     
     // Start the capture
     if (deckLinkInput->StartStreams() != S_OK)
     {
     [uiDelegate stopCapture];
     [uiDelegate showErrorMessage:@"This application was unable to start the capture on the selected device." title:@"Error restarting the capture."];
     goto bail;
     }
     }
     
     // Find the index of the new mode in the mode list so we can update the UI
     while (modeIndex < modeList.size()) {
     if (modeList[modeIndex]->GetDisplayMode() == newMode->GetDisplayMode())
     {
     [uiDelegate selectDetectedVideoModeWithIndex: modeIndex];
     break;
     }
     modeIndex++;
     }
     
     
     bail:
     [pool release];
     return S_OK;*/
}



HRESULT 	DecklinkCallback::VideoInputFrameArrived (/* in */ IDeckLinkVideoInputFrame* videoFrame, /* in */ IDeckLinkAudioInputPacket* audioPacket)
{
    
    @autoreleasepool {
        if(!delegateBusy){
          //  NSLog(@"Frame in %i",this);
            [lock lock];
            //        BMDPixelFormat pixelFormat = videoFrame->GetPixelFormat();
            BMDTimeValue		frameTime, frameDuration;
  //          int					hours, minutes, seconds, frames;
//            HRESULT				theResult;
            
            videoFrame->GetStreamTime(&frameTime, &frameDuration, 600);
            decklinkOutput->ScheduleVideoFrame(videoFrame, frameTime, frameDuration, 600);
            //if (theResult != S_OK)
            //	printf("Scheduling failed with error = %08x\n", (unsigned int)theResult);
            
            
            
            w = (int)videoFrame->GetWidth();
            h = (int)videoFrame->GetHeight();
            size = w * h * 4;
            
            
            /* if(bytes){
             delete bytes;
             }*/
            bytes = YuvToRgb(videoFrame);
            
            if(buffer){
                CVPixelBufferRelease(buffer);
            }
            buffer = [delegate createCVImageBufferFromCallback:this];

            /*imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&bytes
             pixelsWide:w pixelsHigh:h
             bitsPerSample:8 samplesPerPixel:3
             hasAlpha:NO isPlanar:NO
             colorSpaceName:NSDeviceRGBColorSpace
             bitmapFormat:0
             bytesPerRow:3*w bitsPerPixel:8*3];        // bwFrames(bytes,w*h);
             
             */
            newFrame = true;
            
            dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0ul);
            dispatch_sync(queue, ^{
                [delegate newFrame:this];
            });
            
            [lock unlock];
         //   NSLog(@"Frame out %i",this);
        } else {
       //     NSLog(@"busy delegate");
        }
    }
    //  videoFrame->get
    
    /*	BOOL					hasValidInputSource = (videoFrame->GetFlags() & bmdFrameHasNoInputSource) != 0 ? NO : YES;
     AncillaryDataStruct		ancillaryData;
     
     
     
     // Update input source label
     [uiDelegate updateInputSourceState:hasValidInputSource];
     
     // Get the various timecodes and userbits for this frame
     getAncillaryDataFromFrame(videoFrame, bmdTimecodeVITC, &ancillaryData.vitcF1Timecode, &ancillaryData.vitcF1UserBits);
     getAncillaryDataFromFrame(videoFrame, bmdTimecodeVITCField2, &ancillaryData.vitcF2Timecode, &ancillaryData.vitcF2UserBits);
     getAncillaryDataFromFrame(videoFrame, bmdTimecodeRP188VITC1, &ancillaryData.rp188vitc1Timecode, &ancillaryData.rp188vitc1UserBits);
     getAncillaryDataFromFrame(videoFrame, bmdTimecodeRP188LTC, &ancillaryData.rp188ltcTimecode, &ancillaryData.rp188ltcUserBits);
     getAncillaryDataFromFrame(videoFrame, bmdTimecodeRP188VITC2, &ancillaryData.rp188vitc2Timecode, &ancillaryData.rp188vitc2UserBits);
     
     // Update the UI
     [uiDelegate updateAncillaryData:&ancillaryData];
     
     [pool release];
     return S_OK;*/
    return S_OK;
}