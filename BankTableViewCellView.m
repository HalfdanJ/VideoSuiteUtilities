//
//  BankTableViewCellView.m
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "BankTableViewCellView.h"
#import "VideoBankItem.h"

@implementation BankTableViewCellView
static void *RedrawContext = &RedrawContext;

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        [self addObserver:self forKeyPath:@"objectValue" options:0 context:nil];
        [self addObserver:self forKeyPath:@"objectValue.inTime" options:0 context:RedrawContext];
        [self addObserver:self forKeyPath:@"objectValue.durationOriginal" options:0 context:RedrawContext];
        [self addObserver:self forKeyPath:@"objectValue.duration" options:0 context:RedrawContext];
        [self addObserver:self forKeyPath:@"objectValue.outTime" options:0 context:RedrawContext];
        [self addObserver:self forKeyPath:@"objectValue.playing" options:0 context:RedrawContext];
        [self addObserver:self forKeyPath:@"objectValue.queued" options:0 context:RedrawContext];
        [self addObserver:self forKeyPath:@"objectValue.playHeadPosition" options:0 context:RedrawContext];

    }
    return self;
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    
    if(context == RedrawContext){
        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect rect = self.frame;
    rect.size.height -= 0;
    rect.origin.y += 0;
    
    rect.size.width -= 0;
    
    [[NSColor colorWithDeviceWhite:0.3 alpha:1.0] set];
    NSRectFill(rect);
    

    VideoBankItem * item = self.objectValue;
    
    
    NSRect timeRange = rect;
    
    double duration = item.durationOriginal;
    double inTime = 0;
    double outTime = 0;
    
    if(item.inTime){
        inTime = [item.inTime doubleValue] / duration;
        
        timeRange.origin.x += inTime * rect.size.width;
        timeRange.size.width -= inTime * rect.size.width;
    }
    
    if(item.outTime){
        outTime = (duration- [item.outTime doubleValue]) / duration;
        
        timeRange.size.width -= outTime * rect.size.width;
    }
    
    
    if(item.queued){
        [[NSColor colorWithDeviceRed:0.4 green:0.4 blue:0.2 alpha:1.0] set];

    } else {
        [[NSColor colorWithDeviceWhite:0.4 alpha:1.0] set];
    }
    NSRectFill(timeRange);
    
    
    if(item.playing){
        [[NSColor colorWithDeviceRed:0.2 green:0.4 blue:0.2 alpha:1.0] set];
        
        double playhead = item.playHeadPosition  / duration;
        
        NSRect playingRect = rect;
	
        playingRect.origin.x += MAX(inTime, playhead) * rect.size.width;
        playingRect.size.width -= MAX(inTime, playhead) * rect.size.width;
        
        playingRect.size.width -= outTime* rect.size.width;

        NSRectFill(playingRect);
    } 
}

@end
