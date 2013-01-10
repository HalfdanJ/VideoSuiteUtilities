//
//  OutputWindow.m
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "OutputWindow.h"

@implementation OutputWindow
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
