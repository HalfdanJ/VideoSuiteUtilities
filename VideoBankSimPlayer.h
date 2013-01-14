//
//  VideoBankSimPlayer.h
//  SH
//
//  Created by Flyvende Grise on 1/14/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "VideoBank.h"

@interface VideoBankSimPlayer : NSObject

@property VideoBank * videoBank;

@property int bankSelection;
@property int numberOfBanksToPlay;

@property CALayer * layer;

@property BOOL playing;
@property float opacity;

@property NSString * currentTimeString;

@end
