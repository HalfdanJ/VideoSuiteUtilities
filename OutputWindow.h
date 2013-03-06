//
//  OutputWindow.h
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CoreImageViewer.h"

@interface OutputWindow : NSWindow{
  //  BOOL _fullscreen;
    
}
//@property BOOL fullscreen;

@property CoreImageViewer * imageViewer;
@property CALayer * layer;

@property NSArray * filters;

@property CIFilter * transformFilter;
@property CIFilter * movieColorFilter;
@property CIFilter * movieColorControls;



@end
