//
//  VideoBankPlayer.m
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "VideoBankPlayer.h"
#import <AVFoundation/AVFoundation.h>
#import "NSString+Timecode.h"

@interface VideoBankPlayer ()

@property AVQueuePlayer * avPlayer;
@property AVPlayerLayer * avPlayerLayer;
@property id timeObserverToken;


@property id timeOutTimeObserverToken;

@property NSMutableArray * outTimes;

@end


@implementation VideoBankPlayer
static void *AVSPPlayerLayerReadyForDisplay = &AVSPPlayerLayerReadyForDisplay;
static void *AVPlayerRateContext = &AVPlayerRateContext;
static void *AvPlayerCurrentItemContext = &AvPlayerCurrentItemContext;
- (id)init
{
    self = [super init];
    if (self) {
        self.layer = [CALayer layer];
        [self.layer setAutoresizingMask: kCALayerWidthSizable | kCALayerHeightSizable];
        self.layer.hidden = YES;
        
        self.playing = NO;
        self.bankSelection = 0;
        self.numberOfBanksToPlay = 1;
        //self.simultaneousPlayback = NO;
        
    }
    return self;
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if (context == AVSPPlayerLayerReadyForDisplay)
	{
		if ([[change objectForKey:NSKeyValueChangeNewKey] boolValue] == YES)
		{
			// The AVPlayerLayer is ready for display.
            [CATransaction begin];
            [CATransaction setValue:(id)kCFBooleanTrue
                             forKey:kCATransactionDisableActions];
            self.layer.hidden = !self.playing;
            [CATransaction commit];
            
            [self.avPlayer play];
		}
	}
    
    if(context== AvPlayerCurrentItemContext){
        if(self.outTimes.count > 0){
            NSArray * times = @[self.outTimes[0]];
            [self.outTimes removeObjectAtIndex:0];
            
            self.timeOutTimeObserverToken = [self.avPlayer addBoundaryTimeObserverForTimes:times queue:dispatch_get_current_queue() usingBlock:^{
                
                [self.avPlayer removeTimeObserver:self.timeOutTimeObserverToken];
                self.timeOutTimeObserverToken = nil;
                
                [self.avPlayer advanceToNextItem];
                
            }];
        } else {
            self.playing = NO;
        }
    }
}

-(void) preparePlayback{

    //Cleanup
    [self.avPlayer removeTimeObserver:self.timeOutTimeObserverToken];
    self.timeOutTimeObserverToken = nil;
    
    [self.avPlayer removeTimeObserver:self.timeObserverToken];
    self.timeObserverToken = nil;
    
    if(self.avPlayerLayer){
        [self.avPlayerLayer removeFromSuperlayer];
    }
    
    
    //Prepare items
    NSMutableArray * playerItems = [NSMutableArray array];
    self.outTimes = [NSMutableArray array];
    for(int i=self.bankSelection;i<self.bankSelection + self.numberOfBanksToPlay;i++){
        if([self.videoBank.content count] > i){
            VideoBankItem * bankItem = [self.videoBank content][i];
            AVAsset * asset = bankItem.avPlayerItem.asset;
            
            if([asset isPlayable]){
                AVPlayerItem *playerItem = [AVPlayerItem playerItemWithAsset:asset];
                
                CMTime inTime = CMTimeMakeWithSeconds([bankItem.inTime doubleValue], 100);
                [playerItem seekToTime:inTime toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
                
                
                [playerItems addObject:playerItem];
                
                if(bankItem.outTime != nil){
                    [self.outTimes addObject:[NSValue valueWithCMTime:CMTimeMakeWithSeconds([bankItem.outTime doubleValue], 100)]];
                } else {
                    [self.outTimes addObject:[NSValue valueWithCMTime:CMTimeMakeWithSeconds(-1, 100)] ];
                    
                }
            }
        }
    }
    
    //Create AVPlayer
    self.avPlayer = [AVQueuePlayer queuePlayerWithItems:playerItems];
    
    //Layer
    AVPlayerLayer *newPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:self.avPlayer];
    [newPlayerLayer setFrame:self.layer.frame];
    newPlayerLayer.videoGravity = AVLayerVideoGravityResizeAspect;
    [newPlayerLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [newPlayerLayer setHidden:NO];
    
    self.avPlayerLayer = newPlayerLayer;
    [self.layer addSublayer:self.avPlayerLayer];
    
    
    //Out times
    NSArray * times = @[self.outTimes[0]];
    [self.outTimes removeObjectAtIndex:0];
    
    self.timeOutTimeObserverToken = [self.avPlayer addBoundaryTimeObserverForTimes:times queue:dispatch_get_current_queue() usingBlock:^{
        [self.avPlayer removeTimeObserver:self.timeOutTimeObserverToken];
        self.timeOutTimeObserverToken = nil;
        [self.avPlayer advanceToNextItem];
    }];
    
    
    //Timecode updater
    self.timeObserverToken = [self.avPlayer addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:dispatch_get_main_queue() usingBlock:^(CMTime time) {
        self.currentTimeString = [NSString stringWithTimecode:CMTimeGetSeconds(time)];
    }];
    
    
    //Observers
    [self addObserver:self forKeyPath:@"avPlayerLayer.readyForDisplay" options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew context:AVSPPlayerLayerReadyForDisplay];
    
    [self addObserver:self forKeyPath:@"avPlayer.currentItem" options:0 context:AvPlayerCurrentItemContext];
    
}

-(void)setPlaying:(BOOL)playing{
    if(_playing != playing){
        _playing = playing;
        
        if(playing){
            [self preparePlayback];
        } else {
            [self.avPlayer pause];
            [CATransaction begin];
            [CATransaction setValue:(id)kCFBooleanTrue
                             forKey:kCATransactionDisableActions];
            self.layer.hidden = YES;
            [CATransaction commit];
            
        }
    }
}

-(BOOL)playing{
    return _playing;
}

@end
