//
//  VideoBankItem.m
//  SH
//
//  Created by Jonas Jongejan on 06/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "VideoBankItem.h"
#import "NSString+Timecode.h"

@implementation VideoBankItem

static void *VideoStatusContext = &VideoStatusContext;

/*- (id)init
{
    self = [super init];
    if (self) {
        
    }
    return self;
}*/

-(void)loadBankFromDrive{
    [self loadBankFromPath:@"~/Movies/VideoSuite/Bif.mp4" ];
}

-(void) loadBankFromPath:(NSString*)path{
    NSURL * url = [NSURL fileURLWithPath:[path stringByExpandingTildeInPath] isDirectory:NO];
    
    self.avPlayerItem = [AVPlayerItem playerItemWithURL:url];
    [self.avPlayerItem addObserver:self forKeyPath:@"status" options:0 context:VideoStatusContext];
    
    
    self.avPreviewPlayer = [AVPlayer playerWithPlayerItem:self.avPlayerItem];
    self.loaded = NO;

}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    
 
    if(context == VideoStatusContext){
        AVPlayerItem * item = object;
        
        self.loaded = NO;
        if(item.status == AVPlayerItemStatusFailed){
            NSLog(@"Failed loading avplayeritem %@",self.name);
        } else if(item.status == AVPlayerItemStatusReadyToPlay){
            self.loaded = YES;
            self.duration = (float) self.avPlayerItem.duration.value / self.avPlayerItem.duration.timescale;
        }
        
    }
}


-(NSString*)durationString{
    return [NSString stringWithTimecode:self.duration];
}

+(NSSet *)keyPathsForValuesAffectingDurationString{
    return [NSSet setWithObject:@"duration"];
}

@end
