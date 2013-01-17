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
#import "VDKQueue.h"

#import "MIDIReceiver.h"
extern MIDIReceiver * globalMidi;

@interface VideoBank : NSArrayController<VDKQueueDelegate>

@property VideoPlayerView * videoPreviewView;
@property (readonly) int numberBanks;

@property (readonly) VideoBankItem * selectedBank;

@property int copyToBankIndex;

@property VDKQueue * fileWatcher;


- (id)initWithNumberBanks:(int)banks;

@end
