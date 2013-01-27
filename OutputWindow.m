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
    NSRect frame = contentView.frame;
    frame.size.width /= 3;
    self.imageViewer1 = [[CoreImageViewer alloc] initWithFrame:frame];
    [self.imageViewer1 setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable|NSViewMinXMargin|NSViewMaxXMargin];
    [self.contentView addSubview:self.imageViewer1];
    
    NSView * view = [[NSView alloc] initWithFrame:frame];
    [view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [view setWantsLayer:YES];
    self.layer1 = view.layer;
    [self.contentView addSubview:view];
    
    
    
    
    frame.origin.x += frame.size.width;
    self.imageViewer2 = [[CoreImageViewer alloc] initWithFrame:frame];
    [self.imageViewer2 setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable|NSViewMinXMargin|NSViewMaxXMargin];
    [self.contentView addSubview:self.imageViewer2];
    
    NSView * view2 = [[NSView alloc] initWithFrame:frame];
    [view2 setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [view2 setWantsLayer:YES];
    self.layer2 = view2.layer;
    [self.contentView addSubview:view2];

    frame.origin.x += frame.size.width;
    self.imageViewer3 = [[CoreImageViewer alloc] initWithFrame:frame];
    [self.imageViewer3 setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable|NSViewMinXMargin|NSViewMaxXMargin];
    [self.contentView addSubview:self.imageViewer3];
    
    NSView * view3 = [[NSView alloc] initWithFrame:frame];
    [view3 setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [view3 setWantsLayer:YES];
    self.layer3 = view3.layer;
    [self.contentView addSubview:view3];
    
    [self addObserver:self forKeyPath:@"filters" options:0 context:FiltersContext];
    
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
}


-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithFloat:00.0f]
                     forKey:kCATransactionAnimationDuration];
    
    
    for(CALayer * layer in [self.layer1 sublayers]){
        layer.filters = nil;
        layer.filters = self.filters;
    }
    
    for(CALayer * layer in [self.layer2 sublayers]){
        layer.filters = nil;
        layer.filters = self.filters;
    }
    for(CALayer * layer in [self.layer3 sublayers]){
        layer.filters = nil;
        layer.filters = self.filters;
    }
    self.imageViewer1.filters = self.filters;
    self.imageViewer2.filters = self.filters;
    self.imageViewer3.filters = self.filters;
    
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
