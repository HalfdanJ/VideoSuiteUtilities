//
//  OutputWindow.m
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "OutputWindow.h"

@implementation OutputWindow

static void *FiltersContext = &FiltersContext;
static void *DefaultsContext = &DefaultsContext;

-(void)awakeFromNib{
    self.title = @"Output";
    
    [self setLevel: CGShieldingWindowLevel()];

    
    if([[NSScreen screens] count] > 1){
        [self setStyleMask:NSBorderlessWindowMask];

        NSScreen * screen = [NSScreen screens][1];
        
        NSRect screenRect = [screen frame];
        [self setFrame:screenRect display:YES];
    }
    
    
    NSView * contentView = self.contentView;
    self.imageViewer = [[CoreImageViewer alloc] initWithFrame:contentView.frame];
    [self.imageViewer setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [self.contentView addSubview:self.imageViewer];
    
    NSView * view = [[NSView alloc] initWithFrame:contentView.frame];
    [view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [view setWantsLayer:YES];
    self.layer = view.layer;
    [self.contentView addSubview:view];
    
    
    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:@"c1x" options:0 context:DefaultsContext];
    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:@"c2x" options:0 context:DefaultsContext];
    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:@"c3x" options:0 context:DefaultsContext];
    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:@"c4x" options:0 context:DefaultsContext];
    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:@"c1y" options:0 context:DefaultsContext];
    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:@"c2y" options:0 context:DefaultsContext];
    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:@"c3y" options:0 context:DefaultsContext];
    [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:@"c4y" options:0 context:DefaultsContext];

    
    
    [self addObserver:self forKeyPath:@"filters" options:0 context:FiltersContext];
    [self addObserver:self forKeyPath:@"layer.sublayers" options:0 context:FiltersContext];

    /*
    //Shortcuts
    [NSEvent addLocalMonitorForEventsMatchingMask:(NSKeyDownMask) handler:^(NSEvent *incomingEvent) {
		NSLog(@"Events: %@ %i",incomingEvent, [incomingEvent modifierFlags]);
        switch ([incomingEvent keyCode]) {
            case 3:
                if([incomingEvent modifierFlags] &  NSCommandKeyMask)
                    
                break;
            default:
                return incomingEvent;
                
                break;
        }
     
     
     
     return (NSEvent*)nil;
     }];*/
       
    self.transformFilter = [CIFilter filterWithName:@"CIAffineTransform"];
    [self.transformFilter setDefaults];
    
    self.movieColorFilter = [CIFilter filterWithName:@"CIColorMatrix"];
    [self.movieColorFilter setDefaults];
    
    CIVector * green = [CIVector vectorWithX:0 Y:0.93 Z:0 W:0];
    [self.movieColorFilter setValue:green forKey:@"inputGVector"];
    
    self.movieColorControls = [CIFilter filterWithName:@"CIColorControls"];
    [self.movieColorControls setDefaults];
    [self.movieColorControls setValue:@(0.92) forKey:@"inputContrast"];
  //  [self.movieColorControls setValue:@(0.98) forKey:@"inputSaturation"];
      [self.movieColorControls setValue:@(-0.045) forKey:@"inputBrightness"];
    [self observeValueForKeyPath:@"start" ofObject:nil change:nil context:nil];
    
}


-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithFloat:0.0f]
                     forKey:kCATransactionAnimationDuration];
    NSUserDefaults * defaults = [NSUserDefaults standardUserDefaults];
    
    
    
    float transformX = [defaults floatForKey:@"c1x"]*0.05;
    float transformY = [defaults floatForKey:@"c1y"]*0.05;
    float rotation = [defaults floatForKey:@"c2x"]*10-5;
    float scaleX = 1-[defaults floatForKey:@"c3x"]*0.05;
    float scaleY = 1-[defaults floatForKey:@"c3y"]*0.05;
    
    for(CALayer * layer in [self.layer sublayers]){
        layer.filters = nil;
        layer.transform = CATransform3DMakeAffineTransform(CGAffineTransformIdentity);
        
        layer.filters = [[self.filters arrayByAddingObject:self.movieColorFilter] arrayByAddingObject:self.movieColorControls];//[self.filters arrayByAddingObject:self.perspectiveFilterMovie];
        
        
        CATransform3D transform;
        
        
        CGAffineTransform affineTransform = CGAffineTransformIdentity;
        affineTransform = CGAffineTransformMakeTranslation(transformX*1024, transformY*768);
        
        affineTransform = CGAffineTransformTranslate(affineTransform, -1024/2, -768/2);
        affineTransform = CGAffineTransformRotate(affineTransform, rotation * (3.14/180));
        affineTransform = CGAffineTransformTranslate(affineTransform, 1024/2, 768/2);
        
        affineTransform = CGAffineTransformTranslate(affineTransform, -1024/2, -768/2);
        affineTransform = CGAffineTransformScale(affineTransform, scaleX, scaleY);
        affineTransform = CGAffineTransformTranslate(affineTransform, 1024/2, 768/2);
        
        transform = CATransform3DMakeAffineTransform(affineTransform);
        
        
        layer.sublayerTransform = transform;
    }
    
   
    NSAffineTransform * transform = [NSAffineTransform transform];
    [transform translateXBy:transformX*720 yBy:transformY*576];
    [transform rotateByDegrees:rotation];
    
    [transform scaleXBy:scaleX yBy:scaleY];
    
    [self.transformFilter setValue:transform forKey:@"inputTransform"];
    
    self.imageViewer.filters = [self.filters arrayByAddingObject:self.transformFilter];
    
    [CATransaction commit];
    
}

/*
 -(void)setFilters:(NSArray *)filters{
 for(CALayer * layer in [self.layer sublayers]){
 layer.filters = nil;
 layer.filters = filters;
 }
 
 self.imageViewer.filters = filters;
 }
 
 -(CIFilter *)filters{
 return nil;
 }*/

/*
 -(void) toggleFullScreen:(id)sender{
 
 }
 
 -(void) setFullscreen:(BOOL)fullscreen{
 if(_fullscreen != fullscreen){
 _fullscreen = fullscreen;
 if(fullscreen){
 if([[NSScreen screens] count] > 1){
 NSScreen * screen = [NSScreen screens][1];
 NSRect screenRect = [screen frame];
 [self setLevel: CGShieldingWindowLevel()];
 [self setFrame:screenRect display:YES];
 }         } else {
 //            [self setLevel:<#(NSInteger)#>]
 }
 
 }
 }
 
 -(BOOL)fullscreen{
 return _fullscreen;
 }*/
@end
