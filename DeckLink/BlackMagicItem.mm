//
//  BlackMagicItem.m
//  SH
//
//  Created by Flyvende Grise on 1/10/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "BlackMagicItem.h"

@interface BlackMagicItem ()
@end
@implementation BlackMagicItem




-(id) initWithDecklink:(IDeckLink*)deckLink mode:(int)mode{
    self = [self init];
    
    if(self){
        IDeckLinkDisplayModeIterator*	displayModeIterator = NULL;
        
        IDeckLinkDisplayMode*			displayMode = NULL;
        std::vector<IDeckLinkDisplayMode*>	modeList;
        
        self.callback = new DecklinkCallback();
        self.callback->delegate = self;
        
        // self.glhelper = CreateOpenGLScreenPreviewHelper();
        
        // Get the IDeckLinkInput for the selected device
        if ((deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&_deckLinkInput) != S_OK))
        {
            NSLog(@"This application was unable to obtain IDeckLinkInput for the selected device.");
        }
        
        if ((deckLink->QueryInterface(IID_IDeckLinkOutput, (void**)&_deckLinkOutput) != S_OK))
        {
            NSLog(@"This application was unable to obtain IDeckLinkOutput for the selected device.");
        }
        
        
        //
        // Retrieve and cache mode list
        if (self.deckLinkInput->GetDisplayModeIterator(&displayModeIterator) == S_OK)
        {
            CFStringRef			modeName;
            int i=0;
            
            while (displayModeIterator->Next(&displayMode) == S_OK){
                modeList.push_back(displayMode);
                
                if (displayMode->GetName(&modeName) == S_OK)
                {
                    NSLog(@"Mode: %i %@",i++,(__bridge NSString *)modeName);
                }
            }
            
            displayModeIterator->Release();
        }
        
        
        
        
        // Set capture callback
        BMDVideoInputFlags		videoInputFlags = bmdVideoInputFlagDefault;
        
        self.deckLinkInput->SetCallback(self.callback);
        
        self.callback->decklinkOutput = self.deckLinkOutput;
        
        
        // Set the video input mode
        CFStringRef			modeName;
        modeList[2]->GetName(&modeName);
        self.modeDescription = (__bridge NSString*)modeName;
        
        if (self.deckLinkInput->EnableVideoInput(modeList[mode]->GetDisplayMode(), bmdFormat8BitYUV, videoInputFlags) != S_OK)
        {
            /*  [uiDelegate showErrorMessage:@"This application was unable to select the chosen video mode. Perhaps, the selected device is currently in-use." title:@"Error starting the capture"];
             return false;*/
            NSLog(@"This application was unable to select the chosen video mode. Perhaps, the selected device is currently in-use.");
        }
        
        HRESULT				theResult;
        /*   // Turn on video output
         theResult = deckLinkOutputs[index]->EnableVideoOutput(modeList[2]->GetDisplayMode(), bmdVideoOutputFlagDefault);
         if (theResult != S_OK)
         printf("EnableVideoOutput failed with result %08x\n", (unsigned int)theResult);
         //
         theResult = deckLinkOutputs[index]->StartScheduledPlayback(0, 600, 1.0);
         if (theResult != S_OK)
         printf("StartScheduledPlayback failed with result %08x\n", (unsigned int)theResult);
         */
        
        
        // Start the capture
        if (self.deckLinkInput->StartStreams() != S_OK)
        {
            NSLog(@"This application was unable to start the capture. Perhaps, the selected device is currently in-use.");
            /*  [uiDelegate showErrorMessage:@"This application was unable to start the capture. Perhaps, the selected device is currently in-use." title:@"Error starting the capture"];
             return false;*/
        }
        
        
        
        
        // result = true;
    }
    
    return self;
    
}


static dispatch_once_t onceToken;

-(void) newFrame:(DecklinkCallback*)callback{

    callback->delegateBusy = YES;
    //        CVPixelBufferRef buffer = [self createCVImageBufferFromCallback:callback];
    CVPixelBufferRef buffer = callback->buffer;
    if(!buffer){
        NSLog(@"No buffer");
    } else {
        /*
         int num = -1;
         if(callback == [self.blackMagicController callbacks:0]){
         num = 0;
         }
         else if(callback == [self.blackMagicController callbacks:1]){
         num = 1;
         }
         else if(callback == [self.blackMagicController callbacks:2]){
         num = 2;
         }
         
         NSTimeInterval time = [NSDate timeIntervalSinceReferenceDate];
         
         //      dispatch_sync(dispatch_get_main_queue(), ^{
         
         */
        
        //         unsigned t0=clock(),t1;
        
        
          dispatch_group_t group = dispatch_group_create();
         dispatch_queue_t queue = dispatch_queue_create("com.halfdanj.imageWithCVImage", 0);
         dispatch_queue_t queue2 = dispatch_queue_create("com.halfdanj.recorder", DISPATCH_QUEUE_SERIAL);
         
        __block CIImage * image;
        dispatch_group_async(group, queue, ^{
        image = [CIImage imageWithCVImageBuffer:buffer];
                });
        
        
         dispatch_group_async(group, queue2, ^{
         [self.delegate newFrameWithBufer:buffer image:nil item:self];
         });
         
         dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
         
        //   t1=clock()-t0;
        // printf("%i\n",t1);
        
        if(!image){
            NSLog(@"No image");
        } else {
            //dispatch_async(dispatch_get_main_queue(), ^{
            self.inputImage  = image;
            // });
        }
    }
    
    callback->delegateBusy = NO;
    
    // }
    /*
     CoreImageViewer * preview = nil;
     switch (num) {
     case 0:
     preview = self.preview1;
     break;
     case 1:
     preview = self.preview2;
     break;
     case 2:
     preview = self.preview3;
     break;
     
     default:
     break;
     }
     
     
     if(self.playVideo){
     if(num == 1){
     //            avPlayerLayer.filters
     ChromaFilter * filter = [avPlayerLayer valueForKeyPath:@"filters.chroma"];
     if(filter){
     //             NSLog(@" Filter %@",avPlayerLayer.filters);
     image = [self filterCIImage:image];
     
     
     [avPlayerLayer  setValue:image forKeyPath:@"filters.chroma.inputBackgroundImage"];
     }
     
     }
     } else {
     if(!self.recording){
     if(num == 0  && [[NSUserDefaults standardUserDefaults] boolForKey:@"chromaKey"] && self.decklink1input == 8){
     image = [self chromaKey:image backgroundImage:cameras[1]];
     }
     if(num == 0 && [[NSUserDefaults standardUserDefaults] floatForKey:@"chromaScale"] != 1 && self.decklink1input == 8){
     [self updateChromaTransform];
     //                [self.chromaTransform setValue:image forKey:@"inputImage"];
     //                image = [self.chromaTransform valueForKey:@"outputImage"];
     
     [self.chromaCrop setValue:image forKey:@"inputImage"];
     image = [self.chromaCrop valueForKey:@"outputImage"];
     }
     
     image = [self filterCIImage:image];
     
     
     cameras[num] = image;
     
     
     //  dispatch_async(dispatch_get_main_queue(), ^{
     
     if(num == self.outSelector-1 || self.outSelector == 0 || self.outSelector > 3){
     self.mainOutput.ciImage = [self outputImage];
     //if(![self.mainOutput needsDisplay])
     [self.mainOutput setNeedsDisplay:YES];
     }
     //            if(!self.mainOutput.needsDisplay){ //Spar på energien
     preview.ciImage = [self imageForSelector:num+1];
     //     [preview performSelector:@selector(setNeedsDisplay:) withObject:YES afterDelay:1];
     [preview setNeedsDisplay:YES];
     //          }
     //[NSThread sleepForTimeInterval:0.01];
     }
     
     if(self.recording && num == self.outSelector - 1){
     //  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0ul), ^{
     NSTimeInterval diffTime = time - self.startRecordTime;
     int frameCount = diffTime*25.0;
     
     BOOL append_ok = NO;
     int j = 0;
     
     if (adaptor.assetWriterInput.readyForMoreMediaData)
     {
     //                    printf("appending %d attemp %d\n", frameCount, j);
     
     CMTime frameTime = CMTimeMake(frameCount,(int32_t) 25.0);
     append_ok = [adaptor appendPixelBuffer:buffer withPresentationTime:frameTime];
     
     
     //if(buffer)
     //  CVBufferRelease(buffer);
     [NSThread sleepForTimeInterval:0.035];
     }
     else
     {
     printf("adaptor not ready %d, %d\n", frameCount, j);
     // [NSThread sleepForTimeInterval:0.1];
     }
     j++;
     //}
     if (!append_ok) {
     printf("error appending image %d times %d\n", frameCount, j);
     }
     
     // });
     }
     }*/
    
    //[callback->lock unlock];
    //        NSLog(@"New frame out %i",callback);
    
    //      NSLog(@"---Main stop");
    
    // });
    
    //  NSLog(@"--New Frame stop");
}

-(NSString *)name{
    return [NSString stringWithFormat:@"%i - %@",self.index, self.modeDescription];
}

+(NSSet *)keyPathsForValuesAffectingName{
    return [NSSet setWithObjects:@"modeDescription",@"index", nil];
}


void MyPixelBufferReleaseCallback(void *releaseRefCon, const void *baseAddress){
    //   NSLog(@" release %i",baseAddress);
    //   delete (unsigned char*)baseAddress;
    
}




-(CVPixelBufferRef) createCVImageBufferFromCallback:(DecklinkCallback*)callback{
    int w = callback->w;
    int h = callback->h;
    //  unsigned char * bytes = callback->bytes;
    
    dispatch_once(&onceMallocToken, ^{
        bytes = (unsigned char * ) malloc(callback->w*callback->h*4 * sizeof(unsigned char)) ;
    });
    
    memcpy(bytes, callback->bytes, callback->w*callback->h*4);
    //    NSLog(@" create %i",bytes);
    
    
    NSDictionary *d = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey, [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, nil];
    
    
    CVPixelBufferRef buffer = NULL;
    
    CVPixelBufferCreateWithBytes(kCFAllocatorDefault, w, h, k32ARGBPixelFormat, bytes, 4*w, (CVPixelBufferReleaseBytesCallback )MyPixelBufferReleaseCallback, (void*)bytes, (__bridge CFDictionaryRef)d, &buffer);
    // NSLog(@"buffer %i",buffer);
    return buffer;
}

@end
