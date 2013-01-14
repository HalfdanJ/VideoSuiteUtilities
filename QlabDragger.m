//
//  QlabDragger.m
//  SH
//
//  Created by Flyvende Grise on 1/11/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "QlabDragger.h"

@implementation QlabDragger

-(void)awakeFromNib{
    [self registerForDraggedTypes:@[@"Cue"]];
}

-(void)mouseDragged:(NSEvent *)theEvent{
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSSize dragOffset = NSMakeSize(0.0, 0.0);
    NSPasteboard *pboard;
    
    pboard = [NSPasteboard pasteboardWithName:NSDragPboard];
    
    

    
    [self.values enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        [pboard declareTypes:@[key]  owner:self];
        [pboard setPropertyList:obj forType:key];

    }];
    
/*    [pboard declareTypes:self.types  owner:self];
   // [pboard setData:[[NSImage imageNamed:@"SH"] TIFFRepresentation] forType:NSTIFFPboardType];
    
    [pboard setPropertyList:self.propertyList forType:@"Cue"];
  */  
    [self dragImage:[NSImage imageNamed:@"SH"] at:NSZeroPoint offset:dragOffset
              event:theEvent pasteboard:pboard source:self slideBack:YES];
    
    return;
}

-(NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender{
    NSLog(@"Entered");
}
-(NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)flag{
    return NO;
}

-(BOOL)performDragOperation:(id<NSDraggingInfo>)sender{
    
    NSLog(@"Source %@",[sender draggingSource]);
  
    NSPasteboard *pasteboard = [sender draggingPasteboard];

    NSLog(@"Pasteboard name %@",pasteboard.name);
    
    NSArray *classes = [[NSArray alloc] initWithObjects:[NSAttributedString class], [NSString class], nil];
    NSDictionary *options = [NSDictionary dictionary];
    NSArray *copiedItems = [pasteboard readObjectsForClasses:classes options:options];
    /*if (copiedItems != nil) {
     // Do something with the contents...
     }*/
    
    NSPasteboardItem * item = [pasteboard pasteboardItems][0];
    NSLog(@"Pasteboard types %@",[pasteboard types]);


/*    self.values = [NSMutableDictionary dictionary];
    
    for(int i=1;i<[pasteboard types].count;i+=2){
        NSString * type = [pasteboard types][i];
        
        CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassNSPboardType, (__bridge CFStringRef)(type), kUTTypeData);

        [self.values setObject:[item propertyListForType:(__bridge NSString*)uti] forKey:type];
    }
    NSLog(@"%@",self.values);
    */
    
    CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassNSPboardType, CFSTR("Cue"), kUTTypeData);
    //NSLog(@"UTI %@",uti);
    
    self.propertyList = [[item propertyListForType:(__bridge NSString*)uti] mutableCopy];
 NSMutableArray * objects =[[self.propertyList objectForKey:@"$objects"] mutableCopy];
    //NSLog(@"Pasteboard %@",[propertyList objectForKey:@"$objects"]);
    
    int i=0;
    for(id obj in objects){
       NSLog(@"\n\n %i: \n%@",i++,obj);
    }
    
    objects[6] = @"oapsdkpoaskd";
    
    [self.propertyList setObject:objects forKey:@"$objects"];
   
    
    [pasteboard clearContents];
    
    [pasteboard addTypes:@[@"Cue"] owner:nil];
    [pasteboard setPropertyList:self.propertyList forType:@"Cue"];
    
    
   // BOOL write = [objects writeToFile:[@"~/Desktop/drag.plist" stringByExpandingTildeInPath] atomically:YES];
  //  NSLog(@"Write %i",write);

    return YES;
}

@end
