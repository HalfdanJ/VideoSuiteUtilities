//
//  VideoBankRecorder.h
//  SH
//
//  Created by Flyvende Grise on 1/10/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BlackMagicItem.h"
#import "VideoBank.h"

@interface VideoBankRecorder : NSObject<BlackMagicItemDelegate>

@property int bankIndex;
@property int deviceIndex;
@property BOOL record;
@property BOOL readyToRecord;
@property (readonly) BOOL canRecord;
@property NSString * timeString;

@property VideoBank * videoBank;
@property NSSize size;

-(id)initWithBlackmagicItems:(NSArray*)items bank:(VideoBank*)bank;
-(void)newFrameWithBufer:(CVPixelBufferRef)buffer image:(CIImage *)image item:(BlackMagicItem*)bmItem;

@end
