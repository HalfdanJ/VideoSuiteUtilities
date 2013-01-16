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
{
    NSString * _manualPath;
}

@property NSString * name;
@property NSString * path;
@property BOOL locked;

@property BOOL loaded;

@property double duration;
@property (readonly) NSString * durationString;
@property double durationOriginal;
@property (readonly) NSString * durationOriginalString;

@property NSNumber * inTime;
@property (readonly) NSString * inTimeString;
@property NSNumber * outTime;
@property (readonly) NSString * outTimeString;

@property NSNumber * crossfadeTime;

//@property AVAsset * avAsset;

@property AVPlayerItem * avPlayerItemOriginal;
@property AVPlayerItem * avPlayerItemTrim;


@property double playHeadPosition;
@property BOOL queued;
@property BOOL playing;
@property int standardPlayerLabel;
@property int compositePlayerLabel;
@property int recordLabel;

@property NSImage * thumbnail;
@property (readonly) NSSize  size;

- (id)initWithName:(NSString*)name;

-(void) clear;
-(void) loadBankFromDrive;
-(void) loadBankFromPath:(NSString*)path;

-(void) qlabPlay;
-(void) qlabStop;

@end
