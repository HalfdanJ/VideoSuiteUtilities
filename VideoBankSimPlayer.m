//
//  VideoBankSimPlayer.m
//  SH
//
//  Created by Flyvende Grise on 1/14/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "VideoBankSimPlayer.h"
#import "NSString+Timecode.h"
#import "QLabController.h"

@interface VideoBankSimPlayer ()

@property NSArray * avPlayers;
@property NSArray * avPlayerLayers;
@property NSMutableArray * timeObserverToken;
@property NSMutableDictionary * playerData;

@end




@implementation VideoBankSimPlayer
static void *PlayingContext = &PlayingContext;
static void *LabelContext = &LabelContext;

-(NSString*)name{
    return @"Composite Player";

}

-(id)initWithBank:(VideoBank*)bank{
    self = [self init];
    if (self) {
        self.videoBank = bank;
        
        [self addObserver:self forKeyPath:@"bankSelection" options:0 context:LabelContext];
        [self addObserver:self forKeyPath:@"numberOfBanksToPlay" options:0 context:LabelContext];
        [self addObserver:self forKeyPath:@"mask" options:0 context:LabelContext];

        self.layer = [CALayer layer];
        [self.layer setAutoresizingMask: kCALayerWidthSizable | kCALayerHeightSizable];
        self.layer.hidden = YES;
        
        self.playing = NO;
        self.bankSelection = 0;
        self.numberOfBanksToPlay = 2;
        
        [self.layer bind:@"opacity" toObject:self withKeyPath:@"opacity" options:nil];
        self.opacity = 1.0;
        
        [self addObserver:self forKeyPath:@"playing" options:0 context:PlayingContext];
        
        int num = 10;
        [globalMidi addBindingTo:self path:@"bankSelection" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"numberOfBanksToPlay" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"opacity" channel:1 number:num++ rangeMin:0 rangeLength:1];
        [globalMidi addBindingTo:self path:@"mask" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"playing" channel:1 number:num++ rangeMin:0 rangeLength:127];
    }
    return self;
}

-(CALayer*) loadMask:(int)num{
    NSString * path = [[NSString stringWithFormat:@"~/Movies/Compositing Masks/Mask %02i.png", num+1] stringByExpandingTildeInPath];

    NSImage * image = [[NSImage alloc] initWithContentsOfFile:path];
    
    if(!image)
        return  nil;
    
    CALayer * layer = [CALayer layer];
    layer.contents = image;
    
    return layer;
}

-(NSDictionary*)getDataForItem:(AVPlayerItem*)item{
    return [self.playerData objectForKey:item.asset];
}


-(void) preparePlayback{
    self.timeObserverToken = [NSMutableArray array];
    self.playerData = [NSMutableDictionary dictionary];

    
    NSMutableArray * players = [NSMutableArray arrayWithCapacity:self.numberOfBanksToPlay];
    NSMutableArray * layers = [NSMutableArray arrayWithCapacity:self.numberOfBanksToPlay];
    
    int count = 0;
    for(int i=self.bankSelection;i<self.bankSelection + self.numberOfBanksToPlay;i++){
        if([self.videoBank.content count] > i){
            VideoBankItem * bankItem = [self.videoBank content][i];
            
            AVAsset * asset = bankItem.avPlayerItemTrim.asset;
            
            if([asset isPlayable] && bankItem.loaded){

                
                AVPlayerItem *playerItem = [AVPlayerItem playerItemWithAsset:asset];
                AVPlayer * newPlayer = [AVPlayer playerWithPlayerItem:playerItem];
                
        
                [[NSNotificationCenter defaultCenter]
                 addObserver:self
                 selector:@selector(playerItemDidReachEnd:)
                 name:AVPlayerItemDidPlayToEndTimeNotification
                 object:[newPlayer currentItem]];
                
                
                double delayInSeconds = 0.1;
                dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
                dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                  
                    id newToken = [newPlayer addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:NULL usingBlock:^(CMTime time) {
                        self.currentTimeString = [NSString stringWithTimecode:CMTimeGetSeconds(time)];
                        
                        if(newPlayer.rate){
                            VideoBankItem * item = [[self getDataForItem:newPlayer.currentItem] valueForKey:@"bankRef"];
                            item.queued = NO;
                            item.playing = YES;
                            item.playHeadPosition = CMTimeGetSeconds(time)+[item.inTime doubleValue];
                        }
                    }];
                    
                    [self.timeObserverToken addObject:newToken];
                });
                
                [newPlayer play];

                [players addObject:newPlayer];
                
                NSDictionary * dict = @{
                @"bankRef" : bankItem,
                };
                
                
                [self.playerData setObject:dict forKey:playerItem.asset];

                
                
                
                CALayer * mask = [self loadMask:count+self.mask];
                [mask setFrame:self.layer.frame];
                [mask setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
                
                
                AVPlayerLayer *newPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:newPlayer];
                [newPlayerLayer setFrame:self.layer.frame];
                newPlayerLayer.videoGravity = AVLayerVideoGravityResize;
                [newPlayerLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
                [newPlayerLayer setHidden:NO];
                newPlayerLayer.opacity = 1.0;
                newPlayerLayer.mask = mask;
                
                [layers addObject:newPlayerLayer];
                [self.layer addSublayer:newPlayerLayer];
                

            }
        }
        count++;
    }
    
    
    int i=0;
    for(AVPlayer * player in players){
               i++;
    }
    
    self.avPlayerLayers = [NSArray arrayWithArray:layers];
    self.avPlayers = [NSArray arrayWithArray:players];
    
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue
                     forKey:kCATransactionDisableActions];
    self.layer.hidden = NO;
    [CATransaction commit];
}

- (void)playerItemDidReachEnd:(NSNotification *)notification {
    self.playing = NO;
}



-(void) clearBankStatus{
    for(VideoBankItem * item in self.videoBank.content){
        item.queued = NO;
        item.playHeadPosition = 0;
        item.playing = NO;
    }
}


-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context == LabelContext){
        for(VideoBankItem * item in self.videoBank.content){
            item.compositePlayerLabel = -1;
        }
        
        int count =0;
        for(int i=self.bankSelection;i<self.bankSelection + self.numberOfBanksToPlay;i++){
            if([self.videoBank.content count] > i){
                VideoBankItem * bankItem = [self.videoBank content][i];
                bankItem.compositePlayerLabel = self.mask+count;
                
                count++;
            }
        }
    }

    if(context == PlayingContext){
        if(self.playing){
            [self preparePlayback];
        } else {
            [self clearBankStatus];

            [CATransaction begin];
            [CATransaction setValue:(id)kCFBooleanTrue
                             forKey:kCATransactionDisableActions];
            self.layer.hidden = YES;
            [CATransaction commit];
            
            
            for(AVPlayer * player in self.avPlayers){
                [player pause];
            }
            for(AVPlayerLayer * layer in self.avPlayerLayers){
                [layer removeFromSuperlayer];
            }

            
            int i=0;
            for(id t in self.timeObserverToken){
                [self.avPlayers[i] removeTimeObserver:t];
                i++;
            }
            
            
            self.avPlayers = nil;
            self.avPlayerLayers = nil;
            
            self.timeObserverToken = nil;


        }
    }
}

-(void)qlabPlay{
    NSArray * cues = @[
    @{QName : [NSString stringWithFormat:@"Bank Selection: %02i",self.bankSelection], QPath: @"bankSelection"},
    @{QName : [NSString stringWithFormat:@"Banks to play: %i",self.numberOfBanksToPlay], QPath: @"numberOfBanksToPlay"},
    @{QName : [NSString stringWithFormat:@"Opacity: %.2f",self.opacity], QPath: @"opacity"},
    @{QName : [NSString stringWithFormat:@"Mask: %i",self.mask], QPath: @"mask"},
    @{QName : [NSString stringWithFormat:@"Play: Yes"], QPath: @"playing", QValue: @(1)},
    ];
    
    NSString * title = [NSString stringWithFormat:@"Play bank %02i - %02i (Composite Player)",self.bankSelection,self.bankSelection+self.numberOfBanksToPlay-1];
    if(self.numberOfBanksToPlay == 1){
        title = [NSString stringWithFormat:@"Play bank %02i (Composite Player)",self.bankSelection];
    }
    
    [QLabController createCues:cues groupTitle:title sender:self];
}
-(void)qlabStop{
    NSArray * cues = @[
    @{QName : [NSString stringWithFormat:@"Play: No"], QPath: @"playing", QValue: @(0)},
    ];
    
    NSString * title = @"Stop (Composite Player)";
    [QLabController createCues:cues groupTitle:title sender:self];
    
}
@end
