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

@property CoreImageViewer * imageViewer1;
@property CoreImageViewer * imageViewer2;
@property CoreImageViewer * imageViewer3;
@property CALayer * layer1;
@property CALayer * layer2;
@property CALayer * layer3;

@property NSArray * filters;

@end
