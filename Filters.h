//
//  Filters.h
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import "DeinterlaceFilter.h"

@interface Filters : NSObject

@property (readonly) NSArray * filters;

@property CIFilter * colorControlsFilter;

@property float transformX;
@property float transformY;
@property float transformScale;

@property BOOL deinterlace;


-(void)makeDefaults;

@end
