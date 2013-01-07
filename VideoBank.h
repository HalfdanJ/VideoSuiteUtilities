//
//  VideoBank.h
//  SH
//
//  Created by Jonas Jongejan on 06/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "VideoBankItem.h"
#import "VideoPlayerView.h"

@interface VideoBank : NSArrayController

@property VideoPlayerView * videoPreviewView;

- (id)initWithNumberBanks:(int)banks;

@end
