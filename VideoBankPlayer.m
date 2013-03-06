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
#import "QLabController.h"
#import <Quartz/Quartz.h>
#import "MyAvPlayerLayer.h"

@interface VideoBankPlayer ()
@property BOOL pingPong;
@property NSMutableDictionary * playerData;

@end


@implementation VideoBankPlayer
static void *AVSPPlayerLayerReadyForDisplay = &AVSPPlayerLayerReadyForDisplay;
static void *AVPlayerRateContext = &AVPlayerRateContext;
static void *AvPlayerCurrentItemContext = &AvPlayerCurrentItemContext;
static void *LabelContext = &LabelContext;
static void *LastItemContext = &LastItemContext;

-(NSString*)name{
    return @"Standard Player";
}

-(id)initWithBank:(VideoBank*)bank{
    
    self = [self init];
    if (self) {
        self.videoBank = bank;
        
        [self addObserver:self forKeyPath:@"bankSelection" options:0 context:LabelContext];
        [self addObserver:self forKeyPath:@"numberOfBanksToPlay" options:0 context:LabelContext];
        [self addObserver:self forKeyPath:@"lastItem" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:LastItemContext];

        
        self.layer = [CALayer layer];
        [self.layer setAutoresizingMask: kCALayerWidthSizable | kCALayerHeightSizable];
        self.layer.hidden = YES;
        
        self.playing = NO;
        self.bankSelection = 0;
        self.loop = NO;
        self.numberOfBanksToPlay = 1;
        self.playbackRate = 1.0;
        
        [self.layer bind:@"opacity" toObject:self withKeyPath:@"opacity" options:nil];
        self.opacity = 1.0;
        
        self.stopWhenReady = -1;
        
        
        int num = 0;
        [globalMidi addBindingPitchTo:self path:@"bankSelection" channel:2 rangeMin:-8192 rangeLength:128*128];
        [globalMidi addBindingTo:self path:@"numberOfBanksToPlay" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"opacity" channel:1 number:num++ rangeMin:0 rangeLength:1];
        [globalMidi addBindingTo:self path:@"playing" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"loop" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"midi" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"playbackRate" channel:1 number:num++ rangeMin:0 rangeLength:(1.0/31.0)*127.0];
        
    }
    return self;
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context == MaskContext){
        VideoBankItem * bankItem = object;
        if(bankItem.mask){
            CALayer * mask = bankItem.maskLayer;
            [mask setFrame:self.layer.frame];
            [mask setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
            avPlayerLayer[self.pingPong].mask =mask;
        } else {
            avPlayerLayer[self.pingPong].mask = nil;
        }
        
        
        
    }
    if(context==LastItemContext){
        VideoBankItem * old = [change objectForKey:@"old"];
        VideoBankItem * new = [change objectForKey:@"new"];
        
        
        if([old isKindOfClass:[VideoBankItem class]]){
            [old removeObserver:self forKeyPath:@"maskLayer"];
        }
        if([new isKindOfClass:[VideoBankItem class]]){
            [new addObserver:self forKeyPath:@"maskLayer" options:0 context:MaskContext];
        }
    }
    //    if(context == OpacityContext){
    //        self.layer.opacity = self.opacity;
    //    }
    
    if(context == LabelContext){
        for(VideoBankItem * item in self.videoBank.content){
            item.standardPlayerLabel = 0;
        }
        
        int count =0;
        for(int i=self.bankSelection;i<self.bankSelection + self.numberOfBanksToPlay;i++){
            if([self.videoBank.content count] > i){
                VideoBankItem * bankItem = [self.videoBank content][i];
                bankItem.standardPlayerLabel = ++count;
                
            }
        }
    }

    if (context == AVSPPlayerLayerReadyForDisplay)
	{
        AVPlayerLayer * layer = object;
        AVPlayer * player = layer.player;
        
        if ([[change objectForKey:NSKeyValueChangeNewKey] boolValue] == YES)
		{
            [player play];
            player.rate = self.playbackRate;
            
            if(self.stopWhenReady != -1){
                NSLog(@"Stop because ready %i", self.stopWhenReady);
                [self stopItem:self.stopWhenReady];
                self.stopWhenReady = -1;
            }
            
		}
	}
    
    
    if(context== AvPlayerCurrentItemContext){
        [self newItemPlaying];
        /*
         NSLog(@"CurrentItemContext");
         NSLog(@"Out times %@",self.outTimes);
         if(self.outTimes.count > 0){
         if(avPlayer[self.pingPong].rate){
         NSLog(@"Rate");
         NSArray * times = @[self.outTimes[0]];
         [self.outTimes removeObjectAtIndex:0];
         [self.bankRefs removeObjectAtIndex:0];
         
         self.timeOutTimeObserverToken = [avPlayer[self.pingPong] addBoundaryTimeObserverForTimes:times queue:dispatch_get_current_queue() usingBlock:^{
         
         [avPlayer[self.pingPong] removeTimeObserver:self.timeOutTimeObserverToken];
         self.timeOutTimeObserverToken = nil;
         
         [avPlayer[self.pingPong] advanceToNextItem];
         
         }];
         }
         } else {
         NSLog(@"Stop playing");
         self.playing = NO;
         }*/
    }
}


-(NSDictionary*)getDataForCurrentItem{
    return  [self getDataForItem:avPlayer[self.pingPong].currentItem];
}
-(NSDictionary*)getDataForItem:(AVPlayerItem*)item{
    return [self.playerData objectForKey:item.asset];
}


static void *MaskContext = &MaskContext;

-(void) newItemPlaying{
    self.counter = self.counter + 1;
    NSLog(@"\n\nNew item playing %i",self.counter);
    
    NSDictionary * data = [self getDataForCurrentItem];
    if(data){
        
        
        
        VideoBankItem * bankItem = [data valueForKey:@"bankRef"];
        
        
        __weak AVQueuePlayer * thisPlayer = avPlayer[self.pingPong];
        __weak AVPlayerLayer * thisLayer = avPlayerLayer[self.pingPong];
        __weak AVPlayerLayer * otherLayer = avPlayerLayer[!self.pingPong];
        int pingPong = self.pingPong;
        
        if(bankItem.mask){
            CALayer * mask = bankItem.maskLayer;
            [mask setFrame:self.layer.frame];
            [mask setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
            
            thisLayer.mask = mask;
            //            otherLayer.mask = mask;
        } else {
            thisLayer.mask = nil;
        }
        
        thisLayer.opacity = 1.0;
        
        self.lastItem = bankItem;
        
        double eventTime =  bankItem.duration - 0.1;
        eventTime = MAX(0, eventTime);
        CMTime eventCMTime = CMTimeMakeWithSeconds(eventTime, 100);
        NSValue * value = [NSValue valueWithCMTime:eventCMTime];
        
        if(self.midi){
            midiSendObserverToken[pingPong] = [thisPlayer addBoundaryTimeObserverForTimes:@[value] queue:dispatch_get_current_queue() usingBlock:^{
                [globalMidi sendMidiChannel:1 number:1 value:self.bankSelection+self.counter-1];
                [thisPlayer removeTimeObserver:midiSendObserverToken[pingPong]];
                midiSendObserverToken[pingPong] = nil;
            }];
        }
        
        //Crossfade IN
        double crossfadeTimeIn = [[data valueForKey:@"crossfadeTimeIn"] doubleValue];
        if(crossfadeTimeIn == 0){
            thisLayer.opacity = 1.0;
        } else {
            thisLayer.opacity = 0.0;
            
            fadeInObserverToken[pingPong] = [thisPlayer addPeriodicTimeObserverForInterval:CMTimeMake(1, 25) queue:NULL usingBlock:^(CMTime time) {
                
                double p = CMTimeGetSeconds(time) / crossfadeTimeIn;
                thisLayer.opacity = p;
                avPlayer[pingPong].volume = p;
                
                if(p >= 1){
                    [thisPlayer removeTimeObserver:fadeInObserverToken[pingPong]];
                    fadeInObserverToken[pingPong] = nil;
                }
                
                NSLog(@"Fade up %f",p);
            }];
            
        }
        
        //Crossfade OUT
        double crossfadeTimeOut = [[data valueForKey:@"crossfadeTimeOut"] doubleValue];
        if(crossfadeTimeOut > 0){
            double eventTime =  bankItem.duration - crossfadeTimeOut;
            eventTime = MAX(0, eventTime);
            
            CMTime eventCMTime = CMTimeMakeWithSeconds(eventTime, 100);
            NSValue * value = [NSValue valueWithCMTime:eventCMTime];
            
            fadeOutEventObserverToken[pingPong] = [thisPlayer addBoundaryTimeObserverForTimes:@[value] queue:dispatch_get_current_queue() usingBlock:^{
                
                
                //Remove observers
                [thisPlayer removeTimeObserver: fadeOutEventObserverToken[pingPong]];
                fadeOutEventObserverToken[pingPong] = nil;
                
                if(fadeInObserverToken[!pingPong]){
                    [avPlayer[!pingPong] removeTimeObserver:fadeInObserverToken[!pingPong]];
                    fadeInObserverToken[!pingPong] = nil;
                }
                
                //Switch ping pong
                self.pingPong = !self.pingPong;
                
                fadeOutObserverToken[pingPong] = [thisPlayer addPeriodicTimeObserverForInterval:CMTimeMake(1, 25) queue:NULL usingBlock:^(CMTime time) {
                    double p = MAX(0,(CMTimeGetSeconds(time)-eventTime) / crossfadeTimeOut);
                    thisLayer.opacity = 1-p;
                    avPlayer[pingPong].volume = 1-p;
                    
                    //NSLog(@"Fade down %f",1-p);
                    
                    if(p == 0){
                        [thisPlayer removeTimeObserver:fadeOutObserverToken[pingPong]];
                        fadeOutObserverToken[pingPong] = nil;
                    }
                }];
                
                //Start new player
                if([data objectForKey:@"playerItems"] != nil){
                    NSLog(@"Start new player. Current player %@ ",avPlayer[self.pingPong].currentItem);
                    if(avPlayer[self.pingPong].currentItem){
                        [avPlayer[self.pingPong] pause];
                    }
                    
                    avPlayer[self.pingPong] = [AVQueuePlayer queuePlayerWithItems:[data objectForKey:@"playerItems"]];
                    avPlayer[self.pingPong].rate = self.playbackRate;
                    
                    //Clear observers
                    timeObserverToken[self.pingPong] = nil;
                    timeOutTimeObserverToken[self.pingPong] = nil;
                    fadeInObserverToken[self.pingPong] = nil;
                    fadeOutObserverToken[self.pingPong] = nil;
                    fadeOutEventObserverToken[self.pingPong] = nil;
                    
                    [avPlayer[!self.pingPong] removeObserver:self forKeyPath:@"currentItem"];
                    
                    //Start player
                    //[avPlayer[self.pingPong] play];
                    
                    [avPlayer[self.pingPong] addObserver:self forKeyPath:@"currentItem" options:0 context:AvPlayerCurrentItemContext];
                    
                    avPlayerLayer[self.pingPong].player = avPlayer[self.pingPong];
                    
                    
                    //                    if(timeObserverToken[self.pingPong] ){
                    //                        [avPlayer[self.pingPong] removeTimeObserver:timeObserverToken[self.pingPong]];
                    //                        avPlayer[self.pingPong] = nil;
                    //                    }
                    
                    
                    int pingPongNewplayer = self.pingPong;
                    
                    
                    timeObserverToken[self.pingPong] = [avPlayer[self.pingPong] addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:NULL usingBlock:^(CMTime time) {
                        
                        self.currentTimeString = [NSString stringWithTimecode:CMTimeGetSeconds(time)];
                        
                        if(avPlayer[self.pingPong].rate){
                            VideoBankItem * item = [[self getDataForItem:avPlayer[pingPongNewplayer].currentItem] valueForKey:@"bankRef"];
                            item.queued = NO;
                            item.playing = YES;
                            item.playHeadPosition = CMTimeGetSeconds(time)+[item.inTime doubleValue];
                        }
                    }];
                    
                    [self newItemPlaying];
                }
                //                [CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
                //avPlayerLayer[self.pingPong].opacity = 0.1;
                //              [CATransaction commit];
                
            }];
            
        }
        
        
    } else {
        self.playing = NO;
        if(self.loop){
            self.playing = YES;
        }
    }
}

-(void) preparePlayback{
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue
                     forKey:kCATransactionDisableActions];

    
    //Switch ping pong
    self.pingPong = !self.pingPong;
    int pingPong = self.pingPong;
    NSLog(@"Prepare with pingpong %i",self.pingPong);

    
    //Cleanup
    self.counter = 0;

/*    [avPlayerLayer[0] removeObserver:self forKeyPath:@"readyForDisplay"];
    [avPlayerLayer[1] removeObserver:self forKeyPath:@"readyForDisplay"];
    
    if(avPlayerLayer[!self.pingPong]){
        [avPlayerLayer[!self.pingPong] removeFromSuperlayer];
    }
    
    if(avPlayer[!self.pingPong]){
        
        [avPlayer[!self.pingPong] removeObserver:self forKeyPath:@"currentItem"];
        avPlayer[!self.pingPong] = nil;
        
    }
*/
    
    
    //Prepare items
    NSMutableArray * playerItems = [NSMutableArray array];
    NSArray * initialPlayerItems;
    self.playerData = [NSMutableDictionary dictionary];
    
    id lastKey = nil;
    id newPlayerKey = nil;
    
    
    //Fill item arrays
    for(int i=self.bankSelection;i<self.bankSelection + self.numberOfBanksToPlay;i++){
        if([self.videoBank.content count] > i){
            BOOL isLast = (i == self.bankSelection + self.numberOfBanksToPlay -1 )? YES : NO;

            VideoBankItem * bankItem = [self.videoBank content][i];
            AVAsset * asset = bankItem.avPlayerItemTrim.asset;
            
            if([asset isPlayable] && bankItem.loaded){
                AVPlayerItem *playerItem = [AVPlayerItem playerItemWithAsset:asset];
                
                //UI
                bankItem.queued = YES;
                
                //Crossfade times
                double crossfadeTimeIn = [bankItem.crossfadeTime doubleValue];
                double crossfadeTimeOut = 0;
                
                if(crossfadeTimeIn >  bankItem.duration){
                    crossfadeTimeIn = bankItem.duration-0.01;
                }
                
                if(crossfadeTimeIn > 0 && lastKey){
                    NSMutableDictionary * lastDict = [NSMutableDictionary dictionaryWithDictionary:[self.playerData objectForKey:lastKey] ];
                    
                    [lastDict setValue:@(crossfadeTimeIn) forKey:@"crossfadeTimeOut"];
                    
                    [self.playerData setObject:[NSDictionary dictionaryWithDictionary:lastDict] forKey:lastKey];
                }
                
                
                if((crossfadeTimeIn > 0 && lastKey)){
                    if(newPlayerKey == nil){
                        initialPlayerItems = playerItems;
                    } else {
                        NSMutableDictionary * lastDict = [NSMutableDictionary dictionaryWithDictionary:[self.playerData objectForKey:newPlayerKey] ];
                        [lastDict setValue:playerItems forKey:@"playerItems"];
                        [self.playerData setObject:[NSDictionary dictionaryWithDictionary:lastDict] forKey:newPlayerKey];
                    }
                    
                    playerItems = [NSMutableArray array];
                    newPlayerKey = lastKey;
                    
                }
                
                [playerItems addObject:playerItem];
                
                if(isLast){
                    if(newPlayerKey == nil){
                        initialPlayerItems = playerItems;
                    } else {
                        NSMutableDictionary * lastDict = [NSMutableDictionary dictionaryWithDictionary:[self.playerData objectForKey:newPlayerKey] ];
                        [lastDict setValue:playerItems forKey:@"playerItems"];
                        [self.playerData setObject:[NSDictionary dictionaryWithDictionary:lastDict] forKey:newPlayerKey];
                    }
                }
                
                
                NSDictionary * dict = @{
                @"bankRef" : bankItem,
                @"crossfadeTimeIn" : @(crossfadeTimeIn),
                @"crossfadeTimeOut" : @(crossfadeTimeOut),
                @"playerItems" : @[]
                };
                
                
                [self.playerData setObject:dict forKey:playerItem.asset];
                
                lastKey = playerItem.asset;
                
            }
        }
    } //Done preparing items

    
    //Nothing to play?
    if(initialPlayerItems.count == 0){
        dispatch_async(dispatch_get_main_queue(), ^{
            self.playing = NO;
        });
        return;
    }
    
    
    
    //Create AVPlayer
    avPlayer[pingPong] = [AVQueuePlayer queuePlayerWithItems:initialPlayerItems];
    
    //Layer
    avPlayerLayer[pingPong] = [MyAvPlayerLayer playerLayerWithPlayer:avPlayer[pingPong]];
    [avPlayerLayer[pingPong] setFrame:self.layer.frame];
    avPlayerLayer[pingPong].videoGravity = AVLayerVideoGravityResize;
    [avPlayerLayer[pingPong] setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [avPlayerLayer[pingPong] setHidden:NO];
    avPlayerLayer[pingPong].opacity = 0.0;
    
    [self.layer addSublayer:avPlayerLayer[pingPong]];
    
    
    
    [self newItemPlaying];
    //avPlayerLayer[0].opacity = 1.0;
    
    //Timecode updater
    //    timeObserverToken[0] = [avPlayer[0] addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:dispatch_get_main_queue() usingBlock:^(CMTime time) {
    timeObserverToken[self.pingPong] = [avPlayer[pingPong] addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:NULL usingBlock:^(CMTime time) {
        //  NSLog(@"Update1");
        //        dispatch_async(dispatch_get_main_queue(), ^{
        self.currentTimeString = [NSString stringWithTimecode:CMTimeGetSeconds(time)];
        
        if(avPlayer[pingPong].rate){
            VideoBankItem * item = [[self getDataForItem:avPlayer[pingPong].currentItem] valueForKey:@"bankRef"];
            item.queued = NO;
            item.playing = YES;
            item.playHeadPosition = CMTimeGetSeconds(time)+[item.inTime doubleValue];
        }
        //   });
    }];
    
    
    
    
    
    
    self.layer.hidden = !self.playing;

    
    //Observers
    [avPlayerLayer[pingPong] addObserver:self forKeyPath:@"readyForDisplay" options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew context:AVSPPlayerLayerReadyForDisplay];
    
    [avPlayer[self.pingPong] addObserver:self forKeyPath:@"currentItem" options:0 context:AvPlayerCurrentItemContext];
    
    
    [CATransaction commit];
    
}

-(void) clearBankStatus{
    for(VideoBankItem * item in self.videoBank.content){
        item.queued = NO;
        item.playHeadPosition = 0;
        item.playing = NO;
        
    }
}

-(void) stopItem:(int)i{
    NSLog(@"Stop %i",i);
    if(midiSendObserverToken[i]){
        [avPlayer[i] removeTimeObserver:midiSendObserverToken[i]];
        midiSendObserverToken[i] = nil;
    }
    if(timeObserverToken[i]){
        [avPlayer[i] removeTimeObserver:timeObserverToken[i]];
        timeObserverToken[i] = nil;
    }
    if(timeOutTimeObserverToken[i]){
        [avPlayer[i] removeTimeObserver:timeOutTimeObserverToken[i]];
        timeOutTimeObserverToken[i] = nil;
    }
    if(fadeInObserverToken[i]){
        [avPlayer[i] removeTimeObserver:fadeInObserverToken[i]];
        fadeInObserverToken[i] = nil;
    }
    if(fadeOutObserverToken[i]){
        [avPlayer[i] removeTimeObserver:fadeOutObserverToken[i]];
        fadeOutObserverToken[i] = nil;
    }
    if(fadeOutEventObserverToken[i]){
        [avPlayer[i] removeTimeObserver:fadeOutEventObserverToken[i]];
        fadeOutEventObserverToken[i] = nil;
    }
    
    
    
    
    [avPlayer[i] pause];
    
    
    
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
    [avPlayerLayer[i] removeFromSuperlayer];
    
    [CATransaction commit];
    
    
    if(avPlayerLayer[i] != nil){
        [avPlayerLayer[i] removeObserver:self forKeyPath:@"readyForDisplay"];
        avPlayerLayer[i] = nil;
    }
}

-(void) stop{
    NSLog(@"Stop");
    [self clearBankStatus];
    
    for(int i=0;i<2;i++){
        [self stopItem:i];
        
        
    }
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];

    self.layer.hidden = YES;

    [CATransaction commit];

    
}

-(void)setPlaying:(BOOL)playing{
    //if(_playing != playing){
    NSLog(@"Set playing %i",playing);
    
    if(_playing && playing){
        self.stopWhenReady = self.pingPong;
    } else {
        self.stopWhenReady = -1;
    }
    _playing = playing;
    
    
    
    if(playing){
        [self preparePlayback];
    } else {
        [self stop];
    }
    
    
    // }
}

-(BOOL)playing{
    return _playing;
}

-(void)qlabPlay{
    NSArray * cues = @[
    @{QName : [NSString stringWithFormat:@"Bank Selection: %02i",self.bankSelection], QPath: @"bankSelection"},
    @{QName : [NSString stringWithFormat:@"Banks to play: %i",self.numberOfBanksToPlay], QPath: @"numberOfBanksToPlay"},
    @{QName : [NSString stringWithFormat:@"Opacity: %.2f",self.opacity], QPath: @"opacity"},
    @{QName : [NSString stringWithFormat:@"Loop: %i",self.loop], QPath: @"loop"},
    @{QName : [NSString stringWithFormat:@"Midi: %i",self.midi], QPath: @"midi"},
    
    @{QName : [NSString stringWithFormat:@"Playback Rate: %.2f",self.playbackRate], QPath: @"playbackRate"},
    @{QName : [NSString stringWithFormat:@"Play: Yes"], QPath: @"playing", QValue: @(1)},
    ];
    
    NSString * title = [NSString stringWithFormat:@"Play bank %02i - %02i (Standard Player)",self.bankSelection,self.bankSelection+self.numberOfBanksToPlay-1];
    if(self.numberOfBanksToPlay == 1){
        title = [NSString stringWithFormat:@"Play bank %02i (Standard Player)",self.bankSelection];
    }
    
    
    [QLabController createCues:cues groupTitle:title sender:self];
}
-(void)qlabStop{
    NSArray * cues = @[
    @{QName : [NSString stringWithFormat:@"Play: No"], QPath: @"playing", QValue: @(0)},
    ];
    
    NSString * title = @"Stop (Standard Player)";
    [QLabController createCues:cues groupTitle:title sender:self];
    
}

@end
