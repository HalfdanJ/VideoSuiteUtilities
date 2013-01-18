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
    for (int v = 0; v < 256; v++) {
        for (int y = 0; y < 256; y++) {
            yy         = y << 8;
            vv         = v - 128;
            vr         = vv * 359;
            val        = (yy + vr) >>  8;
            red[v][y]  = Clamp(val);
        }
    }
    
    // Blue
    for (int u = 0; u < 256; u++) {
        for (int y = 0; y < 256; y++) {
            yy          = y << 8;
            uu          = u - 128;
            ub          = uu * 454;
            val         = (yy + ub) >> 8;
            blue[u][y]  = Clamp(val);
        }
    }
    
    // Green
    for (int u = 0; u < 256; u++) {
        for (int v = 0; v < 256; v++) {
            for (int y = 0; y < 256; y++) {
                yy              = y << 8;
                uu              = u - 128;
                vv              = v - 128;
                ug_plus_vg      = uu * 88 + vv * 183;
                val             = (yy - ug_plus_vg) >> 8;
                green[u][v][y]  = Clamp(val);
            }
        }
    }
}




void DecklinkCallback::YuvToRgbChunk(unsigned char *yuv, unsigned char * rgb, unsigned int offset, unsigned int chunk_size, unsigned int rowBytes)
{
    
    // convert 4 YUV macropixels to 6 RGB pixels
	unsigned int i, j;
    unsigned int boundry = offset + chunk_size;
    unsigned char y, u, v;
    
    int yy, uu, vv, ug_plus_vg, ub, vr, val;
    
    int num = (offset/4)*8;
    for(i=offset, j=num; i<boundry; i+=4, j+=8){
//        if(i % rowBytes*2 < rowBytes)
        
        y = yuv[i+1];
        u = yuv[i];
        v = yuv[i+2];
        
        yy         = y << 8;
        vv         = v - 128;
        uu              = u - 128;
        
        unsigned char * r = red[v];
        unsigned char * g = green[u][v];
        unsigned char * b = blue[u];
        
        rgb[j+1] =r[y];
        rgb[j+2] = g[y];
        rgb[j+3] = b[y];
        /*
         // rgb[j+1]   =red[y][v];
         {
         vr         = vv * 359;
         val        = (yy + vr) >>  8;
         rgb[j+1]  = Clamp(val);
         
         }
         
         //rgb[j+2] = green[y][u][v];
         {
         ug_plus_vg      = uu * 88 + vv * 183;
         val             = (yy - ug_plus_vg) >> 8;
         rgb[j+2]  = Clamp(val);
         }
         //  rgb[j+3] = blue[y][u];
         {
         ub          = uu * 454;
         val         = (yy + ub) >> 8;
         rgb[j+3]  = Clamp(val);
         }*/
        /*
         rgb[j+3] = 1.164*(y - 16)                   + 2.018*(u - 128);
         rgb[j+2] = 1.164*(u - 16) - 0.813*(v - 128) - 0.391*(u - 128);
         rgb[j+1] = 1.164*(v - 16) + 1.596*(v - 128);
         */
        y = yuv[i+3];
        
        yy         = y << 8;
        
        rgb[j+5] =r[y];
        rgb[j+6] = g[y];
        rgb[j+7] = b[y];
        /*
         // rgb[j+1]   =red[y][v];
         {
         val        = (yy + vr) >>  8;
         rgb[j+5]  = Clamp(val);
         
         }
         
         //rgb[j+2] = green[y][u][v];
         {
         val             = (yy - ug_plus_vg) >> 8;
         rgb[j+6]  = Clamp(val);
         }
         //  rgb[j+3] = blue[y][u];
         {
         val         = (yy + ub) >> 8;
         rgb[j+7]  = Clamp(val);
         }
         */
        /*        rgb[j+3] = 1.164*(y - 16)                   + 2.018*(u - 128);
         rgb[j+2] = 1.164*(u - 16) - 0.813*(v - 128) - 0.391*(u - 128);
         rgb[j+1] = 1.164*(v - 16) + 1.596*(v - 128);
         */
        
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
    //   unsigned t0=clock(),t1;
    
    // split up the image into memory-aligned chunks so they take advantage of
    // the CPU cache
    int     mConversionChunkSize = pArrivedFrame->GetRowBytes() * (long)ceil(pArrivedFrame->GetHeight() /(float) num_workers);
    
    int rowBytes = pArrivedFrame->GetRowBytes();
    
    dispatch_queue_t queue = dispatch_queue_create("com.halfdanj.yuv", 0);
    dispatch_group_t group = dispatch_group_create();
    
    for(int i=0;i<num_workers;i++){
        dispatch_group_async(group,queue,^{
            YuvToRgbChunk(yuv,rgb, mConversionChunkSize*i, mConversionChunkSize, rowBytes);
        });
    }
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    
    //   t1=clock()-t0;
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
    //NSLog(@"-Frame arrived start");
    even = !even;
    @autoreleasepool {
        if(!delegateBusy){
            
            if(!_videoFrame && [lock tryLock]){
                
                _videoFrame = videoFrame;
                _videoFrame->AddRef();
                
                //dispatch_queue_t queue = dispatch_get_main_queue();
                dispatch_queue_t queue = dispatch_queue_create("com.halfdanj.newFrame", 0);
                dispatch_async(queue, ^{
                    [lock lock];
                    
                    BMDTimeValue		frameTime, frameDuration;
                    _videoFrame->GetStreamTime(&frameTime, &frameDuration, 600);
                    decklinkOutput->ScheduleVideoFrame(_videoFrame, frameTime, frameDuration, 600);
                    
                    w = (int)_videoFrame->GetWidth();
                    h = (int)_videoFrame->GetHeight();
                    //                    NSLog(@"%i",_videoFrame->GetFlags());
                    size = w * h * 4;
                    
                    // NSLog(@"%i %i",w,h);
                    
                    bytes = YuvToRgb(videoFrame);
                    //                    videoFrame->GetBytes((void**)&bytes);
                    
                    
                    if(buffer){
                        CVPixelBufferRelease(buffer);
                    }
                    buffer = [delegate createCVImageBufferFromCallback:this];
                    
                    newFrame = true;
                    
                    delegateBusy = YES;
                    [delegate newFrame:this];
                    
                    _videoFrame->Release();
                    _videoFrame = nil;
                    [lock unlock];
                });
                
                [lock unlock];
                
            } else {
                //   NSLog(@"Could not arqquire lock");
            }
            //   NSLog(@"Frame out %i",this);
        } else {
            // NSLog(@"###########busy delegate");
        }
    }
    
    //  NSLog(@"-Frame arrived stop");
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