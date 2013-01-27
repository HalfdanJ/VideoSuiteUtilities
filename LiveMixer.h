//
//  LiveMixer.h
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "MIDIReceiver.h"
extern MIDIReceiver * globalMidi;

@interface LiveMixer : NSObject{
    int _selectedInput;
}

@property int selectedInput;
@property int fadeFromInput;
@property float opacity;
@property float crossfade;

@property CIImage * input1;
@property CIImage * input2;
@property CIImage * input3;

@property (readonly) BOOL input1Selected;
@property (readonly) BOOL input2Selected;
@property (readonly) BOOL input3Selected;

@property int num;

@property (readonly) CIImage * output;

-(void) imageViewMouseDown:(id)sender;
- (id)initWithNum:(int)num;

@end
