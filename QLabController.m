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
            if([midiDict objectForKey:@"object"] == sender){
                if([[midiDict objectForKey:@"path"] isEqualToString:[dict valueForKey:QPath]]){
                    
                    
                    int channel = [[midiDict objectForKey:@"channel"] intValue];
                    int number = [[midiDict objectForKey:@"number"] intValue];
                    
                    NSNumber * value;
                    
                    if([dict valueForKey:QValue] == nil){
                        value = [sender valueForKeyPath:[dict valueForKey:QPath]];
                    } else {
                        value = [dict valueForKey:QValue];
                    }
                    NSRange range = [[midiDict objectForKey:@"range"] rangeValue];
                    float scale =   127.0 / range.length;
                    
                    int _val = ([value floatValue] - range.location) * scale;
                    
                    
                    
                    [string appendFormat:@"make type \"midi\"\n\
                     set myMIDICue to last item of (selected as list)\n\
                     tell myMIDICue\n\
                     set patch to 1\n\
                     set channel to %i\n\
                     set command to control_change\n\
                     set byte one to %i\n\
                     set byte two to %i\n\
                     set q name to \"%@\"\n\
                     end tell\n\
                     \n\
                     \n\
                     if contents of myMIDICue is not groupCueIsIn then -- Skip a Group Cue that contains the new Group Cue\n\
                     set eachCueID to uniqueID of myMIDICue\n\
                     move cue id eachCueID of parent of myMIDICue to end of groupCue\n\
                     end if\n", channel, number, _val, [dict valueForKey:QName]];
                    
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
