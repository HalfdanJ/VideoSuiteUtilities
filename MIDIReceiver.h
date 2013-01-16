//
//  MIDIReceiver.h
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface MIDIReceiver : NSObject

@property NSMutableArray * bindings;

-(void)addBindingTo:(id)object path:(NSString*)path channel:(int)channel number:(int)number range:(NSRange)range;
@end
