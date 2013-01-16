//
//  CoreImageViewer.h
//  ViljensTriumf
//
//  Created by Jonas on 10/9/12.
//
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@interface CoreImageViewer : NSView{
    CIContext *ciContext;
}

@property (strong) CIImage * ciImage;
@property CALayer * activeLayer;
@property NSArray *filters;

@property id delegate;
@property id customData;
@property BOOL highlight;

@end
