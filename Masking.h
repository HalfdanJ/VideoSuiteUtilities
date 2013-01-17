//
//  Masking.h
//  SH
//
//  Created by Flyvende Grise on 1/17/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import "VDKQueue.h"

#import "MIDIReceiver.h"
extern MIDIReceiver * globalMidi;

@interface Masking : NSObject<VDKQueueDelegate>

@property CALayer * maskingLayer;
@property NSArray * masks;
@property int selectedMask;
@property float opacity;
@property VDKQueue * queue;


@end
