//
//  CoreImageViewer.m
//  ViljensTriumf
//
//  Created by Jonas on 10/9/12.
//
//

#import "CoreImageViewer.h"

@implementation CoreImageViewer

static void *CIImageContext = &CIImageContext;

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        self.ciImage = nil;
        [self addObserver:self forKeyPath:@"ciImage" options:0 context:CIImageContext];
    }
    
    return self;
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context == CIImageContext){
        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    if(!self.ciImage){
        NSLog(@"No ciimage");
        [[NSColor blackColor] set];
        NSRectFill(dirtyRect);
    } else {
    @autoreleasepool {
        if(ciContext == nil){
            ciContext = [CIContext contextWithCGContext:
                         [[NSGraphicsContext currentContext] graphicsPort] options: nil];
        }
        
        [ciContext drawImage:self.ciImage inRect:NSRectToCGRect(dirtyRect) fromRect:NSMakeRect(0, 0, 720, 576)];
    }
    }
}

@end
