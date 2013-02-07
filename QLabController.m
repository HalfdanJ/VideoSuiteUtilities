//
//  QLabController.m
//  SH
//
//  Created by Flyvende Grise on 1/15/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "QLabController.h"

NSString *const QName = @"Name";
NSString *const QChannel = @"Channel";
NSString *const QNumber = @"Number";
NSString *const QValue = @"Value";
NSString *const QPath = @"Path";
NSString *const QSelector = @"Selector";
NSString *const QObject = @"QObject";

@implementation QLabController



+(void)createCues:(NSArray*)cues groupTitle:(NSString*)title sender:(id)sender{
    NSMutableString * string = [NSMutableString stringWithFormat:@"tell application \"QLab\" \n\
                                tell workspace 1\n\
                                \n\
                                make type \"Group\"\n\
                                set groupCue to last item of (selected as list)\n\
                                set q name of groupCue to \"%@\"\n\
                                set mode of groupCue to fire_all\n\
                                set groupCueIsIn to parent of groupCue\n\
                                \n\
                                \n\
                                ",title];
    
    for(NSDictionary * dict in cues){
        
        for(NSDictionary * midiDict in [globalMidi bindings]){
            id object = sender;
            if([dict valueForKey:QObject] != nil){
                object = [dict valueForKey:QObject];
            }
            
            if([midiDict objectForKey:@"object"] == object){
                if([[midiDict objectForKey:@"path"] isEqualToString:[dict valueForKey:QPath]]
                   || [[midiDict objectForKey:@"selector"] isEqualToString:[dict valueForKey:QSelector]]){
                    
                    
                    int channel = [[midiDict objectForKey:@"channel"] intValue];
                    int number = [[midiDict objectForKey:@"number"] intValue];
                    BOOL pitch = [[midiDict valueForKey:@"pitch"] boolValue];
                    
                    NSNumber * value;
                    int _val;
                    
                    if([[midiDict objectForKey:@"path"] isEqualToString:[dict valueForKey:QPath]]){
                        if([dict valueForKey:QValue] == nil){
                            value = [object valueForKeyPath:[dict valueForKey:QPath]];
                        } else {
                            value = [dict valueForKey:QValue];
                        }
                        float rangeMin = [[midiDict objectForKey:@"rangeMin"] floatValue];
                        float rangeLength = [[midiDict objectForKey:@"rangeLength"] floatValue];
                        
                        if(pitch){
                            float scale =   (128*128) / rangeLength;
                            
                            _val = ([value floatValue] - rangeMin) * scale;
                        } else {
                            float scale =   127.0 / rangeLength;
                            
                            _val = ([value floatValue] - rangeMin) * scale;
                        }
                    } else {
                        _val = 127;
                    }
                    
                    
                    NSString * type = @"control_change";
                    if(pitch){
                        type = @"pitch_bend";
                        
                        number = _val%128;
                        _val = (_val-number)/128.0;
                    }
                    
                    [string appendFormat:@"make type \"midi\"\n\
                     set myMIDICue to last item of (selected as list)\n\
                     tell myMIDICue\n\
                     set patch to 1\n\
                     set channel to %i\n\
                     set command to %@\n\
                     set byte one to %i\n\
                     set byte two to %i\n\
                     set q name to \"%@\"\n\
                     end tell\n\
                     \n\
                     \n\
                     if contents of myMIDICue is not groupCueIsIn then -- Skip a Group Cue that contains the new Group Cue\n\
                     set eachCueID to uniqueID of myMIDICue\n\
                     move cue id eachCueID of parent of myMIDICue to end of groupCue\n\
                     end if\n", channel,type, number, _val, [dict valueForKey:QName]];
                    
                }
            }
        }
        
        
        
    }
    [string appendFormat:@"\
     set mainCueList to a reference to cue list 1\n\
     set playback position of mainCueList to groupCue\n\
     end tell\n\
     end tell"];
    
    NSAppleScript * appleScript = [[NSAppleScript alloc] initWithSource:string];
    //NSError * error;
    [appleScript executeAndReturnError:nil];
    
    //    NSLog(@"Error %@",error);
}

@end
