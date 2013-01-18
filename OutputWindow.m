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

        
        for(CALayer * layer in [self.layer sublayers]){
            layer.filters = nil;
            layer.filters = self.filters;
        }
        
        self.imageViewer.filters = self.filters;
    
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
