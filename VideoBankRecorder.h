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

@property VideoBank * videoBank;

-(id)initWithBlackmagicItems:(NSArray*)items;
-(void)newFrameWithBufer:(CVPixelBufferRef)buffer image:(CIImage *)image item:(BlackMagicItem*)bmItem;

@end
