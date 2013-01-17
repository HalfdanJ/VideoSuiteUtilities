//
//  QLabController.h
//  SH
//
//  Created by Flyvende Grise on 1/15/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>

FOUNDATION_EXPORT NSString *const QName;
FOUNDATION_EXPORT NSString *const QChannel;
FOUNDATION_EXPORT NSString *const QNumber;
FOUNDATION_EXPORT NSString *const QValue;
FOUNDATION_EXPORT NSString *const QPath;
FOUNDATION_EXPORT NSString *const QSelector;

#import "MIDIReceiver.h"
extern MIDIReceiver * globalMidi;

@interface QLabController : NSObject


+(void)createCues:(NSArray*)cues groupTitle:(NSString*)title sender:(id)sender;
@end
