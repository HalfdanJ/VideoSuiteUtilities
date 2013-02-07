//
//  MIDIReceiver.m
//  SH
//
//  Created by Flyvende Grise on 1/16/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "MIDIReceiver.h"


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
        
        
        
        MIDIOutputPortCreate(client, (CFStringRef)@"MIDI Output Port", &outputPort);
        
        //------------------------------
        
        globalMidi = self;
        
        self.bindings = [NSMutableArray array];
        
    }
    return self;
}


-(void)addBindingTo:(id)object path:(NSString*)path channel:(int)channel number:(int)number rangeMin:(float)rangeMin rangeLength:(float)rangeLength{
    [self willChangeValueForKey:@"bindings"];
    NSDictionary * newBinding = @{
    @"path" : path,
    @"object" : object,
    @"channel" : @(channel),
    @"pitch"   : @(NO),
    @"number": @(number),
    @"rangeMin" : @(rangeMin),
    @"rangeLength" : @(rangeLength)
    };
    
    [self.bindings addObject:newBinding];
    
    [self didChangeValueForKey:@"bindings"];
}

-(void)addBindingPitchTo:(id)object path:(NSString*)path channel:(int)channel rangeMin:(float)rangeMin rangeLength:(float)rangeLength{
    
    NSDictionary * newBinding = @{
    @"path" : path,
    @"object" : object,
    @"channel" : @(channel),
    @"pitch"   : @(YES),
    @"number": @(0),
    @"rangeMin" : @(rangeMin),
    @"rangeLength" : @(rangeLength)
    };
    
    [self.bindings addObject:newBinding];
    
    [self didChangeValueForKey:@"bindings"];
}

-(void)addBindingTo:(id)object selector:(NSString*)selector channel:(int)channel number:(int)number {
    [self willChangeValueForKey:@"bindings"];
    NSDictionary * newBinding = @{
    @"selector" : selector,
    @"object" : object,
    @"channel" : @(channel),
    @"number": @(number),
    @"pitch"   : @(NO),
    };
    
    [self.bindings addObject:newBinding];
    
    [self didChangeValueForKey:@"bindings"];
}


- (void) sendMidiChannel:(int)_midiChannel number:(int)midiNote value:(int)midiValue
{
    
    
	MIDIPacketList packetlist;
	MIDIPacket     *packet     = MIDIPacketListInit(&packetlist);
	Byte mdata[3] = {(const Byte)(143+_midiChannel), (const Byte) midiNote, (const Byte)midiValue};
	packet = MIDIPacketListAdd(&packetlist, sizeof(packetlist),
                      packet, 0, 3, mdata);
    
    
    // Send it to every destination in the system...
    for (ItemCount index = 0; index < MIDIGetNumberOfDestinations(); ++index)
    {
        MIDIEndpointRef outputEndpoint = MIDIGetDestination(index);
        if (outputEndpoint)
        {
            // Send it
            OSStatus s = MIDISend(outputPort, outputEndpoint, &packetlist);
        }
    }
}

static void MyMIDIReadProc(const MIDIPacketList *pklist, void *refCon, void *connRefCon){
    MIDIReceiver * ad = (__bridge MIDIReceiver*)refCon;
    
    MIDIPacket * packet = (MIDIPacket*)pklist->packet;
    
    for (int i = 0; i < pklist->numPackets; ++i) {
        for (int j = 0; j < packet->length; j+=3) {
            
            
            Byte midiCommand = packet->data[0+j] >> 4;
            if(midiCommand == 14){ //Pitch
                int channel = (packet->data[0+j] & 0xF) + 1;
                int value = (packet->data[1+j]& 0x7F) + 128*(packet->data[2+j] & 0x7F);
                
                dispatch_async(dispatch_get_main_queue(), ^{
                    for(NSDictionary * dict in ad.bindings){
                        if([[dict valueForKey:@"channel"] intValue] == channel && [[dict valueForKey:@"pitch"] boolValue]){
                            
                            id object = [dict valueForKey:@"object"];
                            
                            if([dict valueForKey:@"selector"]){
                                SEL selector = NSSelectorFromString([dict valueForKey:@"selector"]);
                                
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
                                [object performSelector:selector];
#pragma clang diagnostic pop
                                
                            } else {
                                
                                float rangeMin = [[dict valueForKey:@"rangeMin"] floatValue];
                                float rangeLength = [[dict valueForKey:@"rangeLength"] floatValue];
                                
                                float _val = value;
                                
                                float scale = rangeLength / (128.0*128.0);
                                _val *= scale;
                                _val += rangeMin;
                                [object setValue:@(_val) forKeyPath:[dict valueForKey:@"path"]];
                            }
                        }
                    }
                    
                });
            }
            if(midiCommand==11){//CC
                int channel = (packet->data[0+j] & 0xF) + 1;
                int number = packet->data[1+j] & 0x7F;
                int value = packet->data[2+j] & 0x7F;
                //      NSLog(@"%i %i %i",channel, number, value);
                
                dispatch_async(dispatch_get_main_queue(), ^{
                    for(NSDictionary * dict in ad.bindings){
                        if([[dict valueForKey:@"number"] intValue] == number && [[dict valueForKey:@"channel"] intValue] == channel&& ![[dict valueForKey:@"pitch"] boolValue]){
                            
                            id object = [dict valueForKey:@"object"];
                            
                            if([dict valueForKey:@"selector"]){
                                SEL selector = NSSelectorFromString([dict valueForKey:@"selector"]);
                                
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
                                [object performSelector:selector];
#pragma clang diagnostic pop
                                
                            } else {
                                
                                float rangeMin = [[dict valueForKey:@"rangeMin"] floatValue];
                                float rangeLength = [[dict valueForKey:@"rangeLength"] floatValue];
                                
                                float _val = value;
                                
                                float scale = rangeLength / 127.0;
                                _val *= scale;
                                _val += rangeMin;
                                
                                [object setValue:@(_val) forKeyPath:[dict valueForKey:@"path"]];
                            }
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
