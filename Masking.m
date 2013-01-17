//
//  Masking.m
//  SH
//
//  Created by Flyvende Grise on 1/17/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "Masking.h"
#import <AVFoundation/AVFoundation.h>
#import "QLabController.h"

@interface Masking ()

@property CIFilter * maskFilter;
@property CIFilter * invertFilter;
@property NSString * path;
@end

@implementation Masking
static void *SelectedMaskContext = &SelectedMaskContext;


-(NSString*)name {
    return @"Masking";
}

- (id)init
{
    self = [super init];
    if (self) {
        self.maskFilter = [CIFilter filterWithName:@"CIMaskToAlpha"];
        self.invertFilter = [CIFilter filterWithName:@"CIColorInvert"];
        
        
        self.maskingLayer = [CALayer layer];
        [self.maskingLayer setAutoresizingMask: kCALayerWidthSizable | kCALayerHeightSizable];
        
        self.maskingLayer.opacity = 1.0;
        self.maskingLayer.contents = [NSImage imageNamed:@"qlab"];
        self.maskingLayer.filters = @[self.invertFilter, self.maskFilter, self.invertFilter];
        [self.maskingLayer bind:@"opacity" toObject:self withKeyPath:@"opacity" options:nil];
        self.maskingLayer.delegate = self;
        
        self.path = [@"~/Movies/Global Masks/" stringByExpandingTildeInPath];
        
        [self addObserver:self forKeyPath:@"selectedMask" options:0 context:SelectedMaskContext];
        
        [self loadFolder];
        
        int num = 50;
        [globalMidi addBindingTo:self path:@"opacity" channel:1 number:num++ rangeMin:0 rangeLength:1];
        [globalMidi addBindingTo:self path:@"selectedMask" channel:1 number:num++ rangeMin:0 rangeLength:127];

        
    }
    return self;
}

- (id <CAAction>)actionForLayer:(CALayer *)layer forKey:(NSString *)event {
    CABasicAnimation *ani = [CABasicAnimation animationWithKeyPath:event];
    ani.duration = 0.0;
    return ani;
    
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context == SelectedMaskContext){
        self.maskingLayer.contents = nil;
        for(CALayer * subLayer in [self.maskingLayer sublayers]){
            [subLayer removeFromSuperlayer];
        }
        
        if(self.selectedMask != 0){
            
            NSString * filePath = [self.masks objectAtIndex:self.selectedMask];
            if([filePath rangeOfString:@"- None -"].location == NSNotFound){
                NSString * fullpath = [NSString stringWithFormat:@"%@/%@",self.path,filePath];
                
                if([self pathIsImage:fullpath]){
                    self.maskingLayer.contents = [[NSImage alloc]initWithContentsOfFile:fullpath];
                }
                else if([self pathIsMovie:fullpath]){
                    AVPlayer * player = [AVPlayer playerWithURL:[NSURL fileURLWithPath:fullpath]];
                    [player play];
                    player.actionAtItemEnd = AVPlayerActionAtItemEndNone;
                    
                    [[NSNotificationCenter defaultCenter] addObserver:self
                                                             selector:@selector(playerItemDidReachEnd:)
                                                                 name:AVPlayerItemDidPlayToEndTimeNotification
                                                               object:[player currentItem]];
                    
                    AVPlayerLayer * layer = [AVPlayerLayer playerLayerWithPlayer:player];
                    layer.frame = self.maskingLayer.frame;
                    [layer setAutoresizingMask: kCALayerWidthSizable | kCALayerHeightSizable];
                    layer.videoGravity = AVLayerVideoGravityResize;
                    
                    
                    [self.maskingLayer addSublayer:layer];
                }
            }
        }
    }
}

- (void)playerItemDidReachEnd:(NSNotification *)notification {
    AVPlayerItem *p = [notification object];
    [p seekToTime:kCMTimeZero];
}



-(BOOL)pathIsImage:(NSString*)path{
    BOOL ret = NO;
    CFStringRef fileExtension = (__bridge CFStringRef) [path pathExtension];
    CFStringRef fileUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, fileExtension, NULL);
    
    if (UTTypeConformsTo(fileUTI, kUTTypeImage) )
    {
        ret = YES;
    }
    CFRelease(fileUTI);
    return ret;
}
-(BOOL)pathIsMovie:(NSString*)path{
    BOOL ret = NO;
    CFStringRef fileExtension = (__bridge CFStringRef) [path pathExtension];
    CFStringRef fileUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, fileExtension, NULL);
    
    if (UTTypeConformsTo(fileUTI, kUTTypeMovie) )
    {
        ret = YES;
    }
    CFRelease(fileUTI);
    return ret;
}


-(void)loadFolder{
    NSMutableArray * array = [NSMutableArray array];
    [array addObject:@"- None -"];
    for(int i=1;i<100;i++){
        [array addObject:[NSString stringWithFormat:@"%02i - None -", i]];
    }
    
    NSFileManager * fileManager = [NSFileManager defaultManager];
    NSArray * files = [fileManager contentsOfDirectoryAtURL:[NSURL fileURLWithPath:self.path] includingPropertiesForKeys:nil options:NSDirectoryEnumerationSkipsHiddenFiles error:nil];
    
    NSNumberFormatter *f = [[NSNumberFormatter alloc] init];
    [f setAllowsFloats: NO];
    
    
    for(NSString * file in files){
        NSNumber * number = [f numberFromString:[[file lastPathComponent]substringToIndex:2] ];
        if(number != nil && [number intValue] > 0 && [number intValue] < 100){
            
            if ([self pathIsImage:file] || [self pathIsMovie:file])
            {
                [array removeObjectAtIndex:[number intValue]];
                [array insertObject:[file lastPathComponent] atIndex:[number intValue]];
            }
        }
    }
    
    self.masks = [array copy];
}


-(NSArray *)filters{
    return @[];
}

-(void)qlab{
    NSArray * cues = @[
    @{QName : [NSString stringWithFormat:@"Selected Mask: %i",self.selectedMask], QPath: @"selectedMask"},
    @{QName : [NSString stringWithFormat:@"Opacity: %.2f",self.opacity], QPath: @"opacity"},
    ];
    
    NSString * title = [NSString stringWithFormat:@"Set Mask %i",self.selectedMask];
    
    [QLabController createCues:cues groupTitle:title sender:self];
    

}

@end
