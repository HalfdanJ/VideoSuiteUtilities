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
        [self addObserver:self forKeyPath:@"highlight" options:0 context:CIImageContext];
        
    }
    
    return self;
}

-(void)mouseUp:(NSEvent *)theEvent{
    if(self.delegate){
        if([self.delegate respondsToSelector:@selector(imageViewMouseDown:)]){
            [self.delegate performSelector:@selector(imageViewMouseDown:) withObject:self];
        }
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context == CIImageContext){
        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    [[NSColor blackColor] set];
    NSRectFill(dirtyRect);
    
    if(!self.ciImage){
        NSLog(@"No ciimage");

    } else {
        @autoreleasepool {
            if(ciContext == nil){
                ciContext = [CIContext contextWithCGContext:
                             [[NSGraphicsContext currentContext] graphicsPort] options: nil];
            }
            
            CIImage * image = self.ciImage;
            if(self.filters){
                for(CIFilter * filter in self.filters){
                    [filter setValue:image forKey:@"inputImage"];
                    image = [filter valueForKey:@"outputImage"];
                }
            }
            
            [ciContext drawImage:image inRect:NSRectToCGRect(dirtyRect) fromRect:NSMakeRect(0, 0, 720, 576)];
            
            if(self.highlight){
                [[NSColor colorWithDeviceRed:1.0 green:0.0 blue:0.0 alpha:1.0] set];
                NSBezierPath * path = [NSBezierPath bezierPathWithRect:dirtyRect];
                [path setLineWidth:5.0];
                [path stroke];
            }
        }
    }
}

@end
