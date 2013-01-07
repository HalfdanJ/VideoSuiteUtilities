//
//  NSString_Timecode.h
//  SH
//
//  Created by Jonas Jongejan on 07/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (Timecode)
+(NSString*) stringWithTimecode:(double)seconds;

@end

@implementation NSString (Timecode)


+(NSString*) stringWithTimecode:(double)seconds{
    int ms = seconds * 1000.0;
//    NSLog(@"%f",seconds);
    return [NSString stringWithFormat:@"%02u:%02u:%03u",
            (ms/1000/60),       // M
            (ms/1000%60),       // S
            (ms % 1000) //MS
            ];
}

@end
