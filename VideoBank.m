//
//  VideoBank.m
//  SH
//
//  Created by Jonas Jongejan on 06/01/13.
//  Copyright (c) 2013 HalfdanJ. All rights reserved.
//

#import "VideoBank.h"
#import "QLabController.h"

@implementation VideoBank
static void *SelectionContext = &SelectionContext;

-(NSString*)name{
    return @"VideoBank";
}

- (id)init
{
    self = [super init];
    if (self) {
        [self addObserver:self forKeyPath:@"selectionIndex" options:0 context:SelectionContext];
        [self addObserver:self forKeyPath:@"selection.avPlayerItemOriginal" options:0 context:SelectionContext];
        
        int num = 40;
        //[globalMidi addBindingTo:self path:@"selectionIndex" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingPitchTo:self path:@"selectionIndex" channel:1 rangeMin:-8192 rangeLength:128*128];
        [globalMidi addBindingTo:self path:@"selectedBank.inTime" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"selectedBank.outTime" channel:1 number:num++ rangeMin:0 rangeLength:127];

        [globalMidi addBindingTo:self path:@"selectedBank.crossfadeTime" channel:1 number:num++ rangeMin:0 rangeLength:127];
        [globalMidi addBindingTo:self path:@"selectedBank.mask" channel:1 number:num++ rangeMin:0 rangeLength:127];

        [globalMidi addBindingPitchTo:self path:@"copyToBankIndex" channel:5 rangeMin:-8192 rangeLength:128*128];

//        [globalMidi addBindingTo:self path:@"copyToBankIndex" channel:1 number:num++ rangeMin:0 rangeLength:127];
        
        
        [globalMidi addBindingTo:self selector:@"defaultsAll" channel:1 number:num++];
        [globalMidi addBindingTo:self selector:@"copyBank" channel:1 number:num++];
        
        [self setSelectionIndex:0];

     /*   self.fileWatcher = [[VDKQueue alloc]init];
        [self.fileWatcher addPath:[@"~/Movies/Bank 13.mov" stringByExpandingTildeInPath]];
        self.fileWatcher.delegate = self;*/
    }
    return self;
}

-(void)VDKQueue:(VDKQueue *)queue receivedNotification:(NSString *)noteName forPath:(NSString *)fpath{
    NSLog(@"Path %@",noteName);
}
/*
-(void)watcher:(id<UKFileWatcher>)kq receivedNotification:(NSString *)nm forPath:(NSString *)fpath{
    for(VideoBankItem * item in self.content){
        [item loadBankFromDrive];
    }
 //   [self loadBankFromDrive];
}*/

- (id)initWithNumberBanks:(int)banks
{
    self = [self init];
    if (self) {
        self.content = [NSMutableArray arrayWithCapacity:banks];
        
        for(int i=0;i<banks;i++){
            VideoBankItem * newItem = [[VideoBankItem alloc] initWithName:[NSString stringWithFormat:@"Bank %02i",i]];
            
            [newItem loadBankFromDrive];

            
            [self addObject:newItem];
        }
        
    }
    return self;
}


-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context{
    if(context == SelectionContext){
        NSLog(@"Selection");
        
        [self willChangeValueForKey:@"selectedBank"];
        [self didChangeValueForKey:@"selectedBank"];
        
        if(self.selectedObjects.count > 0){
            if(self.videoPreviewView){
                VideoBankItem * item = self.selectedObjects[0];
                
                
                [self.videoPreviewView unbind:@"inTime"];
                [self.videoPreviewView bind:@"inTime" toObject:item withKeyPath:@"inTime" options:nil];

                [self.videoPreviewView unbind:@"outTime"];
                [self.videoPreviewView bind:@"outTime" toObject:item withKeyPath:@"outTime" options:nil];

                self.videoPreviewView.movieItem = item.avPlayerItemOriginal;
            }
        }
    }
}

-(int)numberBanks{
    return (int) [self.content count];
}

+(NSSet *)keyPathsForValuesAffectingNumberBanks{
    return [NSSet setWithObjects:@"content", nil];
}

-(void)copyBank{
    if(self.copyToBankIndex != self.selectionIndex){
        VideoBankItem * fromObject = self.selectedBank;
        VideoBankItem * toObject = [self.content objectAtIndex:self.copyToBankIndex];
        
        
        NSString * fromPath = [fromObject.path stringByExpandingTildeInPath];
        NSString * toPath = [toObject.path stringByExpandingTildeInPath];


        NSFileManager * fileManager = [NSFileManager defaultManager];
        [fileManager copyItemAtPath:fromPath toPath:toPath error:nil];
        
        [toObject loadBankFromDrive];
//        [fileManager removeItemAtPath:path error:&error];
        
    }
}

-(void)defaultsAll{
    for(VideoBankItem * item in self.content){
        if(item.loaded){
            item.inTime = nil;
            item.outTime = nil;
            item.crossfadeTime = nil;
        }
    }
}

-(VideoBankItem *)selectedBank{
    if(self.selectedObjects.count > 0){
        return self.selectedObjects[0];
    }
    return nil;
}



-(void) qlabSettings{
    if(self.selectedObjects.count > 0){
        
        VideoBankItem * selected = self.selectedObjects[0];
        NSArray * cues = @[
        @{QName : [NSString stringWithFormat:@"Bank Selection: %02li",self.selectionIndex], QPath: @"selectionIndex"},
        @{QName : [NSString stringWithFormat:@"In time: %.2f",[selected.inTime floatValue]], QPath: @"selectedBank.inTime"},
        @{QName : [NSString stringWithFormat:@"Out time: %.2f",[selected.outTime floatValue]], QPath: @"selectedBank.outTime"},
        @{QName : [NSString stringWithFormat:@"Crossfade time: %.2f",[selected.crossfadeTime floatValue]], QPath: @"selectedBank.crossfadeTime"},
        @{QName : [NSString stringWithFormat:@"Mask: %i",selected.mask], QPath: @"selectedBank.mask"},
        @{QName : [NSString stringWithFormat:@"Bank Selection: 00 (hack)"], QPath: @"selectionIndex", QValue: @(0)},
        @{QName : [NSString stringWithFormat:@"Bank Selection: %02li (hack)",self.selectionIndex], QPath: @"selectionIndex"},
        ];
        
        NSString * title = [NSString stringWithFormat:@"Bank %02li settings",self.selectionIndex];
        
        [QLabController createCues:cues groupTitle:title sender:self];
    }
}


-(void) qlabDefaults{
        NSArray * cues = @[
        @{QName : [NSString stringWithFormat:@"Defaults"], QSelector: @"defaultsAll"},

        ];
        
        NSString * title = [NSString stringWithFormat:@"Defaults Videobanks"];
        
        [QLabController createCues:cues groupTitle:title sender:self];
}

-(void) qlabCopy{
    NSArray * cues = @[
    @{QName : [NSString stringWithFormat:@"Bank Selection: %02li",self.selectionIndex], QPath: @"selectionIndex"},
    @{QName : [NSString stringWithFormat:@"Copy to: %02i",self.copyToBankIndex], QPath: @"copyToBankIndex"},
    @{QName : [NSString stringWithFormat:@"Copy Bank"], QSelector: @"copyBank"},
    ];
    
    NSString * title = [NSString stringWithFormat:@"Copy Bank %02li to %02i",self.selectionIndex, self.copyToBankIndex];
    
    [QLabController createCues:cues groupTitle:title sender:self];
}
@end
