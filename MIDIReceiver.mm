//
//  MIDIReceiver.m
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "MIDIReceiver.h"
#import <CoreMIDI/CoreMIDI.h>


MIDIReceiver * globalMidi;

@interface MIDIReceiver ()
@end

@implementation MIDIReceiver

- (id)init
{
    self = [super init];
    if (self) {
        //------------------------------
        
        MIDIClientRef client = 0;
        MIDIClientCreate(CFSTR("Video"), MyMIDINotifyProc, (__bridge void*)(self), &client);
        
        MIDIPortRef inPort = 0;
        MIDIInputPortCreate(client, CFSTR("Input Port"), MyMIDIReadProc, (__bridge void*)(self), &inPort);
        
        ItemCount sourceCount = MIDIGetNumberOfSources();
        for(ItemCount i=0; i<sourceCount; i++){
            MIDIEndpointRef source = MIDIGetSource(i);
            if(source != 0){
                MIDIPortConnectSource(inPort, source, NULL);
            }
        }
        
        //------------------------------
        
        globalMidi = self;
        
        self.bindings = [NSMutableArray array];

    }
    return self;
}

-(void)addBindingTo:(id)object path:(NSString*)path channel:(int)channel number:(int)number range:(NSRange)range{
    [self willChangeValueForKey:@"bindings"];
    NSDictionary * newBinding = @{
    @"path" : path,
    @"object" : object,
    @"channel" : @(channel),
    @"number": @(number),
    @"range" : [NSValue valueWithRange:range]
    };
    
    [self.bindings addObject:newBinding];
    
        [self didChangeValueForKey:@"bindings"];
}

static void MyMIDIReadProc(const MIDIPacketList *pklist, void *refCon, void *connRefCon){
    MIDIReceiver * ad = (__bridge MIDIReceiver*)refCon;
    
    MIDIPacket * packet = (MIDIPacket*)pklist->packet;
    
    for (int i = 0; i < pklist->numPackets; ++i) {
        for (int j = 0; j < packet->length; j+=3) {
            
            
            Byte midiCommand = packet->data[0+j] >> 4;
            
            if(midiCommand==11){//CC
                int channel = (packet->data[0+j] & 0xF) + 1;
                int number = packet->data[1+j] & 0x7F;
                int value = packet->data[2+j] & 0x7F;
                //      NSLog(@"%i %i %i",channel, number, value);
                
                dispatch_async(dispatch_get_main_queue(), ^{
                    for(NSDictionary * dict in ad.bindings){
                        if([[dict valueForKey:@"number"] intValue] == number && [[dict valueForKey:@"channel"] intValue] == channel){
                            
                            NSRange range = [[dict valueForKey:@"range"] rangeValue];
                            
                            
                            float _val = value;

                            float scale = range.length / 127.0;
                            _val *= scale;
                            _val += range.location;
                            
                            [[dict valueForKey:@"object"] setValue:@(_val) forKeyPath:[dict valueForKey:@"path"]];
                        }
                    }
                    
                    /*  if(channel == 1 && number == 1){
                     ad.fadeTime = value / 128.0;
                     }
                     if(channel == 1 && number == 0){
                     ad.master = value / 128.0;
                     }
                     
                     
                     if(channel == 2){
                     if(number == 0){
                     ad.outSelector = value;
                     }
                     
                     if(number == 1){
                     ad.decklink1input = value-1;
                     }
                     if(number == 2){
                     ad.decklink2input = value-1;
                     }
                     if(number == 3){
                     ad.decklink3input = value-1;
                     }
                     }
                     
                     if (channel == 3) {
                     if(number == 0){
                     [[NSUserDefaults standardUserDefaults] setBool:value forKey:@"chromaKey"];
                     }
                     }*/
                });
                //        if()
            }
        }
        
        packet = MIDIPacketNext(packet);
    }
}

void MyMIDINotifyProc( const MIDINotification *message, void*refCon){
    
}

@end
