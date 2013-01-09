//
//  VideoBank.m
//  SH
//
//  Created by Jonas Jongejan on 06/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "VideoBank.h"

@implementation VideoBank
static void *SelectionContext = &SelectionContext;

- (id)init
{
    self = [super init];
    if (self) {
        [self addObserver:self forKeyPath:@"selection" options:0 context:SelectionContext];
    }
    return self;
}

- (id)initWithNumberBanks:(int)banks
{
    self = [self init];
    if (self) {
        self.content = [NSMutableArray arrayWithCapacity:banks];
        
        for(int i=0;i<banks;i++){
            VideoBankItem * newItem = [[VideoBankItem alloc] init];
            
            newItem.name = [NSString stringWithFormat:@"Bank %i",i];
            newItem.crossfadeTime = @(2);
            //[newItem loadBankFromDrive];
            if(i==0){
                [newItem loadBankFromPath:@"~/Movies/VideoSuite/Bif.mp4"];
                newItem.outTime = @(5);
            
            } /*else if(i==2){
                [newItem loadBankFromPath:@"/Users/jonas/Dropbox/Public/LEDDragt.MOV"];

                
            }*/ else {
                [newItem loadBankFromPath:@"~/Movies/VideoSuite/Bif2.mov"];

            }
            [self addObject:newItem];
        }
        
    }
    return self;
}


-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context == SelectionContext){
        NSLog(@"Selection");
        
        if(self.selectedObjects.count > 0){
            if(self.videoPreviewView){
                VideoBankItem * item = self.selectedObjects[0];
                
                
                [self.videoPreviewView unbind:@"inTime"];
                [self.videoPreviewView bind:@"inTime" toObject:item withKeyPath:@"inTime" options:nil];

                [self.videoPreviewView unbind:@"outTime"];
                [self.videoPreviewView bind:@"outTime" toObject:item withKeyPath:@"outTime" options:nil];

                self.videoPreviewView.movieItem = item.avPlayerItem;
            }
        }
    }
}
@end
