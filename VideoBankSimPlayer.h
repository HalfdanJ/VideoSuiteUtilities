//
//  VideoBankSimPlayer.h
//  SH
//
//  Created by Flyvende Grise on 1/14/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "VideoBank.h"

#import "MIDIReceiver.h"
extern MIDIReceiver * globalMidi;

@interface VideoBankSimPlayer : NSObject{
    id midiSendObserverToken;
    BOOL _playing;
}

@property VideoBank * videoBank;

@property int bankSelection;
@property int numberOfBanksToPlay;
@property float playbackRate;
//@property int mask;
@property BOOL midi;

@property CALayer * layer;

@property BOOL playing;
@property float opacity;


@property NSString * currentTimeString;

@property NSMutableArray * players;

-(id)initWithBank:(VideoBank*)bank;


@end
