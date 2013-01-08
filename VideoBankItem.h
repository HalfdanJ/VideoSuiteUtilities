//
//  VideoBankItem.h
//  SH
//
//  Created by Jonas Jongejan on 06/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@interface VideoBankItem : NSObject

@property NSString * name;
@property BOOL loaded;

@property double duration;
@property (readonly) NSString * durationString;

@property NSNumber * inTime;
@property (readonly) NSString * inTimeString;
@property NSNumber * outTime;
@property (readonly) NSString * outTimeString;

@property NSNumber * crossfadeTime;

@property AVPlayerItem * avPlayerItem;
//@property AVAsset * avAsset;
@property AVPlayer * avPreviewPlayer;

@property double playHeadPosition;
@property BOOL queued;
@property BOOL playing;

-(void) loadBankFromDrive;
-(void) loadBankFromPath:(NSString*)path;

@end
