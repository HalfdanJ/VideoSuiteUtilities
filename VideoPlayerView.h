//
//  VideoPlayerView.h
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>


@interface VideoPlayerView : NSView

@property AVPlayerItem * movieItem;

@property BOOL playing;

@property NSNumber * inTime;
@property NSNumber * outTime;


@end
