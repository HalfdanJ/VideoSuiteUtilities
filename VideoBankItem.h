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



@property AVPlayerItem * avPlayerItem;
//@property AVAsset * avAsset;
@property AVPlayer * avPreviewPlayer;

-(void) loadBankFromDrive;
-(void) loadBankFromPath:(NSString*)path;

@end
