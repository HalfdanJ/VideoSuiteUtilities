//
//  VideoBankItem.m
//  SH
//
//  Created by Jonas Jongejan on 06/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "VideoBankItem.h"
#import "NSString+Timecode.h"

@interface VideoBankItem ()

@property AVPlayer * avPreviewPlayer;


@end

@implementation VideoBankItem

static void *VideoStatusContext = &VideoStatusContext;
static void *TrimContext = &TrimContext;
static void *AssetContext = &AssetContext;

- (id)init
{
    self = [super init];
    if (self) {
        [self addObserver:self forKeyPath:@"inTime" options:0 context:TrimContext];
        [self addObserver:self forKeyPath:@"outTime" options:0 context:TrimContext];
        [self addObserver:self forKeyPath:@"avPlayerItemTrim.asset" options:0 context:AssetContext];
    }
    return self;
}

-(void)loadBankFromDrive{
    [self loadBankFromPath:self.path ];
    NSLog(@"Load %@",self.path);
}

-(void) loadBankFromPath:(NSString*)path{
    NSURL * url = [NSURL fileURLWithPath:[path stringByExpandingTildeInPath] isDirectory:NO];
    
    self.avPlayerItemOriginal = [AVPlayerItem playerItemWithURL:url];
    [self.avPlayerItemOriginal addObserver:self forKeyPath:@"status" options:0 context:VideoStatusContext];
    
    
    
    self.avPreviewPlayer = [AVPlayer playerWithPlayerItem:self.avPlayerItemOriginal];
    self.loaded = NO;
    
}

-(void) updateTrimmedVersion {
    
    if([self.inTime doubleValue] > 0 || self.outTime){
        AVMutableComposition * composition = [AVMutableComposition composition];
        
        CMTime start = CMTimeMakeWithSeconds(MIN([self.inTime doubleValue], self.durationOriginal), 100);
        CMTime end = CMTimeMakeWithSeconds([self.outTime doubleValue], 100);
        if(!self.outTime)
            end = CMTimeMakeWithSeconds(self.durationOriginal, 100);
        
        NSError * error;
        [composition insertTimeRange:CMTimeRangeFromTimeToTime(start, end) ofAsset:self.avPlayerItemOriginal.asset atTime:CMTimeMakeWithSeconds(0, 100) error:&error];
        
        if(error){
            NSLog(@"Error creating composition");
        }
        
        AVComposition * imComposition = [composition copy];
        
        self.avPlayerItemTrim = [AVPlayerItem playerItemWithAsset:imComposition];
        
    } else {
        self.avPlayerItemTrim = self.avPlayerItemOriginal;
    }
    
    self.loaded = YES;
    self.duration = (float) self.avPlayerItemTrim.duration.value / self.avPlayerItemTrim.duration.timescale;
    self.durationOriginal = (float) self.avPlayerItemOriginal.duration.value / self.avPlayerItemOriginal.duration.timescale;
    
    
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context == AssetContext){
        NSLog(@"New asset");
        
        if ([[self.avPlayerItemTrim.asset tracksWithMediaType:AVMediaTypeVideo] count] > 0) {
            AVAssetImageGenerator *imageGenerator = [AVAssetImageGenerator assetImageGeneratorWithAsset:self.avPlayerItemTrim.asset];
            imageGenerator.maximumSize = CGSizeMake(48, 48);
            NSError * error;
            CGImageRef image = [imageGenerator copyCGImageAtTime:CMTimeMakeWithSeconds(0, 100) actualTime:nil error:&error];
            if(error){
                NSLog(@"Could not create thumbnail");
            } else {
                self.thumbnail = [[NSImage alloc] initWithCGImage:image size: NSZeroSize];
            }
        }
    }
    if(context == TrimContext){
        [self updateTrimmedVersion];
    }
 
    if(context == VideoStatusContext){
        AVPlayerItem * item = object;
        
        self.loaded = NO;
        if(item.status == AVPlayerItemStatusFailed){
            NSLog(@"Failed loading avplayeritem %@",self.name);
        } else if(item.status == AVPlayerItemStatusReadyToPlay){
            [self updateTrimmedVersion];
        }
        
    }
}


-(NSString*)durationString{
    return [NSString stringWithTimecode:self.duration];
}

+(NSSet *)keyPathsForValuesAffectingDurationString{
    return [NSSet setWithObject:@"duration"];
}

-(NSString*)durationOriginalString{
    return [NSString stringWithTimecode:self.durationOriginal];
}

+(NSSet *)keyPathsForValuesAffectingDurationOriginalString{
    return [NSSet setWithObject:@"durationOriginal"];
}


-(NSString*)inTimeString{
    return [NSString stringWithTimecode:[self.inTime doubleValue]];
}

+(NSSet *)keyPathsForValuesAffectingInTimeString{
    return [NSSet setWithObject:@"inTime"];
}


-(NSString*)outTimeString{
    return [NSString stringWithTimecode:[self.outTime doubleValue]];
}

+(NSSet *)keyPathsForValuesAffectingOutTimeString{
    return [NSSet setWithObject:@"outTime"];
}

-(NSString *)path{
    if(_manualPath){
        return _manualPath;
    } else {
        return [[NSString stringWithFormat:@"~/Movies/%@.mov",self.name] stringByExpandingTildeInPath];
    }
}

-(void)setPath:(NSString *)path{
    _manualPath = path;
}

@end
