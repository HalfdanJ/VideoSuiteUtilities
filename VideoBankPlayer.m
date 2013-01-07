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

@property AVQueuePlayer * avQueuePlayer;
@property AVPlayerLayer * avPlayerLayer;
@property id timeObserverToken;

@end


@implementation VideoBankPlayer
static void *AVSPPlayerLayerReadyForDisplay = &AVSPPlayerLayerReadyForDisplay;

- (id)init
{
    self = [super init];
    if (self) {
        self.layer = [CALayer layer];
        [self.layer setAutoresizingMask: kCALayerWidthSizable | kCALayerHeightSizable];
//        self.layer.backgroundColor = [[NSColor redColor] CGColor];
        self.layer.hidden = YES;
        
        self.playing = NO;
        self.bankSelection = 0;
        self.numberOfBanksToPlay = 2;
        self.simultaneousPlayback = NO;
        
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
            
            [self.avQueuePlayer play];
		}
	}}

-(void) preparePlayback{
    [self.avQueuePlayer removeTimeObserver:self.timeObserverToken];
    if(self.avPlayerLayer){
        [self.avPlayerLayer removeFromSuperlayer];
    }
    
    
    NSMutableArray * playerItems = [NSMutableArray array];
    for(int i=self.bankSelection;i<self.bankSelection + self.numberOfBanksToPlay;i++){
        if([self.videoBank.content count] > i){
            VideoBankItem * bankItem = [self.videoBank content][i];
            AVAsset * asset = bankItem.avPlayerItem.asset;
            
            if([asset isPlayable]){
                AVPlayerItem *playerItem = [AVPlayerItem playerItemWithAsset:asset];
                
                [playerItems addObject:playerItem];
            }
        }
    }
    
    
    // Create a new AVPlayerItem and make it our player's current item.
    
    
    //[self.avPlayer replaceCurrentItemWithPlayerItem:playerItem];
    self.avQueuePlayer = [AVQueuePlayer queuePlayerWithItems:playerItems];
    
    [self setTimeObserverToken:[self.avQueuePlayer addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:dispatch_get_main_queue() usingBlock:^(CMTime time) {
        
        //        [self.timeSlider setDoubleValue:CMTimeGetSeconds(time)];
        //        self.timeTextField.stringValue = [NSString stringWithTimecode:CMTimeGetSeconds(time)];
        self.currentTimeString = [NSString stringWithTimecode:CMTimeGetSeconds(time)];
    }]];
    
    

    AVPlayerLayer *newPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:self.avQueuePlayer];
    [newPlayerLayer setFrame:self.layer.frame];
    newPlayerLayer.videoGravity = AVLayerVideoGravityResizeAspect;
    [newPlayerLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [newPlayerLayer setHidden:NO];
    
    self.avPlayerLayer = newPlayerLayer;

    [self.layer addSublayer:self.avPlayerLayer];
    
    
    [self addObserver:self forKeyPath:@"avPlayerLayer.readyForDisplay" options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew context:AVSPPlayerLayerReadyForDisplay];
    

}

-(void)setPlaying:(BOOL)playing{
    if(_playing != playing){
        _playing = playing;
        
        if(playing){
            [self preparePlayback];
        }else {
            [self.avQueuePlayer pause];
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
