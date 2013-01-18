//
//  MyAvPlayerLayer.m
//  SH
//
//  Created by Flyvende Grise on 1/18/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "MyAvPlayerLayer.h"

@interface MyAvPlayerLayer ()
@property CIFilter * baseTransformFilter;
@property CIFilter * currentTransformFilter;

@end

@implementation MyAvPlayerLayer
static void *CurrentItemContext = &CurrentItemContext;

+(AVPlayerLayer *)playerLayerWithPlayer:(AVPlayer *)player{
    MyAvPlayerLayer * newPlayer = [[MyAvPlayerLayer alloc] init];
    newPlayer.player = player;
    
    [newPlayer addObserver:newPlayer forKeyPath:@"player.currentItem" options:0 context:CurrentItemContext];
    
    newPlayer.baseTransformFilter = [CIFilter filterWithName:@"CIAffineTransform"];
    [newPlayer.baseTransformFilter setDefaults];

    newPlayer.currentTransformFilter = [CIFilter filterWithName:@"CIAffineTransform"];
    [newPlayer.currentTransformFilter setDefaults];
    
    
    AVAssetTrack * track = player.currentItem.asset.tracks[0];
    CGSize naturalSize = track.naturalSize;
    
    if(naturalSize.width == 720){
        
    } else {
        NSAffineTransform * transform = [NSAffineTransform transform];
        [transform scaleXBy:naturalSize.width/720.0 yBy:naturalSize.height/576.0];
        [newPlayer.baseTransformFilter setValue:transform forKey:@"inputTransform"];
    }
    
    [newPlayer calcCurrentFilter];

    
//    NSLog(@"pw %f",track.naturalSize.width);
    
    return  newPlayer;
}

-(void)dealloc{
    [self removeObserver:self forKeyPath:@"player.currentItem"];
}

-(void) calcCurrentFilter{
    NSLog(@"Calc");
    [CATransaction begin];
    [CATransaction setAnimationDuration:0];
    
    
    AVAssetTrack * track = self.player.currentItem.asset.tracks[0];
    CGSize naturalSize = track.naturalSize;
    
    [self.currentTransformFilter setDefaults];
    
    if(naturalSize.width == 720){
        
    } else {
        float aspect1 = 720.0/576.0;
        float aspect2 = naturalSize.width/naturalSize.height;
        
        float pw = self.preferredFrameSize.width;
        float ph = self.preferredFrameSize.height;
        
        NSAffineTransform * transform = [NSAffineTransform transform];
       // [transform translateXBy:1024*0.5 yBy:0];
        float scale = aspect2/aspect1;

        [transform translateXBy:-((1024*scale-1024)*0.5) yBy:0];
        [transform scaleXBy:720.0/naturalSize.width
                        yBy:576.0/naturalSize.height];
        
        
    //    [transform translateXBy:720*scale yBy:0];

        [transform scaleXBy:scale yBy:1];
       // [transform translateXBy:-((1024*scale-1024)*0.5) yBy:0];
        

        [self.currentTransformFilter setValue:transform forKey:@"inputTransform"];
    }

    
    self.filters = nil;

    [self setFilters:@[self.baseTransformFilter, self.currentTransformFilter]];
    [CATransaction commit];

}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (context == CurrentItemContext) {
        [self calcCurrentFilter];
    } else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

@end
