//
//  VideoBankRecorder.m
//  SH
//
//  Created by Flyvende Grise on 1/10/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "VideoBankRecorder.h"
#import "NSString+Timecode.h"

@interface VideoBankRecorder ()

@property BlackMagicItem * deviceItem;
@property NSArray * blackmagicItems;
@property NSTimeInterval startRecordTime;

@property AVAssetWriter *videoWriter;
@property AVAssetWriterInput* videoWriterInput ;
@property AVAssetWriterInputPixelBufferAdaptor *adaptor;

@property dispatch_queue_t assetWriterQueue;

@property NSRecursiveLock * lock;


@end

@implementation VideoBankRecorder
static void *DeviceIndexContext = &DeviceIndexContext;
static void *RecordContext = &RecordContext;
static void *LabelContext = &LabelContext;


-(id)initWithBlackmagicItems:(NSArray*)items bank:(VideoBank*)bank
{
    self = [self init];
    if (self) {
        self.blackmagicItems = items;
        self.videoBank = bank;
        self.deviceIndex = -1;
        self.readyToRecord = YES;
        
        [self addObserver:self forKeyPath:@"deviceIndex" options:0 context:DeviceIndexContext];
        [self addObserver:self forKeyPath:@"record" options:0 context:RecordContext];
        
        [self addObserver:self forKeyPath:@"bankIndex" options:0 context:LabelContext];
        [self addObserver:self forKeyPath:@"record" options:0 context:LabelContext];
        
        self.bankIndex = 0;
        self.deviceIndex = 0;
        
        self.lock = [[NSRecursiveLock alloc] init];
        
        self.assetWriterQueue = dispatch_queue_create("AssetWriterQueue", DISPATCH_QUEUE_SERIAL);
        [self prepareRecording];
        
    }
    return self;
}


-(void)newFrameWithBufer:(CVPixelBufferRef)buffer image:(CIImage *)image item:(BlackMagicItem*)bmItem{
    
    if(self.record && self.readyToRecord){
        if(!self.startRecordTime){
            self.startRecordTime = [NSDate timeIntervalSinceReferenceDate];
        }
        NSTimeInterval time = [NSDate timeIntervalSinceReferenceDate];
        
        //  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0ul), ^{
        NSTimeInterval diffTime = time - self.startRecordTime;
        
        self.timeString = [NSString stringWithTimecode:diffTime];
        
        int frameCount = diffTime*250.0;
        
        __block BOOL append_ok = NO;
        int j = 0;
        // self.adaptor.assetWriterInput
        //        NSLog(@"%ld",self.videoWriter.status);
        if (self.videoWriterInput.readyForMoreMediaData)
        {
            //                    printf("appending %d attemp %d\n", frameCount, j);
            CMTime frameTime = CMTimeMake(frameCount,(int32_t) 250.0);
            
            
            //   dispatch_sync(self.assetWriterQueue, ^{
            //   [self.lock lock];
            //   while(!self.adaptor.assetWriterInput.readyForMoreMediaData) {}
            //CVPixelBufferPoolCreatePixelBuffer (NULL, self.adaptor.pixelBufferPool, &buffer);
            
            append_ok = [self.adaptor appendPixelBuffer:buffer withPresentationTime:frameTime];
            
            //   while(!self.adaptor.assetWriterInput.readyForMoreMediaData) {}
            
            //    [self.lock unlock];
            //   });
            
            
            //if(buffer)
            //  CVBufferRelease(buffer);
            //[NSThread sleepForTimeInterval:0.035];
        }
        else
        {
            printf("adaptor not ready %d, %d\n", frameCount, j);
            // [NSThread sleepForTimeInterval:0.1];
        }
        j++;
        //}
        if (!append_ok) {
            NSLog(@"%@",self.videoWriter.error);
            printf("error appending image %d times %d\n", frameCount, j);
        }
        
        // });
        
    }
}


-(void) prepareRecording {
    self.readyToRecord = NO;
    self.timeString = @"";
    NSError *error = nil;
    
    //                self.recordingIndex ++;
    //              [[NSUserDefaults standardUserDefaults] setInteger:self.recordingIndex forKey:@"recordingIndex"];
    
    NSString * path = [@"~/Movies/_cache.mov" stringByExpandingTildeInPath];
    NSFileManager * fileManager = [NSFileManager defaultManager];
    [fileManager removeItemAtPath:path error:&error];
    
    self.videoWriter = [[AVAssetWriter alloc] initWithURL:
                        [NSURL fileURLWithPath:path] fileType:AVFileTypeQuickTimeMovie
                                                    error:&error];
    NSParameterAssert(self.videoWriter);
    
    NSSize size = self.deviceItem.size;
    if(size.width == 0){
        size = NSMakeSize(720, 576);
    }
    
    NSDictionary *videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:
                                   AVVideoCodecH264, AVVideoCodecKey,
                                   [NSNumber numberWithInt:size.width], AVVideoWidthKey,
                                   [NSNumber numberWithInt:size.height], AVVideoHeightKey,
                                   nil];
    
    self.videoWriterInput = [AVAssetWriterInput
                             assetWriterInputWithMediaType:AVMediaTypeVideo
                             outputSettings:videoSettings];
    
    
    self.adaptor = [AVAssetWriterInputPixelBufferAdaptor
                    assetWriterInputPixelBufferAdaptorWithAssetWriterInput:self.videoWriterInput
                    sourcePixelBufferAttributes:@{(NSString*)kCVPixelBufferPixelFormatTypeKey:[NSNumber numberWithInt:kCVPixelFormatType_32ARGB]}];
    
    NSParameterAssert(self.videoWriterInput);
    NSParameterAssert([self.videoWriter canAddInput:self.videoWriterInput]);
    self.videoWriterInput.expectsMediaDataInRealTime = YES;
    self.adaptor.assetWriterInput.expectsMediaDataInRealTime = YES;
    [self.videoWriter addInput:self.videoWriterInput];
    
    //Start a session:
    if(![self.videoWriter startWriting]){
        NSLog(@"Could not start writing %@",self.videoWriter.error);
    }
    [self.videoWriter startSessionAtSourceTime:kCMTimeZero];
    
    dispatch_async(dispatch_queue_create("waiter", 0), ^{
        [NSThread sleepForTimeInterval:0.2];
        self.readyToRecord = YES;
        NSLog(@"Ready to record");
    });
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    
    if(context == LabelContext){
        for(VideoBankItem * item in self.videoBank.content){
            item.recordLabel = 0;
        }
        
        NSArray * items = self.videoBank.content;
        if(self.bankIndex < items.count){
            VideoBankItem * item = items[self.bankIndex];
            if(self.record){
                item.recordLabel = 2;
            } else {
                item.recordLabel = 1;
            }
            
        }
    }
    if(context == RecordContext){
        self.startRecordTime = nil;
        
        if(!self.record){
            [self willChangeValueForKey:@"recordings"];
            
            [self.videoWriterInput markAsFinished];
            [self.videoWriter finishWriting];
            
            NSArray * items = self.videoBank.content;
            if(self.bankIndex < items.count){
                VideoBankItem * item = items[self.bankIndex];
                NSString * path = item.path;
                
                NSError * error;
                [[NSFileManager defaultManager] removeItemAtPath:[path stringByExpandingTildeInPath] error:nil];
                [[NSFileManager defaultManager] moveItemAtPath:[@"~/Movies/_cache.mov" stringByExpandingTildeInPath] toPath:[path stringByExpandingTildeInPath] error:&error];
                if(error){
                    NSLog(@"Error moving file %@",error);
                }
                
                [item loadBankFromDrive];
                
            }
            
            self.readyToRecord = YES;
            [self didChangeValueForKey:@"recordings"];
            
            [self performSelector:@selector(prepareRecording) withObject:nil afterDelay:0.5];
            
            //   [self prepareRecording];
            
            
        }
    }
    
    if(context == DeviceIndexContext){
        if(self.deviceItem){
            self.deviceItem.delegate = nil;
        }
        
        if(self.deviceIndex < self.blackmagicItems.count && self.deviceIndex >= 0){
            self.deviceItem = self.blackmagicItems[self.deviceIndex];
            self.deviceItem.delegate = self;
        }
        
        
        [self prepareRecording];
    }
}

-(BOOL)canRecord{
    
    NSArray * items = self.videoBank.content;
    if(self.bankIndex < items.count){
        VideoBankItem * item = items[self.bankIndex];
        return !item.locked;
    }
    return NO;
}

+(NSSet *)keyPathsForValuesAffectingCanRecord{
    return [NSSet setWithObjects:@"bankIndex",@"readyToRecord",nil];
}

@end
