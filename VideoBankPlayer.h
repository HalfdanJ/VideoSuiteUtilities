//
//  VideoBankPlayer.h
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "VideoBank.h"

@interface VideoBankPlayer : NSObject
{
    BOOL _playing;
}



@property VideoBank * videoBank;

@property int bankSelection;
@property int numberOfBanksToPlay;
@property BOOL simultaneousPlayback;

@property CALayer * layer;

@property BOOL playing;

@property NSString * currentTimeString;



@end
